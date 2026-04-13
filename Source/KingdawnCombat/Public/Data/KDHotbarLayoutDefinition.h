// Copyright Kingdawn. All Rights Reserved.
// Data asset describing a fixed hotbar layout for a build.
// Maps slot indices to gameplay tags so runtime code can resolve
// the corresponding abilities without hard-wiring indices.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KDHotbarLayoutDefinition.generated.h"

/**
 * Single hotbar slot entry that pairs a slot index with an ability tag.
 * Designers populate the Slots array in order; the BarIndex identifies
 * which hotbar bar (0-based) this layout targets.
 */
USTRUCT(BlueprintType)
struct KINGDAWNCOMBAT_API FKDHotbarSlotEntry
{
	GENERATED_BODY()

	/** Gameplay tag that resolves to the ability assigned to this slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Hotbar")
	FGameplayTag AbilityTag;

	FKDHotbarSlotEntry()
	{
	}

	explicit FKDHotbarSlotEntry(const FGameplayTag& InTag)
		: AbilityTag(InTag)
	{
	}
};

/**
 * Defines the ability layout for a single hotbar bar.
 *
 * Each UKDBuildDefinition references one UKDHotbarLayoutDefinition to
 * pre-populate the player's hotbar when the build is activated.
 * The Slots array is ordered: index 0 = slot 0, index 1 = slot 1, etc.
 * Empty tags mean the slot is unassigned.
 */
UCLASS(BlueprintType)
class KINGDAWNCOMBAT_API UKDHotbarLayoutDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UKDHotbarLayoutDefinition();

	/** Zero-based index of the hotbar bar this layout describes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Hotbar")
	int32 BarIndex;

	/** Ordered list of slot entries. Index in array == slot index on the bar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Hotbar")
	TArray<FKDHotbarSlotEntry> Slots;
};
