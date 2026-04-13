// Copyright Kingdawn. All Rights Reserved.
// City selection screen - Stage 4 of character creation flow.
// Data source: UTainlordDemoCatalogLibrary::GetDemoCities()
// Selection uses stable FName IDs. No preview/render coupling.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Domain/TainlordCityTypes.h"
#include "WTainlordCitySelectScreen.generated.h"

class UButton;
class UTextBlock;
class UPanelWidget;

/** Broadcast when a city is confirmed and flow should proceed to the next stage. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCityConfirmed);

/** Broadcast when city select is cancelled (Back button). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCitySelectCancelled);

/**
 * City selection screen widget.
 * Stage 4 of character creation flow.
 * Player selects a starting city from the demo catalog.
 * On confirm, commits selection to the flow controller and advances.
 *
 * Data source: UTainlordDemoCatalogLibrary::GetDemoCities()
 * All city entries use stable FName IDs (e.g., "Ironhold", "Ashfall").
 *
 * Layout (enforced at runtime by SetupLayout):
 *   LEFT: City info panel (name, description, allegiance, reputation bonuses)
 *   CENTER: City selection buttons (one per demo city)
 *   BOTTOM: Confirm / Back buttons
 *
 * Blueprint provides visual layout only. C++ owns all logic.
 */
UCLASS(Abstract)
class TAINLORD_API UWTainlordCitySelectScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UWTainlordCitySelectScreen(const FObjectInitializer& ObjectInitializer);

	// --- Delegates ---

	/** Broadcast when city is confirmed and flow should proceed to the next stage. */
	UPROPERTY(BlueprintAssignable, Category = "Creation|CitySelect")
	FOnCityConfirmed OnCityConfirmed;

	/** Broadcast when city select is cancelled (Back button). */
	UPROPERTY(BlueprintAssignable, Category = "Creation|CitySelect")
	FOnCitySelectCancelled OnCitySelectCancelled;

	// --- Public API ---

	/**
	 * Initialize the city select screen.
	 * Loads demo cities from UTainlordDemoCatalogLibrary and populates buttons.
	 * Sets default city to the first recommended city (or first available).
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|CitySelect")
	void InitCitySelect();

	/** Get the currently selected city ID (stable FName). */
	UFUNCTION(BlueprintPure, Category = "Creation|CitySelect")
	FName GetSelectedCityId() const { return SelectedCityId; }

	/** Get the currently selected city definition (read-only). */
	UFUNCTION(BlueprintPure, Category = "Creation|CitySelect")
	FTainlordCityDefinition GetSelectedCityDefinition() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	// --- Bound widgets (optional — C++ finds them flexibly) ---

	/** City selection buttons (fixed slots, up to 4 demo cities). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CityButton0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CityButton1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CityButton2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CityButton3;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> TitleLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> CityNameLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> CityDescriptionLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> AllegianceLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ReputationLabel;

	// --- Blueprint visual hooks ---

	/** Called when city selection changes. Update info panel in BP. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|CitySelect", DisplayName = "OnCityChanged")
	void BP_OnCityChanged(FName CityId, const FTainlordCityDefinition& CityDef);

private:
	// --- Button handlers (UFUNCTION for dynamic delegate binding) ---

	UFUNCTION() void OnCityButton0Clicked();
	UFUNCTION() void OnCityButton1Clicked();
	UFUNCTION() void OnCityButton2Clicked();
	UFUNCTION() void OnCityButton3Clicked();
	UFUNCTION() void OnConfirmClicked();
	UFUNCTION() void OnBackClicked();

	/** Set button text helper (null-safe). */
	void SetButtonText(UButton* Button, const FString& Text);

	/** Select a city by ID and update UI. */
	void SelectCity(FName CityId);

	/** Update the info panel labels with the selected city's data. */
	void UpdateInfoPanel();

	/** Apply highlight to city buttons based on current selection. */
	void UpdateButtonHighlights();

	// --- Visual polish helpers ---

	/** Apply a highlight or neutral color to a single button. */
	void ApplyButtonHighlight(UButton* Button, bool bSelected);

	/** Force uniform size on all city buttons. */
	void ApplyUniformButtonSizes();

	/** Reduce font sizes on all text widgets. */
	void ApplyFontSizes();

	/** Set font size on a TextBlock (null-safe). */
	static void SetTextBlockFontSize(UTextBlock* TextBlock, int32 Size);

	/** Set font size on the TextBlock child inside a Button (null-safe). */
	static void SetButtonFontSize(UButton* Button, int32 Size);

	// --- State ---

	/** Currently selected city stable ID. */
	UPROPERTY(Transient)
	FName SelectedCityId;

	/** Cached demo cities loaded from catalog. */
	UPROPERTY(Transient)
	TArray<FTainlordCityDefinition> CachedCities;

	/** Map button index to city ID for click handling. */
	UPROPERTY(Transient)
	TArray<FName> ButtonIndexToCityId;
};
