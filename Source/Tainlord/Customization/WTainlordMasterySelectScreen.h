// Copyright Kingdawn. All Rights Reserved.
// Mastery selection screen - third stage of character creation flow.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "Domain/TainlordMasteryTypes.h"
#include "WTainlordMasterySelectScreen.generated.h"

class UButton;
class UTextBlock;
struct FTainlordMasteryDefinition;

/** Broadcast when mastery is confirmed and flow should proceed to next stage. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMasteryConfirmed);

/** Broadcast when mastery selection is cancelled (Back button). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMasterySelectCancelled);

/**
 * Mastery selection screen widget.
 * Third stage of character creation flow.
 * Player selects mastery/build, which determines archetype and stat orientation.
 * On confirm, proceeds to the next stage (City or Confirm if City is skipped).
 *
 * Per product flow:
 * 1. RaceSelectScreen → pick race
 * 2. AppearanceScreen → pick head/hair/beard/skin
 * 3. MasterySelectScreen (this) → pick mastery/build
 * 4. CityScreen → pick starting city (future)
 * 5. NameScreen → enter character name (future)
 * 6. Confirm → save profile
 */
UCLASS(Abstract)
class TAINLORD_API UWTainlordMasterySelectScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UWTainlordMasterySelectScreen(const FObjectInitializer& ObjectInitializer);

	// --- Delegates ---

	/** Broadcast when mastery is confirmed and flow should proceed. */
	UPROPERTY(BlueprintAssignable, Category = "Creation|MasterySelect")
	FOnMasteryConfirmed OnMasteryConfirmed;

	/** Broadcast when mastery select is cancelled (Back button). */
	UPROPERTY(BlueprintAssignable, Category = "Creation|MasterySelect")
	FOnMasterySelectCancelled OnMasterySelectCancelled;

	// --- Public API ---

	/**
	 * Initialize the mastery select screen.
	 * Loads demo masteries and populates the mastery grid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|MasterySelect")
	void InitMasterySelect();

	/** Get the currently selected mastery ID. */
	UFUNCTION(BlueprintPure, Category = "Creation|MasterySelect")
	FName GetSelectedMasteryId() const { return SelectedMasteryId; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	// --- Bound widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MasteryButton0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MasteryButton1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MasteryButton2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MasteryButton3;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MasteryButton4;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MasteryButton5;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> TitleLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> SelectedMasteryLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> MasteryDescriptionLabel;

	// --- Blueprint visual hooks ---

	/** Called when mastery selection changes visually. Update UI in BP. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|MasterySelect", DisplayName = "OnMasteryChanged")
	void BP_OnMasteryChanged(const FTainlordMasteryDefinition& NewMastery);

private:
	// --- Button handlers ---

	UFUNCTION() void OnMasteryButton0Clicked();
	UFUNCTION() void OnMasteryButton1Clicked();
	UFUNCTION() void OnMasteryButton2Clicked();
	UFUNCTION() void OnMasteryButton3Clicked();
	UFUNCTION() void OnMasteryButton4Clicked();
	UFUNCTION() void OnMasteryButton5Clicked();
	UFUNCTION() void OnConfirmClicked();
	UFUNCTION() void OnBackClicked();

	/** Set button text helper (null-safe). */
	void SetButtonText(UButton* Button, const FString& Text);

	/** Select a mastery by ID and update UI. */
	void SelectMastery(FName MasteryId);

	/** Update the selected mastery and refresh UI. */
	void RefreshMasteryDisplay();

	// --- Visual polish helpers ---

	/** Apply selected/neutral highlight to all mastery buttons based on current selection. */
	void UpdateButtonHighlights();

	/** Apply a highlight or neutral color to a single button. */
	void ApplyButtonHighlight(UButton* Button, bool bSelected);

	/** Force uniform size on all mastery buttons. */
	void ApplyUniformButtonSizes();

	/** Reduce font sizes on all text widgets. */
	void ApplyFontSizes();

	/** Set font size on a TextBlock (null-safe). */
	static void SetTextBlockFontSize(UTextBlock* TextBlock, int32 Size);

	/** Set font size on the TextBlock child inside a Button (null-safe). */
	static void SetButtonFontSize(UButton* Button, int32 Size);

	// --- State ---

	/** Currently selected mastery ID. */
	UPROPERTY(Transient)
	FName SelectedMasteryId = NAME_None;

	/** Cached mastery definitions from demo catalog. */
	UPROPERTY(Transient)
	TArray<FTainlordMasteryDefinition> CachedMasteries;

	/** Map button index to mastery ID for click handling. */
	UPROPERTY(Transient)
	TArray<FName> ButtonIndexToMasteryId;
};
