// Copyright Kingdawn. All Rights Reserved.
// Creation flow controller — state machine for multi-stage character creation.
// Owns stage transitions, accumulates selection data, and fires delegates for UI binding.
// Does NOT own any widgets, preview logic, or rendering — those stay in GameInstance / screen classes.
// Uses the existing ECreationStage enum from TainlordCharacterCustomizationTypes.h.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TainlordCreationFlowTypes.h"
#include "TainlordCreationFlowController.generated.h"

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

/**
 * Fired when the flow transitions to a new stage.
 * GameInstance listens to this to open/close the appropriate screen widget.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCreationStageChanged,
	ECreationStage, OldStage,
	ECreationStage, NewStage
);

/**
 * Fired when the entire creation flow completes (reached Confirm stage and validated).
 * GameInstance listens to this to save the profile and close the flow.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCreationFlowCompleted,
	const FTainlordProfileData&, CompletedProfile
);

/**
 * Fired when the creation flow is cancelled (back from first stage).
 * GameInstance listens to this to close all widgets.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreationFlowAborted);

/**
 * Creation Flow Controller.
 *
 * Pure C++ state machine that manages the character creation flow.
 * The GameInstance creates this object and binds to its delegates.
 * Screen widgets call into this controller to commit their selections.
 *
 * Responsibilities:
 * - Track current stage (ECreationStage)
 * - Validate stage transitions (Next/Back)
 * - Accumulate selection data into WorkingProfile
 * - Fire delegates on stage change, completion, or abort
 * - Assemble final FTainlordProfileData when flow completes
 *
 * Does NOT:
 * - Create or manage widgets
 * - Touch preview character or rendering
 * - Know about specific widget classes
 *
 * Usage from GameInstance:
 *   FlowController = NewObject<UTainlordCreationFlowController>(this);
 *   FlowController->OnStageChanged.AddDynamic(this, &HandleStageChanged);
 *   FlowController->OnFlowCompleted.AddDynamic(this, &HandleFlowCompleted);
 *   FlowController->BeginFlow();
 *
 * Usage from screen widgets (via GameInstance pass-through):
 *   FlowController->CommitRaceSelection(ECharacterRace::Elf);
 *   FlowController->AdvanceToNextStage();
 */
UCLASS(BlueprintType)
class TAINLORD_API UTainlordCreationFlowController : public UObject
{
	GENERATED_BODY()

public:

	// -----------------------------------------------------------------------
	// Delegates
	// -----------------------------------------------------------------------

	/** Fired on every stage transition. Bind to open/close screen widgets. */
	UPROPERTY(BlueprintAssignable, Category = "Creation|Flow")
	FOnCreationStageChanged OnStageChanged;

	/** Fired when the flow reaches the Confirm stage and profile is validated. */
	UPROPERTY(BlueprintAssignable, Category = "Creation|Flow")
	FOnCreationFlowCompleted OnFlowCompleted;

	/** Fired when the flow is aborted (back from first stage or explicit cancel). */
	UPROPERTY(BlueprintAssignable, Category = "Creation|Flow")
	FOnCreationFlowAborted OnFlowAborted;

	// -----------------------------------------------------------------------
	// Flow lifecycle
	// -----------------------------------------------------------------------

	/**
	 * Start the creation flow from Stage 1 (RaceSelect).
	 * Resets all accumulated state and fires OnStageChanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow")
	void BeginFlow();

	/**
	 * Advance to the next stage in the flow.
	 * If current stage is Confirm, fires OnFlowCompleted with the assembled profile.
	 * Does nothing if the flow is not active.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow")
	void AdvanceToNextStage();

	/**
	 * Go back to the previous stage.
	 * If at the first stage (RaceSelect), fires OnFlowAborted.
	 * Does nothing if the flow is not active.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow")
	void ReturnToPreviousStage();

	/**
	 * Cancel the flow entirely. Fires OnFlowAborted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow")
	void AbortFlow();

	// -----------------------------------------------------------------------
	// Stage data commits
	// -----------------------------------------------------------------------

	/** Commit race and gender selection from Stage 1 (RaceSelect). */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow|Commit")
	void CommitRaceSelection(ECharacterRace Race, ECharacterGender Gender);

	/** Commit appearance data from Stage 2 (Appearance). */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow|Commit")
	void CommitAppearanceData(const FTainlordAppearanceData& Appearance);

	/** Commit mastery selection from Stage 3 (Mastery). */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow|Commit")
	void CommitMasterySelection(FName MasteryId);

	/** Commit city selection from Stage 4 (City). */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow|Commit")
	void CommitCitySelection(FName CityId);

	/** Commit character name from Stage 5 (Name). */
	UFUNCTION(BlueprintCallable, Category = "Creation|Flow|Commit")
	void CommitCharacterName(const FString& Name);

	// -----------------------------------------------------------------------
	// Queries
	// -----------------------------------------------------------------------

	/** Get the current stage. */
	UFUNCTION(BlueprintPure, Category = "Creation|Flow")
	ECreationStage GetCurrentStage() const { return FlowState.CurrentStage; }

	/** Get the full flow state (read-only). */
	UFUNCTION(BlueprintPure, Category = "Creation|Flow")
	const FTainlordCreationFlowState& GetFlowState() const { return FlowState; }

	/** Get the working profile being assembled. */
	UFUNCTION(BlueprintPure, Category = "Creation|Flow")
	const FTainlordProfileData& GetWorkingProfile() const { return FlowState.WorkingProfile; }

	/** Is the flow currently active? */
	UFUNCTION(BlueprintPure, Category = "Creation|Flow")
	bool IsFlowActive() const { return FlowState.IsFlowActive(); }

	/**
	 * Check whether a specific stage should be SKIPPED in the current build.
	 * Used to skip Mastery/City until their UI is implemented.
	 * Reads from StagesToSkip set.
	 */
	UFUNCTION(BlueprintPure, Category = "Creation|Flow")
	bool ShouldSkipStage(ECreationStage Stage) const;

	// -----------------------------------------------------------------------
	// Configuration
	// -----------------------------------------------------------------------

	/**
	 * Stages to skip during flow advancement.
	 * Set this before BeginFlow() to skip Mastery/City until UI is ready.
	 * Default: Mastery, City, and Name are skipped (current production flow is Race → Appearance → Confirm).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creation|Flow|Config")
	TSet<ECreationStage> StagesToSkip;

private:

	/** Transition to a new stage, handling skip logic. */
	void TransitionToStage(ECreationStage NewStage);

	/**
	 * Resolve the actual next stage, skipping any stages in StagesToSkip.
	 * E.g. if Mastery is skipped, Appearance → City (or Name if City is also skipped).
	 */
	ECreationStage ResolveNextStage(ECreationStage FromStage) const;

	/**
	 * Resolve the actual previous stage, skipping any stages in StagesToSkip.
	 */
	ECreationStage ResolvePreviousStage(ECreationStage FromStage) const;

	/** Accumulated creation flow state. */
	UPROPERTY(Transient)
	FTainlordCreationFlowState FlowState;
};
