
#include "Factories/PaintGradient/AssetTypeActions_PaintGradient.h"
#include "Textures/PaintGradient.h"
#include "Interfaces/ITextureEditorModule.h"
#include "ThumbnailRendering/SceneThumbnailInfoWithPrimitive.h"

#define LOCTEXT_NAMESPACE "PaintGradient"

UClass* FAssetTypeActions_PaintGradient::GetSupportedClass() const
{
	return UPaintGradient::StaticClass();
}

void FAssetTypeActions_PaintGradient::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
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


UThumbnailInfo* FAssetTypeActions_PaintGradient::GetThumbnailInfo(UObject* Asset) const
{
	UPaintGradient* PaintGradient = CastChecked<UPaintGradient>(Asset);
	UThumbnailInfo* ThumbnailInfo = PaintGradient->ThumbnailInfo;
	if (ThumbnailInfo == NULL)
	{
		ThumbnailInfo = NewObject<USceneThumbnailInfo>(PaintGradient, NAME_None, RF_Transactional);
		PaintGradient->ThumbnailInfo = ThumbnailInfo;
	}

	return ThumbnailInfo;
}

#undef LOCTEXT_NAMESPACE
