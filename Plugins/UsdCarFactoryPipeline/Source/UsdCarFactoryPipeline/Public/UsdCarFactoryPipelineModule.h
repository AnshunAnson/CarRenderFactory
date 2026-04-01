#pragma once

#include "Modules/ModuleInterface.h"

class FUsdCarFactoryPipelineModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
