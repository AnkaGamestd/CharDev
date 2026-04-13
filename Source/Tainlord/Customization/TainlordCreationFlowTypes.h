// Copyright Kingdawn. All Rights Reserved.
// Creation flow state contract — runtime state tracker for the multi-stage flow.
// Uses the existing ECreationStage enum from TainlordCharacterCustomizationTypes.h.
// This file owns ONLY the flow state struct — no UI, no preview.

#pragma once

#include "CoreMinimal.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCreationFlowTypes.generated.h"

// ---------------------------------------------------------------------------
// Flow stage navigation helpers
// ---------------------------------------------------------------------------

/**
 * Static helper for navigating between creation stages.
 * Centralizes the stage ordering logic so both the flow controller
 * and any future systems can query next/previous without hardcoding order.
 *
 * Uses the existing ECreationStage enum (RaceSelect → Appearance → Mastery → City → Name → Confirm).
 */
USTRUCT(BlueprintType)
struct FCreationStageNavigation
{
	GENERATED_BODY()

	/**
	 * Returns the next stage after the given one.
	 * Returns Confirm for the last meaningful stage.
	 * Returns RaceSelect if None/Confirm is passed (defensive).
	 */
	static ECreationStage GetNextStage(ECreationStage Stage)
	{
		switch (Stage)
		{
		case ECreationStage::RaceSelect:  return ECreationStage::Appearance;
		case ECreationStage::Appearance:  return ECreationStage::Mastery;
		case ECreationStage::Mastery:     return ECreationStage::City;
		case ECreationStage::City:        return ECreationStage::Name;
		case ECreationStage::Name:        return ECreationStage::Confirm;
		case ECreationStage::Confirm:
		default:
			return ECreationStage::Confirm;
		}
	}

	/**
	 * Returns the previous stage (for Back navigation).
	 * Returns RaceSelect if at the first stage.
	 */
	static ECreationStage GetPreviousStage(ECreationStage Stage)
	{
		switch (Stage)
		{
		case ECreationStage::Confirm:     return ECreationStage::Name;
		case ECreationStage::Name:        return ECreationStage::City;
		case ECreationStage::City:        return ECreationStage::Mastery;
		case ECreationStage::Mastery:     return ECreationStage::Appearance;
		case ECreationStage::Appearance:  return ECreationStage::RaceSelect;
		case ECreationStage::RaceSelect:
		default:
			return ECreationStage::RaceSelect;
		}
	}

	/** Returns true if this is the first stage in the flow. */
	static bool IsFirstStage(ECreationStage Stage) { return Stage == ECreationStage::RaceSelect; }

	/** Returns true if this is the terminal confirmation stage. */
	static bool IsTerminalStage(ECreationStage Stage) { return Stage == ECreationStage::Confirm; }
};

// ---------------------------------------------------------------------------
// Flow state struct
// ---------------------------------------------------------------------------

/**
 * Tracks the current runtime state of the creation flow.
 * Owned by UTainlordCreationFlowController.
 * Each stage writes its selection into the FlowState's FTainlordProfileData.
 * On Confirm, the accumulated profile is validated and saved.
 */
USTRUCT(BlueprintType)
struct FTainlordCreationFlowState
{
	GENERATED_BODY()

	/** Which stage the flow is currently on. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creation|Flow")
	ECreationStage CurrentStage = ECreationStage::RaceSelect;

	/** Whether the flow is currently active (player is in creation screens). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creation|Flow")
	bool bIsFlowActive = false;

	/** The accumulated profile being built during creation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creation|Flow")
	FTainlordProfileData WorkingProfile;

	/** Returns true if the flow is actively running. */
	bool IsFlowActive() const { return bIsFlowActive; }
};
