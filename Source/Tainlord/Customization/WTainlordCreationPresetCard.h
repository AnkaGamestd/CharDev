// Copyright Kingdawn. All Rights Reserved.
// Single preset card widget for character creation.
// Displays one option (head, hair, beard, or skin tone) as a selectable card.
// Blueprint subclass provides visual layout; C++ owns state and selection logic.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "WTainlordCreationPresetCard.generated.h"

/**
 * Delegate broadcast when this card is clicked/selected.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPresetCardSelected, class UWTainlordCreationPresetCard*, Card);

/**
 * Category type for the preset card. Determines what kind of data the card represents.
 */
UENUM(BlueprintType)
enum class EPresetCategory : uint8
{
	Head UMETA(DisplayName = "Head"),
	Hair UMETA(DisplayName = "Hair"),
	Beard UMETA(DisplayName = "Beard"),
	SkinTone UMETA(DisplayName = "Skin Tone")
};

/**
 * Single preset card widget for the character creation preset grid.
 *
 * Design:
 *   - C++ owns the preset ID, category, display name, and selection state.
 *   - Blueprint provides visual layout (thumbnail, name label, selection border).
 *   - Blueprint binds widgets via meta=(BindWidget) naming convention.
 *   - When clicked, broadcasts OnPresetCardSelected; parent grid handles the rest.
 *
 * Required Widget Names in Blueprint:
 *   - CardButton (UButton) — clickable area
 *   - PresetName (UTextBlock) — display name
 *   - SelectionBorder (UBorder) — highlights when selected
 *   - PresetThumbnail (UImage) — optional thumbnail image
 */
UCLASS(Blueprintable, BlueprintType)
class TAINLORD_API UWTainlordCreationPresetCard : public UUserWidget
{
	GENERATED_BODY()

public:
	UWTainlordCreationPresetCard(const FObjectInitializer& ObjectInitializer);

	// --- Events ---

	/** Broadcast when this card is clicked. Parent grid uses this to update selection. */
	UPROPERTY(BlueprintAssignable, Category = "Creation|Card")
	FOnPresetCardSelected OnPresetCardSelected;

	// --- Setup ---

	/**
	 * Initialize this card with a preset ID and display info.
	 * Called by the parent grid when populating cards.
	 *
	 * @param InPresetId The stable ID this card represents (maps to catalog entry).
	 * @param InCategory Which category this card belongs to.
	 * @param InDisplayName Human-readable name for the UI.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Card")
	void SetupCard(FName InPresetId, EPresetCategory InCategory, const FText& InDisplayName);

	/**
	 * Setup overload for skin tone cards that also carries the color.
	 * The Blueprint can use this color to render a color swatch.
	 */
	UFUNCTION(BlueprintCallable, Category = "Creation|Card")
	void SetupSkinToneCard(FName InPresetId, const FText& InDisplayName, const FLinearColor& InColor);

	// --- State ---

	/** The stable preset ID this card represents. */
	UFUNCTION(BlueprintPure, Category = "Creation|Card")
	FName GetPresetId() const { return PresetId; }

	/** Which category this card belongs to. */
	UFUNCTION(BlueprintPure, Category = "Creation|Card")
	EPresetCategory GetCategory() const { return Category; }

	/** Whether this card is currently selected. */
	UFUNCTION(BlueprintPure, Category = "Creation|Card")
	bool IsSelected() const { return bIsSelected; }

	/** Get the display name. */
	UFUNCTION(BlueprintPure, Category = "Creation|Card")
	FText GetDisplayName() const { return DisplayName; }

	/** Get the skin tone color (only valid for SkinTone category cards). */
	UFUNCTION(BlueprintPure, Category = "Creation|Card")
	FLinearColor GetSkinToneColor() const { return SkinToneColor; }

	/** Whether this is a "None" card (represents clearing the slot). */
	UFUNCTION(BlueprintPure, Category = "Creation|Card")
	bool IsNoneCard() const { return PresetId.IsNone(); }

	// --- Selection ---

	/** Set the selection state. Called by parent grid. Updates visual state. */
	UFUNCTION(BlueprintCallable, Category = "Creation|Card")
	void SetSelected(bool bInSelected);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	// --- Bound Widgets (Blueprint must provide by name) ---

	/** Clickable button area for the card. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UButton> CardButton;

	/** Text block showing the preset name. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> PresetName;

	/** Border used to show selection highlight. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UBorder> SelectionBorder;

	/** Optional thumbnail image for the preset. */
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<class UImage> PresetThumbnail;

	// --- Blueprint-implementable visual hooks ---

	/** Called when selection state changes. Blueprint updates visual feedback. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Card", DisplayName = "OnSelectionChanged")
	void BP_OnSelectionChanged(bool bNewSelected);

	/** Called when the card is set up. Blueprint can update thumbnail or color swatch. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Card", DisplayName = "OnCardSetup")
	void BP_OnCardSetup();

private:
	/** Handler for the card button click. */
	UFUNCTION()
	void OnCardClicked();

	/** The stable preset ID. NAME_None means "clear this slot". */
	UPROPERTY(Transient)
	FName PresetId;

	/** Category this card belongs to. */
	UPROPERTY(Transient)
	EPresetCategory Category = EPresetCategory::Head;

	/** Whether this card is currently selected. */
	UPROPERTY(Transient)
	bool bIsSelected = false;

	/** Display name for the UI. */
	UPROPERTY(Transient)
	FText DisplayName;

	/** Skin tone color (only used for SkinTone category). */
	UPROPERTY(Transient)
	FLinearColor SkinToneColor = FLinearColor::White;
};
