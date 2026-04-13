// Copyright Kingdawn. All Rights Reserved.

#include "Customization/WTainlordCreationPresetGrid.h"
#include "Customization/WTainlordCreationPresetCard.h"
#include "Customization/TainlordCharacterCreationLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

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

		for (int32 Index = 0; Index < 10; ++Index)
		{
			const FString Candidate = FString::Printf(TEXT("%s_%d"), BaseName, Index);
			if (UWidget* Suffix = Owner->WidgetTree->FindWidget(FName(*Candidate)))
			{
				return Cast<T>(Suffix);
			}
		}

		return nullptr;
	}
}

UWTainlordCreationPresetGrid::UWTainlordCreationPresetGrid(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SelectedPresetId = NAME_None;
	CurrentCategory = EPresetCategory::Head;
	GridColumns = 4;
}

void UWTainlordCreationPresetGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!PresetGridContainer)
	{
		PresetGridContainer = FindNamedWidgetFlexible<UPanelWidget>(this, TEXT("PresetGridContainer"));
	}
	if (!CategoryTitle)
	{
		CategoryTitle = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("CategoryTitle"));
	}
	if (!EmptyLabel)
	{
		EmptyLabel = FindNamedWidgetFlexible<UTextBlock>(this, TEXT("EmptyLabel"));
	}
}

// ---------------------------------------------------------------------------
// Population
// ---------------------------------------------------------------------------

void UWTainlordCreationPresetGrid::PopulateHeads(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race)
{
	ClearGrid();
	CurrentCategory = EPresetCategory::Head;

	if (CategoryTitle)
	{
		CategoryTitle->SetText(FText::FromString(TEXT("Head")));
	}

	// Add "None" option first
	CreateCard(NAME_None, EPresetCategory::Head, FText::FromString(TEXT("None")));

	// Populate from catalog
	TArray<FTainlordHeadEntry> Entries = UTainlordCharacterCreationLibrary::GetAvailableHeads(WorldContext, Gender, Race);
	for (const FTainlordHeadEntry& Entry : Entries)
	{
		CreateCard(Entry.Id, EPresetCategory::Head, FText::FromName(Entry.Id));
	}

	// Show empty label if no entries beyond "None"
	if (EmptyLabel)
	{
		EmptyLabel->SetVisibility(Entries.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnGridPopulated(Cards.Num());
}

void UWTainlordCreationPresetGrid::PopulateHair(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race)
{
	ClearGrid();
	CurrentCategory = EPresetCategory::Hair;

	if (CategoryTitle)
	{
		CategoryTitle->SetText(FText::FromString(TEXT("Hair")));
	}

	CreateCard(NAME_None, EPresetCategory::Hair, FText::FromString(TEXT("None")));

	TArray<FTainlordHairEntry> Entries = UTainlordCharacterCreationLibrary::GetAvailableHair(WorldContext, Gender, Race);
	for (const FTainlordHairEntry& Entry : Entries)
	{
		CreateCard(Entry.Id, EPresetCategory::Hair, FText::FromName(Entry.Id));
	}

	if (EmptyLabel)
	{
		EmptyLabel->SetVisibility(Entries.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnGridPopulated(Cards.Num());
}

void UWTainlordCreationPresetGrid::PopulateBeards(const UObject* WorldContext, ECharacterGender Gender, ECharacterRace Race)
{
	ClearGrid();
	CurrentCategory = EPresetCategory::Beard;

	if (CategoryTitle)
	{
		CategoryTitle->SetText(FText::FromString(TEXT("Beard")));
	}

	CreateCard(NAME_None, EPresetCategory::Beard, FText::FromString(TEXT("None")));

	TArray<FTainlordBeardEntry> Entries = UTainlordCharacterCreationLibrary::GetAvailableBeards(WorldContext, Gender, Race);
	for (const FTainlordBeardEntry& Entry : Entries)
	{
		CreateCard(Entry.Id, EPresetCategory::Beard, FText::FromName(Entry.Id));
	}

	if (EmptyLabel)
	{
		EmptyLabel->SetVisibility(Entries.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnGridPopulated(Cards.Num());
}

void UWTainlordCreationPresetGrid::PopulateSkinTones(const UObject* WorldContext)
{
	ClearGrid();
	CurrentCategory = EPresetCategory::SkinTone;

	if (CategoryTitle)
	{
		CategoryTitle->SetText(FText::FromString(TEXT("Skin Tone")));
	}

	TArray<FTainlordSkinToneEntry> Entries = UTainlordCharacterCreationLibrary::GetAvailableSkinTones(WorldContext);
	for (const FTainlordSkinToneEntry& Entry : Entries)
	{
		CreateSkinToneCard(Entry.Id, FText::FromName(Entry.Id), Entry.Color);
	}

	if (EmptyLabel)
	{
		EmptyLabel->SetVisibility(Entries.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnGridPopulated(Cards.Num());
}

void UWTainlordCreationPresetGrid::ClearGrid()
{
	if (PresetGridContainer)
	{
		PresetGridContainer->ClearChildren();
	}

	Cards.Empty();
	SelectedPresetId = NAME_None;
}

// ---------------------------------------------------------------------------
// Selection
// ---------------------------------------------------------------------------

bool UWTainlordCreationPresetGrid::SetSelectedPreset(FName PresetId)
{
	SelectedPresetId = PresetId;

	bool bFound = false;
	for (UWTainlordCreationPresetCard* Card : Cards)
	{
		if (Card)
		{
			const bool bMatch = (Card->GetPresetId() == PresetId);
			Card->SetSelected(bMatch);
			if (bMatch)
			{
				bFound = true;
			}
		}
	}

	BP_OnGridSelectionChanged(PresetId);
	return bFound;
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

void UWTainlordCreationPresetGrid::OnCardSelected(UWTainlordCreationPresetCard* Card)
{
	if (!Card)
	{
		return;
	}

	const FName NewId = Card->GetPresetId();

	// Deselect previous, select new
	for (UWTainlordCreationPresetCard* Existing : Cards)
	{
		if (Existing)
		{
			Existing->SetSelected(Existing == Card);
		}
	}

	SelectedPresetId = NewId;

	// Notify Blueprint
	BP_OnGridSelectionChanged(NewId);

	// Broadcast to parent screen
	OnPresetGridSelectionChanged.Broadcast(NewId, CurrentCategory);
}

UWTainlordCreationPresetCard* UWTainlordCreationPresetGrid::CreateCard(FName PresetId, EPresetCategory Category, const FText& DisplayName)
{
	if (!CardWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("WTainlordCreationPresetGrid: CardWidgetClass is null. Set a Blueprint card class."));
		return nullptr;
	}

	UWTainlordCreationPresetCard* Card = CreateWidget<UWTainlordCreationPresetCard>(this, CardWidgetClass);
	if (!Card)
	{
		UE_LOG(LogTemp, Warning, TEXT("WTainlordCreationPresetGrid: Failed to create card widget"));
		return nullptr;
	}

	Card->SetupCard(PresetId, Category, DisplayName);
	Card->OnPresetCardSelected.AddDynamic(this, &UWTainlordCreationPresetGrid::OnCardSelected);

	AddCardToGrid(Card);
	Cards.Add(Card);

	return Card;
}

UWTainlordCreationPresetCard* UWTainlordCreationPresetGrid::CreateSkinToneCard(FName PresetId, const FText& DisplayName, const FLinearColor& Color)
{
	if (!CardWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("WTainlordCreationPresetGrid: CardWidgetClass is null. Set a Blueprint card class."));
		return nullptr;
	}

	UWTainlordCreationPresetCard* Card = CreateWidget<UWTainlordCreationPresetCard>(this, CardWidgetClass);
	if (!Card)
	{
		return nullptr;
	}

	Card->SetupSkinToneCard(PresetId, DisplayName, Color);
	Card->OnPresetCardSelected.AddDynamic(this, &UWTainlordCreationPresetGrid::OnCardSelected);

	AddCardToGrid(Card);
	Cards.Add(Card);

	return Card;
}

void UWTainlordCreationPresetGrid::AddCardToGrid(UWTainlordCreationPresetCard* Card)
{
	if (!Card || !PresetGridContainer)
	{
		return;
	}

	PresetGridContainer->AddChild(Card);
}
