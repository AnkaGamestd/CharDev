// Copyright Kingdawn. All Rights Reserved.

#include "Customization/WTainlordCitySelectScreen.h"
#include "Domain/TainlordDemoCatalogLibrary.h"
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

UWTainlordCitySelectScreen::UWTainlordCitySelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWTainlordCitySelectScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Flexible widget finding (supports _0, _1 suffixes from BP)
	if (!CityButton0)
	{
		CityButton0 = FindNamedWidgetFlexible<UButton>(this, TEXT("CityButton0"));
	}
	if (!CityButton1)
	{
		CityButton1 = FindNamedWidgetFlexible<UButton>(this, TEXT("CityButton1"));
	}
	if (!CityButton2)
	{
		CityButton2 = FindNamedWidgetFlexible<UButton>(this, TEXT("CityButton2"));
	}
	if (!CityButton3)
	{
		CityButton3 = FindNamedWidgetFlexible<UButton>(this, TEXT("CityButton3"));
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
	if (!CityNameLabel)
	{
		CityNameLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("CityNameLabel"));
	}
	if (!CityDescriptionLabel)
	{
		CityDescriptionLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("CityDescriptionLabel"));
	}
	if (!AllegianceLabel)
	{
		AllegianceLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("AllegianceLabel"));
	}
	if (!ReputationLabel)
	{
		ReputationLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("ReputationLabel"));
	}

	// Bind city buttons
	if (CityButton0)
	{
		CityButton0->OnClicked.AddDynamic(this, &UWTainlordCitySelectScreen::OnCityButton0Clicked);
	}
	if (CityButton1)
	{
		CityButton1->OnClicked.AddDynamic(this, &UWTainlordCitySelectScreen::OnCityButton1Clicked);
	}
	if (CityButton2)
	{
		CityButton2->OnClicked.AddDynamic(this, &UWTainlordCitySelectScreen::OnCityButton2Clicked);
	}
	if (CityButton3)
	{
		CityButton3->OnClicked.AddDynamic(this, &UWTainlordCitySelectScreen::OnCityButton3Clicked);
	}

	// Bind action buttons
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UWTainlordCitySelectScreen::OnConfirmClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UWTainlordCitySelectScreen::OnBackClicked);
	}
}

void UWTainlordCitySelectScreen::NativeConstruct()
{
	Super::NativeConstruct();

	SetButtonText(ConfirmButton, TEXT("Confirm"));
	SetButtonText(BackButton, TEXT("Back"));

	if (TitleLabel)
	{
		TitleLabel->SetText(FText::FromString(TEXT("SELECT YOUR STARTING CITY")));
	}

	// Visual polish
	ApplyUniformButtonSizes();
	ApplyFontSizes();
}

void UWTainlordCitySelectScreen::InitCitySelect()
{
	// Load demo cities from catalog
	CachedCities = UTainlordDemoCatalogLibrary::GetDemoCities();

	UE_LOG(LogTemp, Log, TEXT("WTainlordCitySelectScreen: Loaded %d demo cities"), CachedCities.Num());

	// Map button indices to city IDs
	ButtonIndexToCityId.SetNum(4);
	for (int32 i = 0; i < FMath::Min(4, CachedCities.Num()); ++i)
	{
		ButtonIndexToCityId[i] = CachedCities[i].CityId;

		// Set button text to city display name
		UButton* Button = nullptr;
		switch (i)
		{
		case 0: Button = CityButton0; break;
		case 1: Button = CityButton1; break;
		case 2: Button = CityButton2; break;
		case 3: Button = CityButton3; break;
		}

		if (Button)
		{
			SetButtonText(Button, CachedCities[i].DisplayName.ToString());
			Button->SetVisibility(CachedCities[i].bIncludedInFundingDemo ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
	}

	// Hide unused buttons
	for (int32 i = CachedCities.Num(); i < 4; ++i)
	{
		UButton* Button = nullptr;
		switch (i)
		{
		case 0: Button = CityButton0; break;
		case 1: Button = CityButton1; break;
		case 2: Button = CityButton2; break;
		case 3: Button = CityButton3; break;
		}

		if (Button)
		{
			Button->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Select the first recommended city by default, or the first available
	FName DefaultCityId = NAME_None;
	for (const FTainlordCityDefinition& City : CachedCities)
	{
		if (City.bRecommendedForNewPlayers)
		{
			DefaultCityId = City.CityId;
			break;
		}
	}

	if (DefaultCityId.IsNone() && CachedCities.Num() > 0)
	{
		DefaultCityId = CachedCities[0].CityId;
	}

	SelectCity(DefaultCityId);

	UE_LOG(LogTemp, Log, TEXT("WTainlordCitySelectScreen: Initialized (default city=%s)"),
		*DefaultCityId.ToString());
}

FTainlordCityDefinition UWTainlordCitySelectScreen::GetSelectedCityDefinition() const
{
	return UTainlordDemoCatalogLibrary::FindCityById(SelectedCityId);
}

void UWTainlordCitySelectScreen::SelectCity(FName CityId)
{
	if (SelectedCityId == CityId)
	{
		return;
	}

	SelectedCityId = CityId;
	UpdateInfoPanel();
	UpdateButtonHighlights();

	// Find the city definition and notify Blueprint
	const FTainlordCityDefinition* CityDef = CachedCities.FindByPredicate(
		[CityId](const FTainlordCityDefinition& Def) { return Def.CityId == CityId; });

	if (CityDef)
	{
		BP_OnCityChanged(CityId, *CityDef);
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordCitySelectScreen: City changed to %s"), *CityId.ToString());
}

void UWTainlordCitySelectScreen::UpdateInfoPanel()
{
	FTainlordCityDefinition CityDef = GetSelectedCityDefinition();

	if (CityNameLabel)
	{
		CityNameLabel->SetText(CityDef.DisplayName);
	}

	if (CityDescriptionLabel)
	{
		CityDescriptionLabel->SetText(CityDef.ShortDescription);
	}

	if (AllegianceLabel)
	{
		FString AllegianceStr = (CityDef.DerivedAllegiance == ETainlordAllegiance::City)
			? TEXT("City")
			: TEXT("Exile");
		AllegianceLabel->SetText(FText::FromString(FString::Printf(TEXT("Allegiance: %s"), *AllegianceStr)));
	}

	if (ReputationLabel)
	{
		ReputationLabel->SetText(FText::FromString(FString::Printf(
			TEXT("City Rep: %+d | Merc Rep: %+d"),
			CityDef.StartingCityReputation,
			CityDef.StartingMercenaryReputation)));
	}
}

void UWTainlordCitySelectScreen::OnCityButton0Clicked()
{
	if (ButtonIndexToCityId.IsValidIndex(0) && !ButtonIndexToCityId[0].IsNone())
	{
		SelectCity(ButtonIndexToCityId[0]);
	}
}

void UWTainlordCitySelectScreen::OnCityButton1Clicked()
{
	if (ButtonIndexToCityId.IsValidIndex(1) && !ButtonIndexToCityId[1].IsNone())
	{
		SelectCity(ButtonIndexToCityId[1]);
	}
}

void UWTainlordCitySelectScreen::OnCityButton2Clicked()
{
	if (ButtonIndexToCityId.IsValidIndex(2) && !ButtonIndexToCityId[2].IsNone())
	{
		SelectCity(ButtonIndexToCityId[2]);
	}
}

void UWTainlordCitySelectScreen::OnCityButton3Clicked()
{
	if (ButtonIndexToCityId.IsValidIndex(3) && !ButtonIndexToCityId[3].IsNone())
	{
		SelectCity(ButtonIndexToCityId[3]);
	}
}

void UWTainlordCitySelectScreen::OnConfirmClicked()
{
	if (SelectedCityId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("WTainlordCitySelectScreen: Confirm clicked but no city selected"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordCitySelectScreen: City confirmed (%s)"), *SelectedCityId.ToString());
	OnCityConfirmed.Broadcast();
}

void UWTainlordCitySelectScreen::OnBackClicked()
{
	UE_LOG(LogTemp, Log, TEXT("WTainlordCitySelectScreen: Cancelled"));
	OnCitySelectCancelled.Broadcast();
}

void UWTainlordCitySelectScreen::SetButtonText(UButton* Button, const FString& Text)
{
	::SetButtonText(Button, Text);
}

// ---------------------------------------------------------------------------
// Visual polish helpers
// ---------------------------------------------------------------------------

void UWTainlordCitySelectScreen::UpdateButtonHighlights()
{
	auto UpdateButton = [this](UButton* Button, int32 Index)
	{
		if (!Button || !ButtonIndexToCityId.IsValidIndex(Index))
		{
			return;
		}
		const bool bSelected = (ButtonIndexToCityId[Index] == SelectedCityId);
		ApplyButtonHighlight(Button, bSelected);
	};

	UpdateButton(CityButton0, 0);
	UpdateButton(CityButton1, 1);
	UpdateButton(CityButton2, 2);
	UpdateButton(CityButton3, 3);
}

void UWTainlordCitySelectScreen::ApplyButtonHighlight(UButton* Button, bool bSelected)
{
	if (!Button)
	{
		return;
	}

	// Selected: cyan highlight. Neutral: dark translucent.
	Button->SetBackgroundColor(bSelected
		? FLinearColor(0.25f, 0.70f, 0.85f, 1.0f)   // Cyan
		: FLinearColor(0.12f, 0.12f, 0.12f, 0.80f)); // Dark gray
}

void UWTainlordCitySelectScreen::ApplyUniformButtonSizes()
{
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

	ApplySize(CityButton0);
	ApplySize(CityButton1);
	ApplySize(CityButton2);
	ApplySize(CityButton3);
	ApplySize(ConfirmButton);
	ApplySize(BackButton);
}

void UWTainlordCitySelectScreen::ApplyFontSizes()
{
	// Title: 20
	SetTextBlockFontSize(TitleLabel, 20);

	// Action buttons: 18
	SetButtonFontSize(ConfirmButton, 18);
	SetButtonFontSize(BackButton, 18);

	// City name label: 18
	SetTextBlockFontSize(CityNameLabel, 18);

	// City description: 14
	SetTextBlockFontSize(CityDescriptionLabel, 14);

	// Allegiance and reputation labels: 14
	SetTextBlockFontSize(AllegianceLabel, 14);
	SetTextBlockFontSize(ReputationLabel, 14);

	// City buttons: 16
	SetButtonFontSize(CityButton0, 16);
	SetButtonFontSize(CityButton1, 16);
	SetButtonFontSize(CityButton2, 16);
	SetButtonFontSize(CityButton3, 16);
}

void UWTainlordCitySelectScreen::SetTextBlockFontSize(UTextBlock* TextBlock, int32 Size)
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = Size;
	TextBlock->SetFont(Font);
}

void UWTainlordCitySelectScreen::SetButtonFontSize(UButton* Button, int32 Size)
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
