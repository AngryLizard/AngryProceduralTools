#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(AngryProceduralTools, Log, All);

class FAngryProceduralToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
