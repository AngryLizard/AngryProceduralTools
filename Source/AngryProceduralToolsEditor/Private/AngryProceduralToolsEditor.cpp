#include "AngryProceduralToolsEditor.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Nodes/AngryProceduralDetails.h"
#include "Nodes/Node_NormalCurve.h"
#include "Nodes/Node_UnitCurve.h"

#include "Actors/ProceduralActor.h"
#include "Textures/PaintGradient.h"
#include "Factories/PaintGradient/AssetTypeActions_PaintGradient.h"
#include "Factories/PaintGradient/PaintGradientThumbnailRenderer.h"
#include "Textures/ExteriorGradient.h"
#include "Factories/ExteriorGradient/AssetTypeActions_ExteriorGradient.h"
#include "Factories/ExteriorGradient/ExteriorGradientThumbnailRenderer.h"

#include "IPlacementModeModule.h"

#include "Actors/WorldPainterLayer.h"
#include "Actors/WorldPainterBrush.h"

DEFINE_LOG_CATEGORY(AngryProceduralToolsEditor);

#define LOCTEXT_NAMESPACE "FAngryProceduralToolsEditor"

void FAngryProceduralToolsEditor::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(AProceduralActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAngryProceduralDetails::MakeInstance));
	}

	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	IAssetTools& AssetTools = AssetToolsModule.Get();

	AssetTypeActions_PaintGradient = MakeShareable(new FAssetTypeActions_PaintGradient);
	AssetTools.RegisterAssetTypeActions(AssetTypeActions_PaintGradient->AsShared());
	UThumbnailManager::Get().RegisterCustomRenderer(UPaintGradient::StaticClass(), UPaintGradientThumbnailRenderer::StaticClass());

	AssetTypeActions_ExteriorGradient = MakeShareable(new FAssetTypeActions_ExteriorGradient);
	AssetTools.RegisterAssetTypeActions(AssetTypeActions_ExteriorGradient->AsShared());
	UThumbnailManager::Get().RegisterCustomRenderer(UExteriorGradient::StaticClass(), UExteriorGradientThumbnailRenderer::StaticClass());

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("NormalCurve", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNode_NormalCurve::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("UnitCurve", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNode_UnitCurve::MakeInstance));

	IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
	PlacementModeModule.RegisterPlacementCategory(FPlacementCategoryInfo(LOCTEXT("WorldPainter_CategoryName", "World Painter"), "WorldPainter", TEXT("PMWorldPainter"), 69));
	PlacementModeModule.OnPlacementModeCategoryRefreshed().AddRaw(this, &FAngryProceduralToolsEditor::OnPlacementModeRefresh);
}

void FAngryProceduralToolsEditor::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (UObjectInitialized())
	{
		FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
		IAssetTools& AssetTools = AssetToolsModule.Get();
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_PaintGradient->AsShared());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_ExteriorGradient->AsShared());
	}
}

void FAngryProceduralToolsEditor::OnPlacementModeRefresh(FName CategoryName)
{
	static FName VolumeName = FName(TEXT("WorldPainter"));
	if (CategoryName == VolumeName)
	{
		IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
		PlacementModeModule.RegisterPlaceableItem(CategoryName, MakeShareable(new FPlaceableItem(nullptr, FAssetData(AWorldPainterLayer::StaticClass()))));
		PlacementModeModule.RegisterPlaceableItem(CategoryName, MakeShareable(new FPlaceableItem(nullptr, FAssetData(AWorldPainterBrush::StaticClass()))));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAngryProceduralToolsEditor, AngryProceduralToolsEditor)
