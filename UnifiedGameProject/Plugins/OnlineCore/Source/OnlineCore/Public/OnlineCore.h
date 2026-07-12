#pragma once

#include "Modules/ModuleManager.h"

class FOnlineCoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
