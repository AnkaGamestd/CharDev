// Copyright Kingdawn. All Rights Reserved.

#include "Customization/WTainlordCreationScreen.h"
#include "Customization/TainlordCharacterCreationLibrary.h"
#include "Customization/TainlordCharacterProfileSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ContentWidget.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Components/Spacer.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

namespace
{
	template <typename T>
	T* FindNamedWidgetFlexible(const UUserWidget* Owner, const TCHAR* BaseName)
	{
		if (!Owner || !Owner->WidgetTree)
		{
			return nullptr;
		}

		if (UWidget* Exact = Owner->WidgetTree->FindWidget(FName(BaseName)))
		{
			return Cast<T>(Exact);
		}

		for (int32 Index = 0; Index < 20; ++Index)
		{
			const FString Candidate = FString::Printf(TEXT("%s_%d"), BaseName, Index);
			if (UWidget* Suffix = Owner->WidgetTree->FindWidget(FName(*Candidate)))
			{
				return Cast<T>(Suffix);
			}
		}

		return nullptr;
	}

	UTextBlock* FindTextBlockInWidget(UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		if (UTextBlock* Text = Cast<UTextBlock>(Widget))
		{
			return Text;
		}

		if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
			{
				if (UTextBlock* ChildText = FindTextBlockInWidget(Panel->GetChildAt(Index)))
				{
					return ChildText;
				}
			}
		}
		else if (UContentWidget* Content = Cast<UContentWidget>(Widget))
		{
			if (UTextBlock* ChildText = FindTextBlockInWidget(Content->GetContent()))
			{
				return ChildText;
			}
		}

		return nullptr;
	}

	void SetButtonText(UButton* Button, const TCHAR* TextValue)
	{
		if (!Button)
		{
			return;
		}

		if (UTextBlock* TextBlock = FindTextBlockInWidget(Button))
		{
			TextBlock->SetText(FText::FromString(TextValue));
		}
	}
}

UWTainlordCreationScreen::UWTainlordCreationScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActiveCategory = EPresetCategory::Head;
}

void UWTainlordCreationScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!CharacterNameInput)
	{
		CharacterNameInput = FindNamedWidgetFlexible<UEditableTextBox>(this, TEXT("CharacterNameInput"));
	}
	if (!GenderMaleButton)
	{
		GenderMaleButton = FindNamedWidgetFlexible<UButton>(this, TEXT("GenderMaleButton"));
	}
	if (!GenderFemaleButton)
	{
		GenderFemaleButton = FindNamedWidgetFlexible<UButton>(this, TEXT("GenderFemaleButton"));
	}
	if (!RaceHumanButton)
	{
		RaceHumanButton = FindNamedWidgetFlexible<UButton>(this, TEXT("RaceHumanButton"));
	}
	if (!RaceElfButton)
	{
		RaceElfButton = FindNamedWidgetFlexible<UButton>(this, TEXT("RaceElfButton"));
	}
	if (!RaceDwarfButton)
	{
		RaceDwarfButton = FindNamedWidgetFlexible<UButton>(this, TEXT("RaceDwarfButton"));
	}
	if (!RaceOrcButton)
	{
		RaceOrcButton = FindNamedWidgetFlexible<UButton>(this, TEXT("RaceOrcButton"));
	}
	if (!CategoryHeadButton)
	{
		CategoryHeadButton = FindNamedWidgetFlexible<UButton>(this, TEXT("CategoryHeadButton"));
	}
	if (!CategoryHairButton)
	{
		CategoryHairButton = FindNamedWidgetFlexible<UButton>(this, TEXT("CategoryHairButton"));
	}
	if (!CategoryBeardButton)
	{
		CategoryBeardButton = FindNamedWidgetFlexible<UButton>(this, TEXT("CategoryBeardButton"));
	}
	if (!CategorySkinToneButton)
	{
		CategorySkinToneButton = FindNamedWidgetFlexible<UButton>(this, TEXT("CategorySkinToneButton"));
	}
	if (!PresetGridContainer)
	{
		PresetGridContainer = FindNamedWidgetFlexible<UPanelWidget>(this, TEXT("PresetGridContainer"));
	}
	if (!ConfirmButton)
	{
		ConfirmButton = FindNamedWidgetFlexible<UButton>(this, TEXT("ConfirmButton"));
	}
	if (!BackButton)
	{
		BackButton = FindNamedWidgetFlexible<UButton>(this, TEXT("BackButton"));
	}
	if (!ValidationMessage)
	{
		ValidationMessage = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("ValidationMessage"));
	}
	if (!GenderLabel)
	{
		GenderLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("GenderLabel"));
	}
	if (!RaceLabel)
	{
		RaceLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("RaceLabel"));
	}
	if (!ActiveCategoryLabel)
	{
		ActiveCategoryLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("ActiveCategoryLabel"));
	}

	if (GenderMaleButton)
	{
		GenderMaleButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnGenderMaleClicked);
	}
	if (GenderFemaleButton)
	{
		GenderFemaleButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnGenderFemaleClicked);
	}
	if (RaceHumanButton)
	{
		RaceHumanButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnRaceHumanClicked);
	}
	if (RaceElfButton)
	{
		RaceElfButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnRaceElfClicked);
	}
	if (RaceDwarfButton)
	{
		RaceDwarfButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnRaceDwarfClicked);
	}
	if (RaceOrcButton)
	{
		RaceOrcButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnRaceOrcClicked);
	}
	if (CategoryHeadButton)
	{
		CategoryHeadButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnCategoryHeadClicked);
	}
	if (CategoryHairButton)
	{
		CategoryHairButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnCategoryHairClicked);
	}
	if (CategoryBeardButton)
	{
		CategoryBeardButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnCategoryBeardClicked);
	}
	if (CategorySkinToneButton)
	{
		CategorySkinToneButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnCategorySkinToneClicked);
	}
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnConfirmClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UWTainlordCreationScreen::OnBackClicked);
	}
	if (CharacterNameInput)
	{
		CharacterNameInput->OnTextChanged.AddDynamic(this, &UWTainlordCreationScreen::OnNameChanged);
		CharacterNameInput->OnTextCommitted.AddDynamic(this, &UWTainlordCreationScreen::OnNameCommitted);
	}
}

void UWTainlordCreationScreen::NativeConstruct()
{
	Super::NativeConstruct();

	auto CollapseWidget = [](UWidget* Widget)
	{
		if (Widget)
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	};
	CollapseWidget(RaceHumanButton);
	CollapseWidget(RaceElfButton);
	CollapseWidget(RaceDwarfButton);
	CollapseWidget(RaceOrcButton);
	CollapseWidget(RaceLabel);
	CollapseWidget(ActiveCategoryLabel);

	SetButtonText(GenderMaleButton, TEXT("Male"));
	SetButtonText(GenderFemaleButton, TEXT("Female"));
	SetButtonText(CategoryHeadButton, TEXT("Head"));
	SetButtonText(CategoryHairButton, TEXT("Hair"));
	SetButtonText(CategoryBeardButton, TEXT("Beard"));
	SetButtonText(CategorySkinToneButton, TEXT("Skin Tone"));
	SetButtonText(BackButton, TEXT("Back"));
	SetButtonText(ConfirmButton, TEXT("Create Character"));

	if (GenderLabel)
	{
		GenderLabel->SetText(FText::FromString(TEXT("GENDER")));
	}
	if (CharacterNameInput)
	{
		CharacterNameInput->SetHintText(FText::FromString(TEXT("Character Name")));
	}

	SetupLayout();
	SetupScrollablePresetGrid();
	SetValidationMessage(TEXT(""), false);
}

// ---------------------------------------------------------------------------
// Layout setup
// ---------------------------------------------------------------------------

void UWTainlordCreationScreen::SetupLayout()
{
	UHorizontalBox* MainHBox = FindNamedWidgetFlexible<UHorizontalBox>(this, TEXT("MainHBox"));
	if (!MainHBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("WTainlordCreationScreen: MainHBox not found"));
		return;
	}

	// --- LeftColumn: compact left side (auto size) ---
	UVerticalBox* LeftColumn = FindNamedWidgetFlexible<UVerticalBox>(this, TEXT("LeftColumn"));
	if (LeftColumn)
	{
		if (UHorizontalBoxSlot* LeftSlot = Cast<UHorizontalBoxSlot>(LeftColumn->Slot))
		{
			LeftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			LeftSlot->SetPadding(FMargin(10.0f, 10.0f, 10.0f, 10.0f));
			LeftSlot->SetVerticalAlignment(VAlign_Top);
		}
	}

	// --- LeftPresetPanel: auto size for preset grid ---
	UBorder* LeftPresetPanel = FindNamedWidgetFlexible<UBorder>(this, TEXT("LeftPresetPanel"));
	if (LeftPresetPanel)
	{
		if (UHorizontalBoxSlot* PresetSlot = Cast<UHorizontalBoxSlot>(LeftPresetPanel->Slot))
		{
			PresetSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			PresetSlot->SetPadding(FMargin(5.0f, 10.0f, 5.0f, 10.0f));
			PresetSlot->SetVerticalAlignment(VAlign_Top);
		}
	}

	// --- Insert a spacer to push RightCustomizationPanel to the right edge ---
	bool bHasSpacer = false;
	for (int32 i = 0; i < MainHBox->GetChildrenCount(); ++i)
	{
		if (Cast<USpacer>(MainHBox->GetChildAt(i)))
		{
			bHasSpacer = true;
			break;
		}
	}

	UVerticalBox* RightPanel = FindNamedWidgetFlexible<UVerticalBox>(this, TEXT("RightCustomizationPanel"));

	if (!bHasSpacer && RightPanel)
	{
		MainHBox->RemoveChild(RightPanel);

		USpacer* CenterSpacer = NewObject<USpacer>(this);
		UHorizontalBoxSlot* SpacerSlot = Cast<UHorizontalBoxSlot>(MainHBox->AddChild(CenterSpacer));
		if (SpacerSlot)
		{
			SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		MainHBox->AddChild(RightPanel);
	}

	// --- RightCustomizationPanel: auto size, pushed right via spacer ---
	if (RightPanel)
	{
		if (UHorizontalBoxSlot* RightSlot = Cast<UHorizontalBoxSlot>(RightPanel->Slot))
		{
			RightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			RightSlot->SetPadding(FMargin(10.0f, 20.0f, 20.0f, 10.0f));
			RightSlot->SetVerticalAlignment(VAlign_Top);
		}

		UTextBlock* CustLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("CustomizationLabel"));
		if (CustLabel)
		{
			CustLabel->SetText(FText::FromString(TEXT("Appearance")));
		}
	}

	// --- Collapse old BottomBar in LeftColumn ---
	UHorizontalBox* OldBottomBar = FindNamedWidgetFlexible<UHorizontalBox>(this, TEXT("BottomBar"));
	if (OldBottomBar)
	{
		OldBottomBar->SetVisibility(ESlateVisibility::Collapsed);
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: Layout setup complete"));
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void UWTainlordCreationScreen::InitCreationScreen()
{
	WorkingProfile = FTainlordProfileData();
	WorkingProfile.Gender = ECharacterGender::Male;
	WorkingProfile.Race = ECharacterRace::Human;

	if (CharacterNameInput)
	{
		CharacterNameInput->SetText(FText::GetEmpty());
	}

	ActiveCategory = EPresetCategory::Head;
	RefreshPresetGrid();
	RefreshPreview();

	BP_OnGenderChanged(WorkingProfile.Gender);
	BP_OnRaceChanged(WorkingProfile.Race);
	BP_OnCategorySwitched(ActiveCategory);

	UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: Initialized (Gender=Male, Race=Human, Category=Head)"));
}

void UWTainlordCreationScreen::SetCharacterName(const FString& Name)
{
	WorkingProfile.CharacterName = Name;
}

void UWTainlordCreationScreen::SetGender(ECharacterGender Gender)
{
	if (WorkingProfile.Gender == Gender) return;
	WorkingProfile.Gender = Gender;
	BP_OnGenderChanged(Gender);
	HandleContextChange();
}

void UWTainlordCreationScreen::SetRace(ECharacterRace Race)
{
	if (WorkingProfile.Race == Race) return;
	WorkingProfile.Race = Race;
	BP_OnRaceChanged(Race);
	HandleContextChange();
}

void UWTainlordCreationScreen::SwitchCategory(EPresetCategory Category)
{
	UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: SwitchCategory - Category=%d (Head=0, Hair=1, Beard=2, SkinTone=3)"),
		static_cast<int32>(Category));
	
	if (ActiveCategory == Category)
	{
		UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: SwitchCategory - Already on this category, skipping"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: SwitchCategory - Changing from %d to %d"),
		static_cast<int32>(ActiveCategory), static_cast<int32>(Category));
	
	ActiveCategory = Category;
	RefreshPresetGrid();
	BP_OnCategorySwitched(Category);

	if (ActiveCategoryLabel)
	{
		ActiveCategoryLabel->SetText(FText::FromString(StaticEnum<EPresetCategory>()->GetDisplayValueAsText(Category).ToString()));
	}
}

void UWTainlordCreationScreen::OnConfirmClicked()
{
	FString Reason;
	if (!UTainlordCharacterCreationLibrary::ConfirmCreation(this, WorkingProfile, Reason))
	{
		SetValidationMessage(Reason, true);
		BP_OnValidationResult(false, Reason);
		return;
	}
	SetValidationMessage(TEXT(""), false);
	BP_OnValidationResult(true, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: Creation confirmed for '%s'"), *WorkingProfile.CharacterName);
	OnCreationConfirmed.Broadcast();
}

void UWTainlordCreationScreen::OnBackClicked()
{
	UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: Creation cancelled"));
	OnCreationCancelled.Broadcast();
}

void UWTainlordCreationScreen::RandomizeAppearance()
{
	TArray<FTainlordHeadEntry> Heads = UTainlordCharacterCreationLibrary::GetAvailableHeads(this, WorkingProfile.Gender, WorkingProfile.Race);
	TArray<FTainlordHairEntry> HairOptions = UTainlordCharacterCreationLibrary::GetAvailableHair(this, WorkingProfile.Gender, WorkingProfile.Race);
	TArray<FTainlordBeardEntry> Beards = UTainlordCharacterCreationLibrary::GetAvailableBeards(this, WorkingProfile.Gender, WorkingProfile.Race);
	TArray<FTainlordSkinToneEntry> SkinTones = UTainlordCharacterCreationLibrary::GetAvailableSkinTones(this);

	if (Heads.Num() > 0) { int32 Idx = FMath::RandRange(-1, Heads.Num() - 1); WorkingProfile.AppearanceData.HeadId = (Idx >= 0) ? Heads[Idx].Id : NAME_None; } else { WorkingProfile.AppearanceData.HeadId = NAME_None; }
	if (HairOptions.Num() > 0) { int32 Idx = FMath::RandRange(-1, HairOptions.Num() - 1); WorkingProfile.AppearanceData.HairId = (Idx >= 0) ? HairOptions[Idx].Id : NAME_None; } else { WorkingProfile.AppearanceData.HairId = NAME_None; }
	if (Beards.Num() > 0) { int32 Idx = FMath::RandRange(-1, Beards.Num() - 1); WorkingProfile.AppearanceData.BeardId = (Idx >= 0) ? Beards[Idx].Id : NAME_None; } else { WorkingProfile.AppearanceData.BeardId = NAME_None; }
	if (SkinTones.Num() > 0) { int32 Idx = FMath::RandRange(0, SkinTones.Num() - 1); WorkingProfile.AppearanceData.SkinToneId = SkinTones[Idx].Id; }

	RefreshPresetGrid();
	RefreshPreview();
}

// ---------------------------------------------------------------------------
// Button handlers
// ---------------------------------------------------------------------------

void UWTainlordCreationScreen::OnGenderMaleClicked() { SetGender(ECharacterGender::Male); }
void UWTainlordCreationScreen::OnGenderFemaleClicked() { SetGender(ECharacterGender::Female); }
void UWTainlordCreationScreen::OnRaceHumanClicked() { SetRace(ECharacterRace::Human); }
void UWTainlordCreationScreen::OnRaceElfClicked() { SetRace(ECharacterRace::Elf); }
void UWTainlordCreationScreen::OnRaceDwarfClicked() { SetRace(ECharacterRace::Dwarf); }
void UWTainlordCreationScreen::OnRaceOrcClicked() { SetRace(ECharacterRace::Orc); }
void UWTainlordCreationScreen::OnCategoryHeadClicked() { SwitchCategory(EPresetCategory::Head); }
void UWTainlordCreationScreen::OnCategoryHairClicked() { SwitchCategory(EPresetCategory::Hair); }
void UWTainlordCreationScreen::OnCategoryBeardClicked() { SwitchCategory(EPresetCategory::Beard); }
void UWTainlordCreationScreen::OnCategorySkinToneClicked() { SwitchCategory(EPresetCategory::SkinTone); }

void UWTainlordCreationScreen::OnNameChanged(const FText& NewText) { WorkingProfile.CharacterName = NewText.ToString(); }
void UWTainlordCreationScreen::OnNameCommitted(const FText& NewText, ETextCommit::Type CommitMethod) { WorkingProfile.CharacterName = NewText.ToString(); }

// ---------------------------------------------------------------------------
// Preset grid selection handler
// ---------------------------------------------------------------------------

void UWTainlordCreationScreen::OnPresetCardSelected(UWTainlordCreationPresetCard* Card)
{
	if (!Card) return;

	for (UWTainlordCreationPresetCard* Existing : PresetCards)
	{
		if (Existing) Existing->SetSelected(Existing == Card);
	}

	const FName PresetId = Card->GetPresetId();
	const EPresetCategory Category = Card->GetCategory();

	switch (Category)
	{
	case EPresetCategory::Head:     WorkingProfile.AppearanceData.HeadId = PresetId; break;
	case EPresetCategory::Hair:     WorkingProfile.AppearanceData.HairId = PresetId; break;
	case EPresetCategory::Beard:    WorkingProfile.AppearanceData.BeardId = PresetId; break;
	case EPresetCategory::SkinTone: WorkingProfile.AppearanceData.SkinToneId = PresetId; break;
	}

	RefreshPreviewSlot(Category, PresetId);
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void UWTainlordCreationScreen::RefreshPresetGrid()
{
	if (!PresetGridContainer) { UE_LOG(LogTemp, Warning, TEXT("WTainlordCreationScreen: PresetGridContainer is null")); return; }

	ClearPresetGrid();

	switch (ActiveCategory)
	{
	case EPresetCategory::Head:
		AddPresetCard(NAME_None, EPresetCategory::Head, FText::FromString(TEXT("None")));
		for (const FTainlordHeadEntry& Entry : UTainlordCharacterCreationLibrary::GetAvailableHeads(this, WorkingProfile.Gender, WorkingProfile.Race))
			AddPresetCard(Entry.Id, EPresetCategory::Head, FText::FromName(Entry.Id));
		break;
	case EPresetCategory::Hair:
		UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: RefreshPresetGrid - HAIR category selected, Gender=%d, Race=%d"),
			static_cast<int32>(WorkingProfile.Gender), static_cast<int32>(WorkingProfile.Race));
		AddPresetCard(NAME_None, EPresetCategory::Hair, FText::FromString(TEXT("None")));
		for (const FTainlordHairEntry& Entry : UTainlordCharacterCreationLibrary::GetAvailableHair(this, WorkingProfile.Gender, WorkingProfile.Race))
		{
			UE_LOG(LogTemp, Log, TEXT("  - Adding hair card: Id=%s"), *Entry.Id.ToString());
			AddPresetCard(Entry.Id, EPresetCategory::Hair, FText::FromName(Entry.Id));
		}
		break;
	case EPresetCategory::Beard:
		AddPresetCard(NAME_None, EPresetCategory::Beard, FText::FromString(TEXT("None")));
		for (const FTainlordBeardEntry& Entry : UTainlordCharacterCreationLibrary::GetAvailableBeards(this, WorkingProfile.Gender, WorkingProfile.Race))
			AddPresetCard(Entry.Id, EPresetCategory::Beard, FText::FromName(Entry.Id));
		break;
	case EPresetCategory::SkinTone:
		for (const FTainlordSkinToneEntry& Entry : UTainlordCharacterCreationLibrary::GetAvailableSkinTones(this))
			AddSkinToneCard(Entry.Id, FText::FromName(Entry.Id), Entry.Color);
		break;
	}

	const FName SelectedId =
		(ActiveCategory == EPresetCategory::Head) ? WorkingProfile.AppearanceData.HeadId :
		(ActiveCategory == EPresetCategory::Hair) ? WorkingProfile.AppearanceData.HairId :
		(ActiveCategory == EPresetCategory::Beard) ? WorkingProfile.AppearanceData.BeardId :
		WorkingProfile.AppearanceData.SkinToneId;

	for (UWTainlordCreationPresetCard* Card : PresetCards)
	{
		if (Card) Card->SetSelected(Card->GetPresetId() == SelectedId);
	}
}

void UWTainlordCreationScreen::RefreshPreview()
{
	if (!PreviewCharacter) { UE_LOG(LogTemp, Warning, TEXT("WTainlordCreationScreen: PreviewCharacter is null")); return; }
	UTainlordCharacterCreationLibrary::ApplyPreview(PreviewCharacter, WorkingProfile);
}

void UWTainlordCreationScreen::RefreshPreviewSlot(EPresetCategory Category, FName PresetId)
{
	if (!PreviewCharacter) return;
	UTainlordCharacterCreationLibrary::ApplyPreviewSlot(PreviewCharacter, WorkingProfile, CategoryToSlotName(Category), PresetId);
}

void UWTainlordCreationScreen::HandleContextChange()
{
	UTainlordCharacterCreationLibrary::SanitizeAppearanceForContext(this, WorkingProfile);
	RefreshPresetGrid();
	RefreshPreview();
}

void UWTainlordCreationScreen::SetValidationMessage(const FString& Message, bool bIsError)
{
	if (!ValidationMessage) return;
	if (Message.IsEmpty()) { ValidationMessage->SetVisibility(ESlateVisibility::Collapsed); }
	else { ValidationMessage->SetText(FText::FromString(Message)); ValidationMessage->SetVisibility(ESlateVisibility::Visible); }
}

FName UWTainlordCreationScreen::CategoryToSlotName(EPresetCategory Category)
{
	switch (Category)
	{
	case EPresetCategory::Head:        return FName(TEXT("Head"));
	case EPresetCategory::Hair:        return FName(TEXT("Hair"));
	case EPresetCategory::Beard:       return FName(TEXT("Beard"));
	case EPresetCategory::SkinTone:    return FName(TEXT("SkinTone"));
	case EPresetCategory::Shoulders:   return FName(TEXT("Shoulders"));
	case EPresetCategory::LeftBracer:  return FName(TEXT("LeftBracer"));
	case EPresetCategory::RightBracer: return FName(TEXT("RightBracer"));
	default:                           return NAME_None;
	}
}

void UWTainlordCreationScreen::ClearPresetGrid()
{
	UPanelWidget* Target = CardInsertionTarget ? CardInsertionTarget : PresetGridContainer;
	if (Target) Target->ClearChildren();
	PresetCards.Reset();
}

UWTainlordCreationPresetCard* UWTainlordCreationScreen::AddPresetCard(FName PresetId, EPresetCategory Category, const FText& DisplayName)
{
	UPanelWidget* Target = CardInsertionTarget ? CardInsertionTarget : PresetGridContainer;
	if (!Target || !CardWidgetClass) return nullptr;

	UWTainlordCreationPresetCard* Card = CreateWidget<UWTainlordCreationPresetCard>(this, CardWidgetClass);
	if (!Card) return nullptr;

	Card->SetupCard(PresetId, Category, DisplayName);
	Card->OnPresetCardSelected.AddDynamic(this, &UWTainlordCreationScreen::OnPresetCardSelected);
	Target->AddChild(Card);
	PresetCards.Add(Card);
	return Card;
}

UWTainlordCreationPresetCard* UWTainlordCreationScreen::AddSkinToneCard(FName PresetId, const FText& DisplayName, const FLinearColor& Color)
{
	UPanelWidget* Target = CardInsertionTarget ? CardInsertionTarget : PresetGridContainer;
	if (!Target || !CardWidgetClass) return nullptr;

	UWTainlordCreationPresetCard* Card = CreateWidget<UWTainlordCreationPresetCard>(this, CardWidgetClass);
	if (!Card) return nullptr;

	Card->SetupSkinToneCard(PresetId, DisplayName, Color);
	Card->OnPresetCardSelected.AddDynamic(this, &UWTainlordCreationScreen::OnPresetCardSelected);
	Target->AddChild(Card);
	PresetCards.Add(Card);
	return Card;
}

void UWTainlordCreationScreen::SetupScrollablePresetGrid()
{
	if (!PresetGridContainer) { UE_LOG(LogTemp, Warning, TEXT("WTainlordCreationScreen: PresetGridContainer is null")); return; }

	// Move ActiveCategoryLabel into the preset panel area
	UWidget* PresetParentWidget = PresetGridContainer->GetParent();
	UBorder* PresetBorder = Cast<UBorder>(PresetParentWidget);

	if (PresetBorder)
	{
		PresetBorder->SetContent(nullptr);

		UVerticalBox* LayoutBox = NewObject<UVerticalBox>(this);

		ActiveCategoryLabel = NewObject<UTextBlock>(this);
		ActiveCategoryLabel->SetText(FText::FromString(TEXT("HEAD")));

		UVerticalBoxSlot* LabelSlot = Cast<UVerticalBoxSlot>(LayoutBox->AddChild(ActiveCategoryLabel));
		if (LabelSlot)
		{
			LabelSlot->SetPadding(FMargin(10.0f, 10.0f, 10.0f, 5.0f));
			LabelSlot->SetHorizontalAlignment(HAlign_Left);
		}

		UVerticalBoxSlot* GridSlot = Cast<UVerticalBoxSlot>(LayoutBox->AddChild(PresetGridContainer));
		if (GridSlot)
		{
			GridSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		PresetBorder->SetContent(LayoutBox);
		UE_LOG(LogTemp, Log, TEXT("WTainlordCreationScreen: Moved ActiveCategoryLabel into preset panel"));
	}

	if (UWrapBox* ExistingWrap = Cast<UWrapBox>(PresetGridContainer))
	{
		ExistingWrap->SetInnerSlotPadding(FVector2D(8.0f, 8.0f));
		ExistingWrap->SetWrapSize(340.0f);
		CardInsertionTarget = PresetGridContainer;
		return;
	}

	if (UScrollBox* ExistingScroll = Cast<UScrollBox>(PresetGridContainer))
	{
		UWrapBox* WrapBoxWidget = NewObject<UWrapBox>(this);
		WrapBoxWidget->SetInnerSlotPadding(FVector2D(8.0f, 8.0f));
		WrapBoxWidget->SetWrapSize(340.0f);
		ExistingScroll->AddChild(WrapBoxWidget);
		CardInsertionTarget = WrapBoxWidget;
		return;
	}

	PresetGridContainer->ClearChildren();
	UScrollBox* ScrollBoxWidget = NewObject<UScrollBox>(this);
	ScrollBoxWidget->SetScrollBarVisibility(ESlateVisibility::Collapsed);
	ScrollBoxWidget->SetConsumeMouseWheel(EConsumeMouseWheel::Always);
	UWrapBox* WrapBoxWidget = NewObject<UWrapBox>(this);
	WrapBoxWidget->SetInnerSlotPadding(FVector2D(8.0f, 8.0f));
	WrapBoxWidget->SetWrapSize(340.0f);
	ScrollBoxWidget->AddChild(WrapBoxWidget);
	PresetGridContainer->AddChild(ScrollBoxWidget);
	CardInsertionTarget = WrapBoxWidget;
}
