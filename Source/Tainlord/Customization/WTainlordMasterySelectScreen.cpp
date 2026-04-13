// Copyright Kingdawn. All Rights Reserved.

#include "Customization/WTainlordMasterySelectScreen.h"
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

UWTainlordMasterySelectScreen::UWTainlordMasterySelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWTainlordMasterySelectScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Flexible widget finding (supports _0, _1 suffixes from BP)
	if (!MasteryButton0)
	{
		MasteryButton0 = FindNamedWidgetFlexible<UButton>(this, TEXT("MasteryButton0"));
	}
	if (!MasteryButton1)
	{
		MasteryButton1 = FindNamedWidgetFlexible<UButton>(this, TEXT("MasteryButton1"));
	}
	if (!MasteryButton2)
	{
		MasteryButton2 = FindNamedWidgetFlexible<UButton>(this, TEXT("MasteryButton2"));
	}
	if (!MasteryButton3)
	{
		MasteryButton3 = FindNamedWidgetFlexible<UButton>(this, TEXT("MasteryButton3"));
	}
	if (!MasteryButton4)
	{
		MasteryButton4 = FindNamedWidgetFlexible<UButton>(this, TEXT("MasteryButton4"));
	}
	if (!MasteryButton5)
	{
		MasteryButton5 = FindNamedWidgetFlexible<UButton>(this, TEXT("MasteryButton5"));
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
	if (!SelectedMasteryLabel)
	{
		SelectedMasteryLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("SelectedMasteryLabel"));
	}
	if (!MasteryDescriptionLabel)
	{
		MasteryDescriptionLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("MasteryDescriptionLabel"));
	}

	// Bind mastery buttons
	if (MasteryButton0)
	{
		MasteryButton0->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnMasteryButton0Clicked);
	}
	if (MasteryButton1)
	{
		MasteryButton1->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnMasteryButton1Clicked);
	}
	if (MasteryButton2)
	{
		MasteryButton2->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnMasteryButton2Clicked);
	}
	if (MasteryButton3)
	{
		MasteryButton3->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnMasteryButton3Clicked);
	}
	if (MasteryButton4)
	{
		MasteryButton4->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnMasteryButton4Clicked);
	}
	if (MasteryButton5)
	{
		MasteryButton5->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnMasteryButton5Clicked);
	}

	// Bind action buttons
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnConfirmClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UWTainlordMasterySelectScreen::OnBackClicked);
	}
}

void UWTainlordMasterySelectScreen::NativeConstruct()
{
	Super::NativeConstruct();

	SetButtonText(ConfirmButton, TEXT("Select"));
	SetButtonText(BackButton, TEXT("Back"));

	if (TitleLabel)
	{
		TitleLabel->SetText(FText::FromString(TEXT("SELECT YOUR MASTERY")));
	}

	// Visual polish
	ApplyUniformButtonSizes();
	ApplyFontSizes();
}

void UWTainlordMasterySelectScreen::InitMasterySelect()
{
	// Load demo masteries from catalog
	CachedMasteries = UTainlordDemoCatalogLibrary::GetDemoMasteries();

	UE_LOG(LogTemp, Log, TEXT("WTainlordMasterySelectScreen: Loaded %d demo masteries"), CachedMasteries.Num());

	// Map button indices to mastery IDs
	ButtonIndexToMasteryId.SetNum(6);
	for (int32 i = 0; i < FMath::Min(6, CachedMasteries.Num()); ++i)
	{
		ButtonIndexToMasteryId[i] = CachedMasteries[i].MasteryId;

		// Set button text to mastery display name
		UButton* Button = nullptr;
		switch (i)
		{
		case 0: Button = MasteryButton0; break;
		case 1: Button = MasteryButton1; break;
		case 2: Button = MasteryButton2; break;
		case 3: Button = MasteryButton3; break;
		case 4: Button = MasteryButton4; break;
		case 5: Button = MasteryButton5; break;
		}

		if (Button)
		{
			SetButtonText(Button, CachedMasteries[i].DisplayName.ToString());
			Button->SetVisibility(CachedMasteries[i].bIncludedInFundingDemo ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
	}

	// Hide unused buttons
	for (int32 i = CachedMasteries.Num(); i < 6; ++i)
	{
		UButton* Button = nullptr;
		switch (i)
		{
		case 0: Button = MasteryButton0; break;
		case 1: Button = MasteryButton1; break;
		case 2: Button = MasteryButton2; break;
		case 3: Button = MasteryButton3; break;
		case 4: Button = MasteryButton4; break;
		case 5: Button = MasteryButton5; break;
		}

		if (Button)
		{
			Button->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Select the first mastery by default
	if (CachedMasteries.Num() > 0)
	{
		SelectMastery(CachedMasteries[0].MasteryId);
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordMasterySelectScreen: Initialized (default mastery=%s)"),
		*SelectedMasteryId.ToString());
}

void UWTainlordMasterySelectScreen::SelectMastery(FName MasteryId)
{
	if (SelectedMasteryId == MasteryId)
	{
		return;
	}

	SelectedMasteryId = MasteryId;
	RefreshMasteryDisplay();
	UpdateButtonHighlights();

	// Find the mastery definition and notify Blueprint
	const FTainlordMasteryDefinition* MasteryDef = CachedMasteries.FindByPredicate(
		[MasteryId](const FTainlordMasteryDefinition& Def) { return Def.MasteryId == MasteryId; });

	if (MasteryDef)
	{
		BP_OnMasteryChanged(*MasteryDef);
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordMasterySelectScreen: Mastery changed to %s"), *MasteryId.ToString());
}

void UWTainlordMasterySelectScreen::RefreshMasteryDisplay()
{
	if (SelectedMasteryLabel)
	{
		const FTainlordMasteryDefinition* MasteryDef = CachedMasteries.FindByPredicate(
			[this](const FTainlordMasteryDefinition& Def) { return Def.MasteryId == SelectedMasteryId; });

		if (MasteryDef)
		{
			SelectedMasteryLabel->SetText(MasteryDef->DisplayName);

			if (MasteryDescriptionLabel)
			{
				MasteryDescriptionLabel->SetText(MasteryDef->ShortDescription);
			}
		}
		else
		{
			SelectedMasteryLabel->SetText(FText::FromString(TEXT("None")));
			if (MasteryDescriptionLabel)
			{
				MasteryDescriptionLabel->SetText(FText::GetEmpty());
			}
		}
	}
}

void UWTainlordMasterySelectScreen::OnMasteryButton0Clicked()
{
	if (ButtonIndexToMasteryId.IsValidIndex(0) && !ButtonIndexToMasteryId[0].IsNone())
	{
		SelectMastery(ButtonIndexToMasteryId[0]);
	}
}

void UWTainlordMasterySelectScreen::OnMasteryButton1Clicked()
{
	if (ButtonIndexToMasteryId.IsValidIndex(1) && !ButtonIndexToMasteryId[1].IsNone())
	{
		SelectMastery(ButtonIndexToMasteryId[1]);
	}
}

void UWTainlordMasterySelectScreen::OnMasteryButton2Clicked()
{
	if (ButtonIndexToMasteryId.IsValidIndex(2) && !ButtonIndexToMasteryId[2].IsNone())
	{
		SelectMastery(ButtonIndexToMasteryId[2]);
	}
}

void UWTainlordMasterySelectScreen::OnMasteryButton3Clicked()
{
	if (ButtonIndexToMasteryId.IsValidIndex(3) && !ButtonIndexToMasteryId[3].IsNone())
	{
		SelectMastery(ButtonIndexToMasteryId[3]);
	}
}

void UWTainlordMasterySelectScreen::OnMasteryButton4Clicked()
{
	if (ButtonIndexToMasteryId.IsValidIndex(4) && !ButtonIndexToMasteryId[4].IsNone())
	{
		SelectMastery(ButtonIndexToMasteryId[4]);
	}
}

void UWTainlordMasterySelectScreen::OnMasteryButton5Clicked()
{
	if (ButtonIndexToMasteryId.IsValidIndex(5) && !ButtonIndexToMasteryId[5].IsNone())
	{
		SelectMastery(ButtonIndexToMasteryId[5]);
	}
}

void UWTainlordMasterySelectScreen::OnConfirmClicked()
{
	if (SelectedMasteryId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("WTainlordMasterySelectScreen: Confirm clicked but no mastery selected"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WTainlordMasterySelectScreen: Mastery confirmed (%s)"), *SelectedMasteryId.ToString());
	OnMasteryConfirmed.Broadcast();
}

void UWTainlordMasterySelectScreen::OnBackClicked()
{
	UE_LOG(LogTemp, Log, TEXT("WTainlordMasterySelectScreen: Cancelled"));
	OnMasterySelectCancelled.Broadcast();
}

void UWTainlordMasterySelectScreen::SetButtonText(UButton* Button, const FString& Text)
{
	::SetButtonText(Button, Text);
}

// ---------------------------------------------------------------------------
// Visual polish helpers
// ---------------------------------------------------------------------------

void UWTainlordMasterySelectScreen::UpdateButtonHighlights()
{
	auto UpdateButton = [this](UButton* Button, int32 Index)
	{
		if (!Button || !ButtonIndexToMasteryId.IsValidIndex(Index))
		{
			return;
		}
		const bool bSelected = (ButtonIndexToMasteryId[Index] == SelectedMasteryId);
		ApplyButtonHighlight(Button, bSelected);
	};

	UpdateButton(MasteryButton0, 0);
	UpdateButton(MasteryButton1, 1);
	UpdateButton(MasteryButton2, 2);
	UpdateButton(MasteryButton3, 3);
	UpdateButton(MasteryButton4, 4);
	UpdateButton(MasteryButton5, 5);
}

void UWTainlordMasterySelectScreen::ApplyButtonHighlight(UButton* Button, bool bSelected)
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

void UWTainlordMasterySelectScreen::ApplyUniformButtonSizes()
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

	ApplySize(MasteryButton0);
	ApplySize(MasteryButton1);
	ApplySize(MasteryButton2);
	ApplySize(MasteryButton3);
	ApplySize(MasteryButton4);
	ApplySize(MasteryButton5);
	ApplySize(ConfirmButton);
	ApplySize(BackButton);
}

void UWTainlordMasterySelectScreen::ApplyFontSizes()
{
	// Title: 20
	SetTextBlockFontSize(TitleLabel, 20);

	// Action buttons: 18
	SetButtonFontSize(ConfirmButton, 18);
	SetButtonFontSize(BackButton, 18);

	// Selected mastery label: 16
	SetTextBlockFontSize(SelectedMasteryLabel, 16);

	// Mastery description: 14
	SetTextBlockFontSize(MasteryDescriptionLabel, 14);

	// Mastery buttons: 16
	SetButtonFontSize(MasteryButton0, 16);
	SetButtonFontSize(MasteryButton1, 16);
	SetButtonFontSize(MasteryButton2, 16);
	SetButtonFontSize(MasteryButton3, 16);
	SetButtonFontSize(MasteryButton4, 16);
	SetButtonFontSize(MasteryButton5, 16);
}

void UWTainlordMasterySelectScreen::SetTextBlockFontSize(UTextBlock* TextBlock, int32 Size)
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = Size;
	TextBlock->SetFont(Font);
}

void UWTainlordMasterySelectScreen::SetButtonFontSize(UButton* Button, int32 Size)
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
