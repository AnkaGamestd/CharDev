// Copyright Kingdawn. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Mastery/KDMasterySlot.h"
#include "KDMasteryAssetAuthoringLibrary.generated.h"

class UKDSkillDefinition;
class UKDMasteryDefinition;
class UGameplayAbility;

UCLASS()
class KINGDAWNCOMBAT_API UKDMasteryAssetAuthoringLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Mastery|Authoring")
	static FKDMasterySlot MakeMasterySlot(
		FGameplayTag SlotTag,
		UKDSkillDefinition* SkillDefinition,
		const TArray<FGameplayTag>& PrerequisiteSlots,
		FVector2D TreePosition);

	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Mastery|Authoring")
	static void ConfigureSkillDefinition(
		UKDSkillDefinition* SkillDefinition,
		FGameplayTag SkillTag,
		FText DisplayName,
		FText Description,
		EKDMasteryTier RequiredTier,
		int32 MasteryPointCost,
		TSubclassOf<UGameplayAbility> AbilityClass);

	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Mastery|Authoring")
	static void ConfigureMasteryDefinition(
		UKDMasteryDefinition* MasteryDefinition,
		EKDMasteryClass MasteryClass,
		EKDMasteryBranch MasteryBranch,
		FText DisplayName,
		FText Description,
		const TMap<EKDMasteryTier, int32>& TierUnlockThresholds,
		const TArray<FKDMasterySlot>& Slots);
};
