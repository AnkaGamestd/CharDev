// Copyright Kingdawn. All Rights Reserved.

#include "Customization/WTainlordRaceSelectScreen.h"
#include "Customization/TainlordCharacterCreationLibrary.h"
#include "Customization/TainlordCharacterSpawnLibrary.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

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
			for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
			{
				if (UTextBlock* ChildText = FindTextBlockInWidget(Panel->GetChildAt(i)))
				{
					return ChildText;
				}
			}
		}

		return nullptr;
	}

	void SetButtonText(UButton* Button, const FString& Text)
	{
		if (!Button)
		{
			return;
		}

		if (UTextBlock* TextBlock = FindTextBlockInWidget(Button))
		{
			TextBlock->SetText(FText::FromString(Text));
		}
	}
}

UWTainlordRaceSelectScreen::UWTainlordRaceSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWTainlordRaceSelectScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Flexible widget finding (supports _0, _1 suffixes from BP)
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
	if (!ConfirmButton)
	{
		ConfirmButton = FindNamedWidgetFlexible<UButton>(this, TEXT("ConfirmButton"));
	}
	if (!BackButton)
	{
		BackButton = FindNamedWidgetFlexible<UButton>(this, TEXT("BackButton"));
	}
	if (!TitleLabel)
	{
		TitleLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("TitleLabel"));
	}
	if (!SelectedRaceLabel)
	{
		SelectedRaceLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("SelectedRaceLabel"));
	}

	// Bind race buttons
	if (RaceHumanButton)
	{
		RaceHumanButton->OnClicked.AddDynamic(this, &UWTainlordRaceSelectScreen::OnRaceHumanClicked);
	}
	if (RaceElfButton)
	{
		RaceElfButton->OnClicked.AddDynamic(this, &UWTainlordRaceSelectScreen::OnRaceElfClicked);
	}
	if (RaceDwarfButton)
	{
		RaceDwarfButton->OnClicked.AddDynamic(this, &UWTainlordRaceSelectScreen::OnRaceDwarfClicked);
	}
	if (RaceOrcButton)
	{
		RaceOrcButton->OnClicked.AddDynamic(this, &UWTainlordRaceSelectScreen::OnRaceOrcClicked);
	}

	// Bind action buttons
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UWTainlordRaceSelectScreen::OnConfirmClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UWTainlordRaceSelectScreen::OnBackClicked);
	}
}

void UWTainlordRaceSelectScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// Set button texts
	SetButtonText(RaceHumanButton, TEXT("Human"));
	SetButtonText(RaceElfButton, TEXT("Elf"));
	SetButtonText(RaceDwarfButton, TEXT("Dwarf"));
	SetButtonText(RaceOrcButton, TEXT("Orc"));
	SetButtonText(ConfirmButton, TEXT("Select"));
	SetButtonText(BackButton, TEXT("Back"));

	if (TitleLabel)
	{
		TitleLabel->SetText(FText::FromString(TEXT("SELECT YOUR RACE")));
	}

	// --- Visual polish (Fix 2: uniform sizes, Fix 3: font sizes) ---
	ApplyUniformButtonSizes();
	ApplyFontSizes();

	// Fix 1: highlight default selection
	UpdateButtonHighlights();
}

void UWTainlordRaceSelectScreen::InitRaceSelect()
{
	SelectedRace = ECharacterRace::Human;
	RefreshPreview();
	BP_OnRaceChanged(SelectedRace);
	UpdateButtonHighlights();

	if (SelectedRaceLabel)
	{
		SelectedRaceLabel->SetText(FText::FromString(TEXT("Human")));
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordRaceSelectScreen: Initialized (default race=Human)"));
}

void UWTainlordRaceSelectScreen::SelectRace(ECharacterRace Race)
{
	if (SelectedRace == Race)
	{
		return;
	}

	SelectedRace = Race;
	BP_OnRaceChanged(Race);
	UpdateButtonHighlights();  // Fix 1: update visual highlights
	RefreshPreview();

	if (SelectedRaceLabel)
	{
		SelectedRaceLabel->SetText(FText::FromString(
			StaticEnum<ECharacterRace>()->GetDisplayValueAsText(Race).ToString()));
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordRaceSelectScreen: Race changed to %d"), static_cast<int32>(Race));
}

void UWTainlordRaceSelectScreen::RefreshPreview()
{
	if (!PreviewCharacter)
	{
		return;
	}

	// Apply a minimal profile with just the race to show the default body
	FTainlordProfileData PreviewProfile;
	PreviewProfile.Gender = ECharacterGender::Male;
	PreviewProfile.Race = SelectedRace;

	UTainlordCharacterCreationLibrary::ApplyPreview(PreviewCharacter, PreviewProfile);
}

void UWTainlordRaceSelectScreen::OnRaceHumanClicked() { SelectRace(ECharacterRace::Human); }
void UWTainlordRaceSelectScreen::OnRaceElfClicked() { SelectRace(ECharacterRace::Elf); }
void UWTainlordRaceSelectScreen::OnRaceDwarfClicked() { SelectRace(ECharacterRace::Dwarf); }
void UWTainlordRaceSelectScreen::OnRaceOrcClicked() { SelectRace(ECharacterRace::Orc); }

void UWTainlordRaceSelectScreen::OnConfirmClicked()
{
	UE_LOG(LogTemp, Log, TEXT("WTainlordRaceSelectScreen: Race confirmed (%d)"), static_cast<int32>(SelectedRace));
	OnRaceConfirmed.Broadcast();
}

void UWTainlordRaceSelectScreen::OnBackClicked()
{
	UE_LOG(LogTemp, Log, TEXT("WTainlordRaceSelectScreen: Cancelled"));
	OnRaceSelectCancelled.Broadcast();
}

void UWTainlordRaceSelectScreen::SetButtonText(UButton* Button, const FString& Text)
{
	::SetButtonText(Button, Text);
}

// ---------------------------------------------------------------------------
// Visual polish helpers
// ---------------------------------------------------------------------------

void UWTainlordRaceSelectScreen::UpdateButtonHighlights()
{
	ApplyButtonHighlight(RaceHumanButton,  SelectedRace == ECharacterRace::Human);
	ApplyButtonHighlight(RaceElfButton,    SelectedRace == ECharacterRace::Elf);
	ApplyButtonHighlight(RaceDwarfButton,  SelectedRace == ECharacterRace::Dwarf);
	ApplyButtonHighlight(RaceOrcButton,    SelectedRace == ECharacterRace::Orc);
}

void UWTainlordRaceSelectScreen::ApplyButtonHighlight(UButton* Button, bool bSelected)
{
	if (!Button)
	{
		return;
	}

	// Selected: gold tint. Neutral: dark translucent.
	Button->SetBackgroundColor(bSelected
		? FLinearColor(0.85f, 0.70f, 0.25f, 1.0f)   // Gold
		: FLinearColor(0.12f, 0.12f, 0.12f, 0.80f)); // Dark gray
}

void UWTainlordRaceSelectScreen::ApplyUniformButtonSizes()
{
	// Fix 2: Force uniform sizing on race buttons.
	// UButton inherits from UWidget which has SetRenderTransformPivot but not SetMinDesiredWidth.
	// Instead, we apply uniform padding on the VerticalBoxSlot (most common layout for this screen)
	// and set the button's minimum width/height through its underlying Slate widget via WidthOverride
	// on the SButton style.

	auto ApplySize = [](UButton* Button)
	{
		if (!Button)
		{
			return;
		}

		// Apply uniform padding if inside a VerticalBox
		if (UVerticalBoxSlot* VBoxSlot = Cast<UVerticalBoxSlot>(Button->Slot))
		{
			VBoxSlot->SetPadding(FMargin(4.0f, 3.0f));
			VBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		}

		// Override the button style to enforce consistent look
		FButtonStyle Style = Button->GetStyle();
		Style.NormalPadding = FMargin(16.0f, 10.0f);
		Style.PressedPadding = FMargin(16.0f, 11.0f, 16.0f, 9.0f);
		Button->SetStyle(Style);
	};

	ApplySize(RaceHumanButton);
	ApplySize(RaceElfButton);
	ApplySize(RaceDwarfButton);
	ApplySize(RaceOrcButton);
}

void UWTainlordRaceSelectScreen::ApplyFontSizes()
{
	// Fix 3: reduce font sizes across the board

	// Title: 20
	SetTextBlockFontSize(TitleLabel, 20);

	// Race buttons: 18
	SetButtonFontSize(RaceHumanButton, 18);
	SetButtonFontSize(RaceElfButton, 18);
	SetButtonFontSize(RaceDwarfButton, 18);
	SetButtonFontSize(RaceOrcButton, 18);

	// Action buttons: 18
	SetButtonFontSize(ConfirmButton, 18);
	SetButtonFontSize(BackButton, 18);

	// Selected race label: 16
	SetTextBlockFontSize(SelectedRaceLabel, 16);
}

void UWTainlordRaceSelectScreen::SetTextBlockFontSize(UTextBlock* TextBlock, int32 Size)
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = Size;
	TextBlock->SetFont(Font);
}

void UWTainlordRaceSelectScreen::SetButtonFontSize(UButton* Button, int32 Size)
{
	if (!Button)
	{
		return;
	}

	if (UTextBlock* Text = FindTextBlockInWidget(Button))
	{
		SetTextBlockFontSize(Text, Size);
	}
}
