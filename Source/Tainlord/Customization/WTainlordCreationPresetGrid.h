// Copyright Kingdawn. All Rights Reserved.
// Preset grid widget for character creation.
// Populates cards from catalog data for a specific category (Head/Hair/Beard/SkinTone).
// C++ owns population, filtering, and selection; Blueprint provides grid layout.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "WTainlordCreationPresetCard.h"
#include "WTainlordCreationPresetGrid.generated.h"

class UUniformGridPanel;
class UUniformGridSlot;
class UGridPanel;
class UScrollBox;
class UWidgetSwitcher;

/**
 * Delegate broadcast when a preset is selected in this grid.
 * @param PresetId The stable ID of the selected preset (NAME_None = "clear slot").
 * @param Category Which category the selection belongs to.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPresetGridSelectionChanged, FName, PresetId, EPresetCategory, Category);

/**
 * Grid widget that displays preset cards for one customization category.
 *
 * Design:
 *   - C++ populates the grid from catalog data via PopulateForCategory().
 *   - C++ manages selection state (only one card selected at a time).
 *   - Blueprint provides the grid layout container (UniformGridPanel or WrapBox).
 *   - When a card is selected, broadcasts OnPresetGridSelectionChanged.
 *
 * Required Widget Names in Blueprint:
 *   - PresetGridContainer (UPanelWidget) — container for preset cards (UniformGridPanel, WrapBox, etc.)
 *
 * Optional Widget Names:
 *   - CategoryTitle (UTextBlock) — shows the category name
 *   - EmptyLabel (UTextBlock) — shown when no presets are available
 */
UCLASS(Blueprintable, BlueprintType)
class TAINLORD_API UWTainlordCreationPresetGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	UWTainlordCreationPresetGrid(const FObjectInitializer& ObjectInitializer);

	// --- Events ---

	/** Broadcast when the player selects a different preset in this grid. */
	UPROPERTY(BlueprintAssignable, Category = "Creation|Grid")
	FOnPresetGridSelectionChanged OnPresetGridSelectionChanged;

	// --- Configuration ---

	/** The card widget class to use when populating the grid.
	 *  Must be a Blueprint subclass of WTainlordCreationPresetCard.
	 *  Set this in Blueprint defaults or via SetCardClass(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creation|Grid")
	TSubclassOf<UWTainlordCreationPresetCard> CardWidgetClass;

	/** Number of columns in the grid layout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creation|Grid")
	int32 GridColumns = 4;

	// --- Population ---

	/**
	 * Populate the grid with head presets from the catalog.
	 * Clears existing cards and creates new ones from filtered catalog data.
	 *
	 * @param WorldContext For catalog lookup.
	 * @param Gender Current gender filter.
	 * @param Race Current race filter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Grid")
	void PopulateHeads(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race);

	/** Populate with hair presets. */
	UFUNCTION(BlueprintCallable, Category = "Creation|Grid")
	void PopulateHair(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race);

	/** Populate with beard presets. */
	UFUNCTION(BlueprintCallable, Category = "Creation|Grid")
	void PopulateBeards(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race);

	/** Populate with skin tone presets. */
	UFUNCTION(BlueprintCallable, Category = "Creation|Grid")
	void PopulateSkinTones(const UObject* WorldContext);

	/**
	 * Clear all cards from the grid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Grid")
	void ClearGrid();

	// --- Selection ---

	/** Get the currently selected preset ID. NAME_None if nothing selected. */
	UFUNCTION(BlueprintPure, Category = "Creation|Grid")
	FName GetSelectedPresetId() const { return SelectedPresetId; }

	/** Get the current category this grid is showing. */
	UFUNCTION(BlueprintPure, Category = "Creation|Grid")
	EPresetCategory GetCurrentCategory() const { return CurrentCategory; }

	/** Set the selected preset by ID. Updates card visuals. */
	UFUNCTION(BlueprintCallable, Category = "Creation|Grid")
	bool SetSelectedPreset(FName PresetId);

	/** Get the number of preset cards currently in the grid. */
	UFUNCTION(BlueprintPure, Category = "Creation|Grid")
	int32 GetCardCount() const { return Cards.Num(); }

protected:
	virtual void NativeOnInitialized() override;

	// --- Bound Widgets ---

	/** Container widget for the preset cards. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UPanelWidget> PresetGridContainer;

	/** Optional category title label. */
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<class UTextBlock> CategoryTitle;

	/** Optional empty state label. */
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<class UTextBlock> EmptyLabel;

	// --- Blueprint hooks ---

	/** Called after grid is populated. Blueprint can animate or update layout. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Grid", DisplayName = "OnGridPopulated")
	void BP_OnGridPopulated(int32 CardCount);

	/** Called when selection changes. Blueprint can update visual feedback. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Grid", DisplayName = "OnGridSelectionChanged")
	void BP_OnGridSelectionChanged(FName NewPresetId);

private:
	/** Handler for card selection events. */
	UFUNCTION()
	void OnCardSelected(UWTainlordCreationPresetCard* Card);

	/** Create a single card widget and add it to the grid. */
	UWTainlordCreationPresetCard* CreateCard(FName PresetId, EPresetCategory Category, const FText& DisplayName);

	/** Create a skin tone card with color swatch support. */
	UWTainlordCreationPresetCard* CreateSkinToneCard(FName PresetId, const FText& DisplayName, const FLinearColor& Color);

	/** Add a card widget to the grid container at the correct position. */
	void AddCardToGrid(UWTainlordCreationPresetCard* Card);

	/** Currently active category. */
	UPROPERTY(Transient)
	EPresetCategory CurrentCategory = EPresetCategory::Head;

	/** Currently selected preset ID. */
	UPROPERTY(Transient)
	FName SelectedPresetId;

	/** All card widgets currently in the grid. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWTainlordCreationPresetCard>> Cards;
};
