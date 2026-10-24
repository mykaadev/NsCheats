#pragma once

#include "Modules/ModuleManager.h"

class FNsCheatsRuntimeModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
