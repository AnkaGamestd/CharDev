// Copyright Kingdawn. All Rights Reserved.
// Appearance customization screen (Stage 2 of character creation flow).
// Race is already selected in Stage 1 (WTainlordRaceSelectScreen) and passed in.
// This screen handles: name, gender, head/hair/beard/skin preset browsing, preview.
// C++ owns all logic; Blueprint provides visual layout only.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "WTainlordCreationPresetCard.h"
#include "WTainlordCreationScreen.generated.h"

class UPanelWidget;
class UButton;
class UTextBlock;
class UEditableTextBox;
class UScrollBox;
class UWrapBox;
class ATainlordPreviewCharacter;

/**
 * Delegate broadcast when character creation is confirmed and saved.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreationConfirmed);

/**
 * Delegate broadcast when creation is cancelled.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreationCancelled);

/**
 * Main character creation screen.
 *
 * Layout (enforced at runtime by SetupLayout):
 *   LEFT: Character Name, Gender buttons
 *   CENTER-LEFT: Preset Grid (cards) with category label above
 *   CENTER: Open preview area (3D character)
 *   RIGHT: Appearance category buttons (Head/Hair/Beard/Skin Tone)
 *   BOTTOM: Confirm / Back buttons
 *
 * NOTE: Race selection has moved to Stage 1 (WTainlordRaceSelectScreen).
 *       Race widgets are kept for backward compat but collapsed at runtime.
 */
UCLASS(Blueprintable, BlueprintType)
class TAINLORD_API UWTainlordCreationScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UWTainlordCreationScreen(const FObjectInitializer& ObjectInitializer);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Creation|Screen")
	FOnCreationConfirmed OnCreationConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Creation|Screen")
	FOnCreationCancelled OnCreationCancelled;

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creation|Screen")
	TObjectPtr<AActor> PreviewCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creation|Screen")
	TSubclassOf<UWTainlordCreationPresetCard> CardWidgetClass;

	// --- API ---

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void InitCreationScreen();

	UFUNCTION(BlueprintPure, Category = "Creation|Screen")
	const FTainlordProfileData& GetWorkingProfile() const { return WorkingProfile; }

	UFUNCTION(BlueprintPure, Category = "Creation|Screen")
	EPresetCategory GetActiveCategory() const { return ActiveCategory; }

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void SetCharacterName(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void SetGender(ECharacterGender Gender);

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void SetRace(ECharacterRace Race);

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void SwitchCategory(EPresetCategory Category);

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void OnConfirmClicked();

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void OnBackClicked();

	UFUNCTION(BlueprintCallable, Category = "Creation|Screen")
	void RandomizeAppearance();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> CharacterNameInput;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> GenderMaleButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> GenderFemaleButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceHumanButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceElfButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceDwarfButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RaceOrcButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CategoryHeadButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CategoryHairButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CategoryBeardButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CategorySkinToneButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> PresetGridContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ValidationMessage;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> GenderLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> RaceLabel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ActiveCategoryLabel;

	// --- Blueprint visual hooks ---

	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Screen", DisplayName = "OnGenderChanged")
	void BP_OnGenderChanged(ECharacterGender NewGender);

	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Screen", DisplayName = "OnRaceChanged")
	void BP_OnRaceChanged(ECharacterRace NewRace);

	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Screen", DisplayName = "OnCategorySwitched")
	void BP_OnCategorySwitched(EPresetCategory NewCategory);

	UFUNCTION(BlueprintImplementableEvent, Category = "Creation|Screen", DisplayName = "OnValidationResult")
	void BP_OnValidationResult(bool bIsValid, const FString& Message);

private:
	// --- Button handlers ---

	UFUNCTION() void OnGenderMaleClicked();
	UFUNCTION() void OnGenderFemaleClicked();
	UFUNCTION() void OnRaceHumanClicked();
	UFUNCTION() void OnRaceElfClicked();
	UFUNCTION() void OnRaceDwarfClicked();
	UFUNCTION() void OnRaceOrcClicked();
	UFUNCTION() void OnCategoryHeadClicked();
	UFUNCTION() void OnCategoryHairClicked();
	UFUNCTION() void OnCategoryBeardClicked();
	UFUNCTION() void OnCategorySkinToneClicked();
	UFUNCTION() void OnNameChanged(const FText& NewText);
	UFUNCTION() void OnNameCommitted(const FText& NewText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnPresetCardSelected(UWTainlordCreationPresetCard* Card);

	// --- Internal helpers ---

	void RefreshPresetGrid();
	void RefreshPreview();
	void RefreshPreviewSlot(EPresetCategory Category, FName PresetId);
	void HandleContextChange();
	void SetValidationMessage(const FString& Message, bool bIsError);
	static FName CategoryToSlotName(EPresetCategory Category);
	void ClearPresetGrid();
	UWTainlordCreationPresetCard* AddPresetCard(FName PresetId, EPresetCategory Category, const FText& DisplayName);
	UWTainlordCreationPresetCard* AddSkinToneCard(FName PresetId, const FText& DisplayName, const FLinearColor& Color);
	void SetupScrollablePresetGrid();

	/** Enforce correct slot layout at runtime (since Blueprint property changes may not stick). */
	void SetupLayout();

	UPROPERTY(Transient)
	TObjectPtr<UPanelWidget> CardInsertionTarget;

	// --- State ---

	UPROPERTY(Transient)
	FTainlordProfileData WorkingProfile;

	UPROPERTY(Transient)
	EPresetCategory ActiveCategory = EPresetCategory::Head;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWTainlordCreationPresetCard>> PresetCards;
};
