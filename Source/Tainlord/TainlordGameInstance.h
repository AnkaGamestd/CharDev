// Copyright Kingdawn. All Rights Reserved.
// Tainlord Game Instance: binds creation flow delegate, manages global state.
// Now uses UTainlordCreationFlowController for stage management.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordGameInstance.generated.h"

class UUserWidget;
class UWTainlordRaceSelectScreen;
class UWTainlordCreationScreen;
class UWTainlordMasterySelectScreen;
class UWTainlordCitySelectScreen;
class UTainlordCreationFlowController;

/**
 * Tainlord Game Instance.
 *
 * Responsibilities:
 * - Bind to UTainlordCharacterProfileSubsystem::OnCreationFlowNeeded
 * - Create and own a UTainlordCreationFlowController for stage management
 * - Listen to OnStageChanged / OnFlowCompleted / OnFlowAborted to open/close widgets
 * - Hold global game state (future: online session, etc.)
 *
 * The flow controller owns stage transitions and working profile accumulation.
 * This class owns widget lifecycle and input mode management only.
 */
UCLASS(Config=Game)
class TAINLORD_API UTainlordGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	/** Access the flow controller (e.g. from widgets that need to commit data). */
	UFUNCTION(BlueprintPure, Category = "Tainlord|CharacterCreation")
	UTainlordCreationFlowController* GetFlowController() const { return FlowController; }

protected:
	// --- Widget class references (configurable via DefaultGame.ini) ---

	/**
	 * Widget class for Stage 1: Race Select.
	 * If not set, falls back to /Game/Tainlord/UI/Creation/WBP_RaceSelectScreen.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tainlord|CharacterCreation")
	TSoftClassPtr<UUserWidget> RaceSelectWidgetClass;

	/**
	 * Widget class for Stage 2: Appearance Customization.
	 * If not set, falls back to WBP_CharacterCreationScreen_Fresh.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tainlord|CharacterCreation")
	TSoftClassPtr<UUserWidget> CreationWidgetClass;

	/**
	 * Widget class for Stage 3: Mastery Selection.
	 * If not set, falls back to WBP_MasterySelectScreen.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tainlord|CharacterCreation")
	TSoftClassPtr<UUserWidget> MasterySelectWidgetClass;

	/**
	 * Widget class for Stage 4: City Selection.
	 * If not set, falls back to WBP_CitySelectScreen.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tainlord|CharacterCreation")
	TSoftClassPtr<UUserWidget> CitySelectWidgetClass;

	// --- Profile subsystem callback ---

	/** Called when the profile subsystem detects no save exists. Starts the flow. */
	UFUNCTION()
	void OnCreationFlowNeeded();

	// --- Widget-level callbacks (bound to screen widgets) ---

	/** Called when race is confirmed on Stage 1. Commits to flow controller and advances. */
	UFUNCTION()
	void OnRaceConfirmed();

	/** Called when race select is cancelled. Tells flow controller to go back (abort). */
	UFUNCTION()
	void OnRaceSelectCancelled();

	/** Called when the appearance screen confirms. Commits appearance and advances. */
	UFUNCTION()
	void OnCreationConfirmed();

	/** Called when the appearance screen Back button is pressed. Returns to previous stage. */
	UFUNCTION()
	void OnCreationCancelled();

	// --- Mastery screen callbacks ---

	/** Called when mastery is confirmed on Stage 3. Commits to flow controller and advances. */
	UFUNCTION()
	void OnMasteryConfirmed();

	/** Called when mastery select is cancelled. Tells flow controller to go back. */
	UFUNCTION()
	void OnMasterySelectCancelled();

	// --- City screen callbacks ---

	/** Called when city is confirmed on Stage 4. Commits to flow controller and advances. */
	UFUNCTION()
	void OnCityConfirmed();

	/** Called when city select is cancelled. Tells flow controller to go back. */
	UFUNCTION()
	void OnCitySelectCancelled();

	// --- Flow controller callbacks ---

	/** Respond to flow stage transitions by opening/closing the appropriate widget. */
	UFUNCTION()
	void HandleStageChanged(ECreationStage OldStage, ECreationStage NewStage);

	/** Respond to flow completion by saving profile and closing widgets. */
	UFUNCTION()
	void HandleFlowCompleted(const FTainlordProfileData& CompletedProfile);

	/** Respond to flow abort by closing all widgets. */
	UFUNCTION()
	void HandleFlowAborted();

private:
	// --- Helpers ---

	/** Open Stage 1: Race Select. */
	void OpenRaceSelectScreen();

	/** Open Stage 2: Appearance Customization. */
	void OpenAppearanceScreen();

	/** Open Stage 3: Mastery Selection. */
	void OpenMasteryScreen();

	/** Open Stage 4: City Selection. */
	void OpenCityScreen();

	/** Close any active widget and restore game-only input mode. */
	void CloseActiveWidget();

	/** Set UI+Game input mode with mouse cursor visible. */
	void SetUIInputMode(UUserWidget* FocusWidget);

	/** Find preview character in the world. */
	AActor* FindPreviewCharacter() const;

	// --- State ---

	/** The flow controller instance. Created during Init(), destroyed during Shutdown(). */
	UPROPERTY(Transient)
	TObjectPtr<UTainlordCreationFlowController> FlowController;

	/** The currently open creation widget instance (race select or appearance). */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveCreationWidget;
};
