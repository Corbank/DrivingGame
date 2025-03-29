#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Module for OpenWorldExplorer game
 */
class FOpenWorldExplorerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};