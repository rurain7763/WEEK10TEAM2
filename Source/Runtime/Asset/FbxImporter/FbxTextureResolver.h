#pragma once
#include "Asset/ModelData.h"
#include "Containers/Map.h"
#include "Core/EngineLog.h"
#include "Core/PathUtils.h"
#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fbxsdk.h>


// 모델 단위의 텍스처 탐색과 파일 인덱스 관리
class FFbxTextureResolver
{
    using FPath = std::filesystem::path;
    using FFileIndex = TMap<FString, TArray<FPath>>;

    // 검색 실패와 중복 후보의 구분
    enum class ESearchResult
    {
        NotFound,
        Found,
        Ambiguous
    };

public:
    FFbxTextureResolver(const FPath& FbxPath, const TArray<FPath>& InAdditionalDirectories)
        : FbxDirectory(std::filesystem::weakly_canonical(FbxPath.parent_path())),
        WorkingDirectory(std::filesystem::weakly_canonical(std::filesystem::current_path())),
        AdditionalDirectories(InAdditionalDirectories)
    {
        ExtractionDirectory = FbxPath;
        ExtractionDirectory.replace_extension(".fbm");
        ExtractionDirectory = ExtractionDirectory.lexically_normal();
    }

    // SDK에 전달할 기본 추출 폴더
    const FPath& GetExtractionDirectory() const
    {
        return ExtractionDirectory;
    }

    // SDK가 확정한 실제 추출 폴더 반영
    void SetExtractionDirectory(const char* Directory)
    {
        if (Directory && Directory[0] != '\0')
        {
            ExtractionDirectory = std::filesystem::weakly_canonical(
                std::filesystem::absolute(ReadReference(Directory)));
        }
    }

    // 캐시가 참조하는 실제 텍스처 파일 확인
    bool AreCachedTexturesAvailable(const FModelData& ModelData) const
    {
        for (const FMeshMaterialSlot& Slot : ModelData.MeshData.MaterialSlots)
        {
            const FString* Paths[] = { &Slot.DiffuseTexturePath, &Slot.NormalTexturePath };

            for (const FString* Path : Paths)
            {
                if (!Path->empty() && FindRegularFile(PathUtils::FromUtf8(*Path)).empty())
                {
                    return false;
                }
            }
        }
        return true;
    }

    // 미해결 참조가 포함된 결과의 캐시 저장 제외
    bool HasUnresolvedTextures() const
    {
        return bHasUnresolvedTextures;
    }

    // 단일 슬롯에서 사용할 첫 번째 유효 파일 연결 해석
    FString ResolveProperty(const fbxsdk::FbxProperty& Property) const
    {
        const int32 TextureCount = Property.GetSrcObjectCount<fbxsdk::FbxFileTexture>();

        for (int32 Index = 0; Index < TextureCount; ++Index)
        {
            const fbxsdk::FbxFileTexture* Texture =
                Property.GetSrcObject<fbxsdk::FbxFileTexture>(Index);

            FPath ResolvedPath;
            const ESearchResult Result = ResolveTexture(*Texture, ResolvedPath);
            if (Result == ESearchResult::Found)
            {
                return MakeRuntimePath(ResolvedPath);
            }
            if (Result == ESearchResult::Ambiguous)
            {
                HTR_LOG(Warning, "[Fbx] Ambiguous texture reference: {}", Texture->GetName());
                bHasUnresolvedTextures = true;
                return {};
            }
        }

        if (TextureCount > 0)
        {
            bHasUnresolvedTextures = true;
        }
        return {};
    }

private:
    // 빈 문자열 및 경로 구분자 정리
    FPath ReadReference(const char* Name) const
    {
        if (!Name || Name[0] == '\0')
        {
            return {};
        }

        FString Text = Name;
        std::replace(Text.begin(), Text.end(), '\\', '/');
        return PathUtils::FromUtf8(Text).lexically_normal();
    }

    // 대소문자를 통일한 경로 비교 키
    FString MakePathKey(const FPath& Path) const
    {
        std::wstring Text = Path.generic_wstring();
        std::transform(Text.begin(), Text.end(), Text.begin(), [](wchar_t Character)
            {
                return static_cast<wchar_t>(std::towlower(Character));
            });
        return PathUtils::ToUtf8(FPath(Text));
    }

    // 디렉터리를 제외한 실제 파일 경로 확인
    FPath FindRegularFile(const FPath& Candidate) const
    {
        if (Candidate.empty())
        {
            return {};
        }

        std::error_code Error;
        if (!std::filesystem::is_regular_file(Candidate, Error))
        {
            return {};
        }

        FPath Result = std::filesystem::weakly_canonical(Candidate, Error);
        return Error ? FPath{} : Result;
    }

    // 문자열 접두사가 아닌 디렉터리 구성 요소 비교
    bool IsWithinDirectory(const FPath& Path, const FPath& Directory) const
    {
        if (Path.empty() || Directory.empty())
        {
            return false;
        }

        auto PathPart = Path.begin();
        for (const FPath& DirectoryPart : Directory)
        {
            if (PathPart == Path.end() ||
                MakePathKey(*PathPart) != MakePathKey(DirectoryPart))
            {
                return false;
            }
            ++PathPart;
        }
        return true;
    }

    // 동일 파일의 중복 후보 제거
    void AddUniquePath(TArray<FPath>& Paths, const FPath& Path) const
    {
        if (Path.empty())
        {
            return;
        }

        const FString Key = MakePathKey(Path);
        for (const FPath& Existing : Paths)
        {
            if (MakePathKey(Existing) == Key)
            {
                return;
            }
        }
        Paths.Add(Path);
    }

    // 고유 후보 선택 및 모호한 후보 보고
    ESearchResult SelectMatch(const TArray<FPath>& Matches, FPath& OutPath) const
    {
        if (Matches.IsEmpty())
        {
            return ESearchResult::NotFound;
        }
        if (Matches.Num() > 1)
        {
            HTR_LOG(Warning, "[Fbx] Multiple texture candidates:");
            for (const FPath& Match : Matches)
            {
                HTR_LOG(Warning, "  {}", PathUtils::ToUtf8(Match));
            }
            return ESearchResult::Ambiguous;
        }

        OutPath = Matches[0];
        return ESearchResult::Found;
    }

    // 검색 폴더의 파일명 인덱스 생성
    void IndexDirectory(const FPath& Directory, FFileIndex& Index) const
    {
        std::error_code Error;
        std::filesystem::recursive_directory_iterator Iterator(
            Directory, std::filesystem::directory_options::skip_permission_denied, Error);
        const std::filesystem::recursive_directory_iterator End;

        if (Error)
        {
            return;
        }

        while (Iterator != End)
        {
            if (Iterator->is_regular_file(Error))
            {
                const FPath File = FindRegularFile(Iterator->path());
                if (!File.empty())
                {
                    AddUniquePath(Index[MakePathKey(File.filename())], File);
                }
            }

            Error.clear();
            Iterator.increment(Error);
            if (Error)
            {
                HTR_LOG(Warning, "[Fbx] Texture directory scan failed: {}",
                    PathUtils::ToUtf8(Directory));
                break;
            }
        }
    }

    // 기본 검색과 추가 검색을 분리한 지연 인덱싱
    void EnsureFileIndices() const
    {
        if (bFileIndicesReady)
        {
            return;
        }
        bFileIndicesReady = true;

        IndexDirectory(FbxDirectory, DefaultFiles);
        if (!IsWithinDirectory(ExtractionDirectory, FbxDirectory))
        {
            IndexDirectory(ExtractionDirectory, DefaultFiles);
        }

        const char* SiblingNames[] = { "textures", "maps", "images", "materials" };
        for (const char* Name : SiblingNames)
        {
            const FPath Directory = FbxDirectory.parent_path() / Name;
            if (MakePathKey(Directory) != MakePathKey(FbxDirectory))
            {
                IndexDirectory(Directory, DefaultFiles);
            }
        }

        for (const FPath& Directory : AdditionalDirectories)
        {
            if (!Directory.empty())
            {
                IndexDirectory(std::filesystem::absolute(Directory), AdditionalFiles);
            }
        }
    }

    // 파일명을 포함한 후행 경로 구성 요소 일치 개수
    int32 CountMatchingSuffix(const FPath& File, const FPath& Reference) const
    {
        auto FilePart = File.end();
        auto ReferencePart = Reference.end();
        int32 Count = 0;

        while (FilePart != File.begin() && ReferencePart != Reference.begin())
        {
            --FilePart;
            --ReferencePart;

            if (*ReferencePart == "." || *ReferencePart == ".." ||
                ReferencePart->has_root_path() ||
                MakePathKey(*FilePart) != MakePathKey(*ReferencePart))
            {
                break;
            }
            ++Count;
        }
        return Count;
    }

    // 가장 긴 후행 경로 및 고유 파일명 검색
    ESearchResult FindIndexed(const FPath* References, int32 ReferenceCount,
        const FFileIndex& Index, FPath& OutPath) const
    {
        TArray<FPath> NameMatches;
        TArray<FPath> SuffixMatches;
        int32 BestSuffixLength = 1;

        for (int32 ReferenceIndex = 0; ReferenceIndex < ReferenceCount; ++ReferenceIndex)
        {
            const FPath& Reference = References[ReferenceIndex];
            if (Reference.empty())
            {
                continue;
            }

            const TArray<FPath>* Files = Index.Find(MakePathKey(Reference.filename()));
            if (!Files)
            {
                continue;
            }

            for (const FPath& File : *Files)
            {
                AddUniquePath(NameMatches, File);
                const int32 Length = CountMatchingSuffix(File, Reference);

                if (Length > BestSuffixLength)
                {
                    BestSuffixLength = Length;
                    SuffixMatches.Reset();
                }
                if (Length >= 2 && Length == BestSuffixLength)
                {
                    AddUniquePath(SuffixMatches, File);
                }
            }
        }

        return SelectMatch(BestSuffixLength >= 2 ? SuffixMatches : NameMatches, OutPath);
    }

    // 명시 참조부터 추가 폴더까지의 우선순위 검색
    ESearchResult ResolveTexture(const fbxsdk::FbxFileTexture& Texture, FPath& OutPath) const
    {
        const FPath References[] = {
            ReadReference(Texture.GetRelativeFileName()),
            ReadReference(Texture.GetFileName())
        };

        for (int32 Priority = 1; Priority <= 3; ++Priority)
        {
            TArray<FPath> Matches;

            for (const FPath& Reference : References)
            {
                if (Reference.empty())
                {
                    continue;
                }
                if (Priority == 2 && Reference.has_root_path())
                {
                    continue;
                }
                if (Priority == 3 && !Reference.is_absolute())
                {
                    continue;
                }
                if (Reference.has_root_path() && !Reference.is_absolute())
                {
                    continue;
                }

                const FPath Candidate = FindRegularFile(
                    Reference.is_absolute() ? Reference : FbxDirectory / Reference);

                if (Priority == 1 && !IsWithinDirectory(Candidate, ExtractionDirectory))
                {
                    continue;
                }
                AddUniquePath(Matches, Candidate);
            }

            const ESearchResult Result = SelectMatch(Matches, OutPath);
            if (Result != ESearchResult::NotFound)
            {
                return Result;
            }
        }

        EnsureFileIndices();

        const ESearchResult DefaultResult = FindIndexed(References, 2, DefaultFiles, OutPath);
        if (DefaultResult != ESearchResult::NotFound)
        {
            return DefaultResult;
        }

        const ESearchResult AdditionalResult = FindIndexed(References, 2, AdditionalFiles, OutPath);
        if (AdditionalResult == ESearchResult::NotFound)
        {
            HTR_LOG(Warning, "[Fbx] Texture not found: file='{}', relative='{}'",
                PathUtils::ToUtf8(References[1]), PathUtils::ToUtf8(References[0]));
        }
        return AdditionalResult;
    }

    // 프로젝트 내부 파일의 기존 상대 경로 표기 유지
    FString MakeRuntimePath(const FPath& Path) const
    {
        if (IsWithinDirectory(Path, WorkingDirectory))
        {
            return PathUtils::ToUtf8(Path.lexically_relative(WorkingDirectory));
        }
        return PathUtils::ToUtf8(Path);
    }

private:
    FPath FbxDirectory;
    FPath WorkingDirectory;
    FPath ExtractionDirectory;
    TArray<FPath> AdditionalDirectories;

    mutable FFileIndex DefaultFiles;
    mutable FFileIndex AdditionalFiles;
    mutable bool bFileIndicesReady = false;
    mutable bool bHasUnresolvedTextures = false;
};