// Copyright Kingdawn. All Rights Reserved.

#include "KingdawnCombat.h"
#include "Core/KDGameplayTags.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FKingdawnCombatModule"

void FKingdawnCombatModule::StartupModule()
{
	// Register native gameplay tags
	FKDGameplayTags::InitTags();
}

void FKingdawnCombatModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FKingdawnCombatModule, KingdawnCombat)
