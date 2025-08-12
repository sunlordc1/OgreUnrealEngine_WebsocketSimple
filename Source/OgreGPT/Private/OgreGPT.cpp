// Copyright Epic Games, Inc. All Rights Reserved.
#include "OgreGPT.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FOgreGPTModule"

void FOgreGPTModule::StartupModule()
{
 
}

void FOgreGPTModule::ShutdownModule()
{
    // Cleanup nếu cần
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOgreGPTModule, OgreGPT)

