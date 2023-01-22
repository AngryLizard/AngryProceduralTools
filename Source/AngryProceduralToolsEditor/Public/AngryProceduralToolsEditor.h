#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(AngryProceduralToolsEditor, Log, All);

/**
 * The public interface to this module
 */
class FAngryProceduralToolsEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<class FAssetTypeActions_PaintGradient> AssetTypeActions_PaintGradient;
	TSharedPtr<class FAssetTypeActions_ExteriorGradient> AssetTypeActions_ExteriorGradient;
	TSharedPtr<class FAssetTypeActions_WorldPainterTexture> AssetTypeActions_WorldPainterTexture;

	void OnPlacementModeRefresh(FName CategoryName);
};