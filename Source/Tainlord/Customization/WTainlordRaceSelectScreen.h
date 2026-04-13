// Copyright Kingdawn. All Rights Reserved.
// Race selection screen - first stage of character creation flow.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "WTainlordRaceSelectScreen.generated.h"

class ATainlordPreviewCharacter;
class UButton;
class UTextBlock;

/** Broadcast when race is confirmed and flow should proceed to appearance screen. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceConfirmed);

/** Broadcast when race select is cancelled (Back button). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceSelectCancelled);

/**
 * Race selection screen widget.
 * First stage of character creation flow.
 * Player selects race, preview character updates in real-time.
 * On confirm, proceeds to appearance customization screen.
 *
 * Per product flow:
 * 1. RaceSelectScreen (this) → pick race, preview updates
 * 2. AppearanceScreen (WTainlordCreationScreen) → pick head/hair/beard/skin, name
 */
UCLASS(Abstract)
class TAINLORD_API UWTainlordRaceSelectScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UWTainlordRaceSelectScreen(const FObjectInitializer& ObjectInitializer);

	// --- Delegates ---

	/** Broadcast when race is confirmed and flow should proceed to appearance screen. */
	UPROPERTY(BlueprintAssignable, Category = "Creation|RaceSelect")
	FOnRaceConfirmed OnRaceConfirmed;

	/** Broadcast when race select is cancelled (Back button). */
	UPROPERTY(BlueprintAssignable, Category = "Creation|RaceSelect")
	FOnRaceSelectCancelled OnRaceSelectCancelled;

	// --- Public API ---

	/**
	 * Initialize the race select screen.
	 * Sets default race to Human and updates preview.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|RaceSelect")
	void InitRaceSelect();

	/** Get the currently selected race. */
	UFUNCTION(BlueprintPure, Category = "Creation|RaceSelect")
	ECharacterRace GetSelectedRace() const { return SelectedRace; }

	/** Preview character reference (set by GameInstance before opening screen). */
	UPROPERTY(BlueprintReadWrite, Category = "Creation|RaceSelect")
	TObjectPtr<AActor> PreviewCharacter;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	// --- Bound widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceHumanButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceElfButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceDwarfButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceOrcButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> TitleLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> SelectedRaceLabel;

	// --- Blueprint visual hooks ---

	/** Called when race selection changes visually. Update button highlights in BP. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|RaceSelect", DisplayName = "OnRaceChanged")
	void BP_OnRaceChanged(ECharacterRace NewRace);

private:
	// --- Button handlers ---

	UFUNCTION() void OnRaceHumanClicked();
	UFUNCTION() void OnRaceElfClicked();
	UFUNCTION() void OnRaceDwarfClicked();
	UFUNCTION() void OnRaceOrcClicked();
	UFUNCTION() void OnConfirmClicked();
	UFUNCTION() void OnBackClicked();

	/** Set button text helper (null-safe). */
	void SetButtonText(UButton* Button, const FString& Text);

	/** Update the selected race and refresh preview. */
	void SelectRace(ECharacterRace Race);

	/** Apply selected race to preview character (shows default body for that race). */
	void RefreshPreview();

	// --- Visual polish helpers ---

	/** Apply selected/neutral highlight to all race buttons based on current SelectedRace. */
	void UpdateButtonHighlights();

	/** Apply a highlight or neutral color to a single button. */
	void ApplyButtonHighlight(UButton* Button, bool bSelected);

	/** Force uniform size on all race buttons. */
	void ApplyUniformButtonSizes();

	/** Reduce font sizes on all text widgets. */
	void ApplyFontSizes();

	/** Set font size on a TextBlock (null-safe). */
	static void SetTextBlockFontSize(UTextBlock* TextBlock, int32 Size);

	/** Set font size on the TextBlock child inside a Button (null-safe). */
	static void SetButtonFontSize(UButton* Button, int32 Size);

	// --- State ---

	/** Currently selected race. */
	UPROPERTY(Transient)
	ECharacterRace SelectedRace = ECharacterRace::Human;
};
