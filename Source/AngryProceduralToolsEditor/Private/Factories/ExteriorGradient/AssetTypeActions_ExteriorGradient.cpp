
#include "Factories/ExteriorGradient/AssetTypeActions_ExteriorGradient.h"
#include "Textures/ExteriorGradient.h"
#include "Interfaces/ITextureEditorModule.h"
#include "ThumbnailRendering/SceneThumbnailInfoWithPrimitive.h"

#define LOCTEXT_NAMESPACE "ExteriorGradient"

UClass* FAssetTypeActions_ExteriorGradient::GetSupportedClass() const
{
	return UExteriorGradient::StaticClass();
}

void FAssetTypeActions_ExteriorGradient::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Texture = Cast<UTexture>(*ObjIt);
		if (Texture != NULL)
		{
			ITextureEditorModule* TextureEditorModule = &FModuleManager::LoadModuleChecked<ITextureEditorModule>("TextureEditor");
			TextureEditorModule->CreateTextureEditor(Mode, EditWithinLevelEditor, Texture);
		}
	}
}


UThumbnailInfo* FAssetTypeActions_ExteriorGradient::GetThumbnailInfo(UObject* Asset) const
{
	UExteriorGradient* ExteriorGradient = CastChecked<UExteriorGradient>(Asset);
	UThumbnailInfo* ThumbnailInfo = ExteriorGradient->ThumbnailInfo;
	if (ThumbnailInfo == NULL)
	{
		ThumbnailInfo = NewObject<USceneThumbnailInfo>(ExteriorGradient, NAME_None, RF_Transactional);
		ExteriorGradient->ThumbnailInfo = ThumbnailInfo;
	}

	return ThumbnailInfo;
}

#undef LOCTEXT_NAMESPACE
