// Copyright Kingdawn. All Rights Reserved.

#include "TainlordGameInstance.h"
#include "Customization/TainlordPreviewCharacter.h"
#include "Customization/TainlordCreationFlowController.h"
#include "Customization/WTainlordRaceSelectScreen.h"
#include "Customization/WTainlordCreationScreen.h"
#include "Customization/WTainlordMasterySelectScreen.h"
#include "Customization/WTainlordCitySelectScreen.h"
#include "Customization/TainlordCharacterProfileSubsystem.h"
#include "Customization/TainlordCharacterCreationLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void UTainlordGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Init"));

	// --- Create the flow controller ---
	FlowController = NewObject<UTainlordCreationFlowController>(this);

	// Bind to flow controller delegates
	FlowController->OnStageChanged.AddDynamic(this, &UTainlordGameInstance::HandleStageChanged);
	FlowController->OnFlowCompleted.AddDynamic(this, &UTainlordGameInstance::HandleFlowCompleted);
	FlowController->OnFlowAborted.AddDynamic(this, &UTainlordGameInstance::HandleFlowAborted);

	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: FlowController created and delegates bound"));

	// --- Bind to profile subsystem ---
	UTainlordCharacterProfileSubsystem* ProfileSubsystem = GetSubsystem<UTainlordCharacterProfileSubsystem>();
	if (ProfileSubsystem)
	{
		ProfileSubsystem->OnCreationFlowNeeded.AddDynamic(this, &UTainlordGameInstance::OnCreationFlowNeeded);

		// If the subsystem already determined creation is needed (no save found),
		// the delegate was broadcast during subsystem init — before we could bind.
		if (!ProfileSubsystem->HasActiveProfile())
		{
			UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: No active profile at init - deferring creation flow"));
			FTimerHandle DeferredHandle;
			GetTimerManager().SetTimer(DeferredHandle, FTimerDelegate::CreateUObject(
				this, &UTainlordGameInstance::OnCreationFlowNeeded), 0.2f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: Profile subsystem not found during Init"));
	}
}

void UTainlordGameInstance::Shutdown()
{
	// Unbind from profile subsystem
	UTainlordCharacterProfileSubsystem* ProfileSubsystem = GetSubsystem<UTainlordCharacterProfileSubsystem>();
	if (ProfileSubsystem)
	{
		ProfileSubsystem->OnCreationFlowNeeded.RemoveDynamic(this, &UTainlordGameInstance::OnCreationFlowNeeded);
	}

	// Unbind from flow controller
	if (FlowController)
	{
		FlowController->OnStageChanged.RemoveDynamic(this, &UTainlordGameInstance::HandleStageChanged);
		FlowController->OnFlowCompleted.RemoveDynamic(this, &UTainlordGameInstance::HandleFlowCompleted);
		FlowController->OnFlowAborted.RemoveDynamic(this, &UTainlordGameInstance::HandleFlowAborted);
		FlowController = nullptr;
	}

	Super::Shutdown();
}

// ---------------------------------------------------------------------------
// Profile subsystem callback
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OnCreationFlowNeeded()
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Creation flow triggered"));

	if (ActiveCreationWidget && ActiveCreationWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Widget already open - ignoring"));
		return;
	}

	if (!FlowController)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: FlowController is null!"));
		return;
	}

	// Start the flow — this fires OnStageChanged(RaceSelect, RaceSelect) which opens Stage 1
	FlowController->BeginFlow();
}

// ---------------------------------------------------------------------------
// Flow controller callbacks
// ---------------------------------------------------------------------------

void UTainlordGameInstance::HandleStageChanged(ECreationStage OldStage, ECreationStage NewStage)
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Stage changed %s -> %s"),
		*UEnum::GetValueAsString(OldStage),
		*UEnum::GetValueAsString(NewStage));

	// Close any existing widget before opening the new one
	CloseActiveWidget();

	// Open the widget for the new stage
	switch (NewStage)
	{
	case ECreationStage::RaceSelect:
		OpenRaceSelectScreen();
		break;

	case ECreationStage::Appearance:
		OpenAppearanceScreen();
		break;

	case ECreationStage::Mastery:
		OpenMasteryScreen();
		break;

	case ECreationStage::City:
		OpenCityScreen();
		break;

	case ECreationStage::Name:
		// Future: OpenNameScreen();
		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Name stage - no widget yet (should be skipped)"));
		break;

	case ECreationStage::Confirm:
		// Confirm stage: auto-advance via flow controller (AdvanceToNextStage at Confirm fires OnFlowCompleted)
		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Confirm stage reached - auto-completing"));
		if (FlowController)
		{
			FlowController->AdvanceToNextStage();
		}
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: Unknown stage"));
		break;
	}
}

void UTainlordGameInstance::HandleFlowCompleted(const FTainlordProfileData& CompletedProfile)
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Flow completed - saving profile (name=%s, race=%d, gender=%d, mastery=%s, city=%s)"),
		*CompletedProfile.CharacterName,
		static_cast<int32>(CompletedProfile.Race),
		static_cast<int32>(CompletedProfile.Gender),
		*CompletedProfile.SelectedMasteryId.ToString(),
		*CompletedProfile.SelectedCityId.ToString());

	CloseActiveWidget();

	// Save the completed profile through the existing save chain
	FString OutReason;
	if (!UTainlordCharacterCreationLibrary::ConfirmCreation(this, CompletedProfile, OutReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: ConfirmCreation failed - %s"), *OutReason);
		// Profile save failed — but flow is done. Log the issue.
		// Future: show error dialog to player
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Profile saved successfully"));
	}
}

void UTainlordGameInstance::HandleFlowAborted()
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Flow aborted - closing all widgets"));
	CloseActiveWidget();
}

// ---------------------------------------------------------------------------
// Widget-level callbacks (from screen widgets)
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OnRaceConfirmed()
{
	if (!FlowController)
	{
		return;
	}

	// Read the selected race from the widget
	if (UWTainlordRaceSelectScreen* RaceScreen = Cast<UWTainlordRaceSelectScreen>(ActiveCreationWidget))
	{
		ECharacterRace SelectedRace = RaceScreen->GetSelectedRace();

		// Commit to flow controller
		// Gender defaults to Male for now (future: gender picker in race select)
		FlowController->CommitRaceSelection(SelectedRace, ECharacterGender::Male);

		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Race confirmed (%d) - advancing via flow controller"),
			static_cast<int32>(SelectedRace));
	}

	// Advance — flow controller will fire OnStageChanged which opens the next screen
	FlowController->AdvanceToNextStage();
}

void UTainlordGameInstance::OnRaceSelectCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Race select cancelled"));

	if (FlowController)
	{
		FlowController->ReturnToPreviousStage(); // At first stage, this aborts
	}
}

void UTainlordGameInstance::OnCreationConfirmed()
{
	if (!FlowController)
	{
		return;
	}

	// Read appearance data from the creation screen's working profile
	if (UWTainlordCreationScreen* CreationScreen = Cast<UWTainlordCreationScreen>(ActiveCreationWidget))
	{
		const FTainlordProfileData& ScreenProfile = CreationScreen->GetWorkingProfile();

		// Commit appearance data to flow controller
		FlowController->CommitAppearanceData(ScreenProfile.AppearanceData);

		// Also commit the character name if one was entered
		if (!ScreenProfile.CharacterName.IsEmpty())
		{
			FlowController->CommitCharacterName(ScreenProfile.CharacterName);
		}

		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Appearance confirmed - advancing via flow controller"));
	}

	// Advance — this will skip Mastery/City/Name (if configured) and go to Confirm → OnFlowCompleted
	FlowController->AdvanceToNextStage();
}

void UTainlordGameInstance::OnCreationCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Appearance cancelled - returning via flow controller"));

	if (FlowController)
	{
		FlowController->ReturnToPreviousStage(); // Goes back to RaceSelect
	}
}

// ---------------------------------------------------------------------------
// Mastery screen callbacks
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OnMasteryConfirmed()
{
	if (!FlowController)
	{
		return;
	}

	// Read the selected mastery from the widget
	if (UWTainlordMasterySelectScreen* MasteryScreen = Cast<UWTainlordMasterySelectScreen>(ActiveCreationWidget))
	{
		FName SelectedMasteryId = MasteryScreen->GetSelectedMasteryId();

		// Commit to flow controller
		FlowController->CommitMasterySelection(SelectedMasteryId);

		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Mastery confirmed (%s) - advancing via flow controller"),
			*SelectedMasteryId.ToString());
	}

	// Advance — flow controller will fire OnStageChanged which opens the next screen
	FlowController->AdvanceToNextStage();
}

void UTainlordGameInstance::OnMasterySelectCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Mastery select cancelled"));

	if (FlowController)
	{
		FlowController->ReturnToPreviousStage(); // Goes back to Appearance
	}
}

// ---------------------------------------------------------------------------
// City screen callbacks
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OnCityConfirmed()
{
	if (!FlowController)
	{
		return;
	}

	// Read the selected city from the widget
	if (UWTainlordCitySelectScreen* CityScreen = Cast<UWTainlordCitySelectScreen>(ActiveCreationWidget))
	{
		FName SelectedCityId = CityScreen->GetSelectedCityId();

		// Commit to flow controller
		FlowController->CommitCitySelection(SelectedCityId);

		UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: City confirmed (%s) - advancing via flow controller"),
			*SelectedCityId.ToString());
	}

	// Advance — flow controller will fire OnStageChanged which opens the next screen
	FlowController->AdvanceToNextStage();
}

void UTainlordGameInstance::OnCitySelectCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: City select cancelled"));

	if (FlowController)
	{
		FlowController->ReturnToPreviousStage(); // Goes back to Mastery
	}
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

AActor* UTainlordGameInstance::FindPreviewCharacter() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> PreviewActors;
	UGameplayStatics::GetAllActorsOfClass(World, ATainlordPreviewCharacter::StaticClass(), PreviewActors);
	return (PreviewActors.Num() > 0) ? PreviewActors[0] : nullptr;
}

void UTainlordGameInstance::SetUIInputMode(UUserWidget* FocusWidget)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	if (FocusWidget)
	{
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
	PC->ConsoleCommand(TEXT("r.SetNearClipPlane 0.1"));
	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Applied UI preview near clip override (0.1)"));
}

void UTainlordGameInstance::CloseActiveWidget()
{
	if (!ActiveCreationWidget)
	{
		return;
	}

	ActiveCreationWidget->RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(false);
			UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Input mode restored to game-only"));
		}
	}

	ActiveCreationWidget = nullptr;
}

// ---------------------------------------------------------------------------
// Stage 1: Race Select
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OpenRaceSelectScreen()
{
	// Resolve widget class
	UClass* WidgetClass = nullptr;
	if (!RaceSelectWidgetClass.IsNull())
	{
		WidgetClass = RaceSelectWidgetClass.LoadSynchronous();
	}

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UUserWidget>(
			nullptr,
			TEXT("/Game/Tainlord/UI/Creation/WBP_RaceSelectScreen.WBP_RaceSelectScreen_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Cannot find Race Select widget class. "
			"Set RaceSelectWidgetClass in DefaultGame.ini or create WBP_RaceSelectScreen."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: World not ready - retrying Race Select in 0.5s"));
		FTimerHandle DeferredHandle;
		GetTimerManager().SetTimer(DeferredHandle, FTimerDelegate::CreateUObject(
			this, &UTainlordGameInstance::OpenRaceSelectScreen), 0.5f, false);
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: No PlayerController - retrying Race Select in 0.5s"));
		FTimerHandle DeferredHandle;
		GetTimerManager().SetTimer(DeferredHandle, FTimerDelegate::CreateUObject(
			this, &UTainlordGameInstance::OpenRaceSelectScreen), 0.5f, false);
		return;
	}

	ActiveCreationWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!ActiveCreationWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Failed to create Race Select widget"));
		return;
	}

	// Configure the race select screen
	if (UWTainlordRaceSelectScreen* RaceScreen = Cast<UWTainlordRaceSelectScreen>(ActiveCreationWidget))
	{
		RaceScreen->PreviewCharacter = FindPreviewCharacter();
		RaceScreen->OnRaceConfirmed.AddDynamic(this, &UTainlordGameInstance::OnRaceConfirmed);
		RaceScreen->OnRaceSelectCancelled.AddDynamic(this, &UTainlordGameInstance::OnRaceSelectCancelled);
		RaceScreen->InitRaceSelect();
	}

	ActiveCreationWidget->AddToViewport(100);
	SetUIInputMode(ActiveCreationWidget);

	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Race Select screen opened (Stage: RaceSelect)"));
}

// ---------------------------------------------------------------------------
// Stage 2: Appearance Customization
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OpenAppearanceScreen()
{
	// Resolve widget class
	UClass* WidgetClass = nullptr;
	if (!CreationWidgetClass.IsNull())
	{
		WidgetClass = CreationWidgetClass.LoadSynchronous();
	}

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UUserWidget>(
			nullptr,
			TEXT("/Game/Tainlord/UI/Creation/WBP_CharacterCreationScreen_Fresh.WBP_CharacterCreationScreen_Fresh_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Cannot find Appearance widget class"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: World is null - cannot open Appearance screen"));
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: No PlayerController - cannot open Appearance screen"));
		return;
	}

	ActiveCreationWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!ActiveCreationWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Failed to create Appearance widget"));
		return;
	}

	if (UWTainlordCreationScreen* CreationScreen = Cast<UWTainlordCreationScreen>(ActiveCreationWidget))
	{
		CreationScreen->PreviewCharacter = FindPreviewCharacter();

		// Bind to confirm/cancel delegates
		CreationScreen->OnCreationConfirmed.AddDynamic(this, &UTainlordGameInstance::OnCreationConfirmed);
		CreationScreen->OnCreationCancelled.AddDynamic(this, &UTainlordGameInstance::OnCreationCancelled);

		// Initialize the screen
		CreationScreen->InitCreationScreen();

		// Set the race from the flow controller's working profile (committed in Stage 1)
		if (FlowController)
		{
			CreationScreen->SetRace(FlowController->GetWorkingProfile().Race);
		}
	}

	ActiveCreationWidget->AddToViewport(100);
	SetUIInputMode(ActiveCreationWidget);

	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Appearance screen opened (Stage: Appearance, Race=%d)"),
		FlowController ? static_cast<int32>(FlowController->GetWorkingProfile().Race) : -1);
}

// ---------------------------------------------------------------------------
// Stage 3: Mastery Selection
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OpenMasteryScreen()
{
	// Resolve widget class
	UClass* WidgetClass = nullptr;
	if (!MasterySelectWidgetClass.IsNull())
	{
		WidgetClass = MasterySelectWidgetClass.LoadSynchronous();
	}

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UUserWidget>(
			nullptr,
			TEXT("/Game/Tainlord/UI/Creation/WBP_MasterySelectScreen.WBP_MasterySelectScreen_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Cannot find Mastery Select widget class. "
			"Set MasterySelectWidgetClass in DefaultGame.ini or create WBP_MasterySelectScreen."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: World not ready - retrying Mastery Select in 0.5s"));
		FTimerHandle DeferredHandle;
		GetTimerManager().SetTimer(DeferredHandle, FTimerDelegate::CreateUObject(
			this, &UTainlordGameInstance::OpenMasteryScreen), 0.5f, false);
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: No PlayerController - retrying Mastery Select in 0.5s"));
		FTimerHandle DeferredHandle;
		GetTimerManager().SetTimer(DeferredHandle, FTimerDelegate::CreateUObject(
			this, &UTainlordGameInstance::OpenMasteryScreen), 0.5f, false);
		return;
	}

	ActiveCreationWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!ActiveCreationWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Failed to create Mastery Select widget"));
		return;
	}

	// Configure the mastery select screen
	if (UWTainlordMasterySelectScreen* MasteryScreen = Cast<UWTainlordMasterySelectScreen>(ActiveCreationWidget))
	{
		MasteryScreen->OnMasteryConfirmed.AddDynamic(this, &UTainlordGameInstance::OnMasteryConfirmed);
		MasteryScreen->OnMasterySelectCancelled.AddDynamic(this, &UTainlordGameInstance::OnMasterySelectCancelled);
		MasteryScreen->InitMasterySelect();
	}

	ActiveCreationWidget->AddToViewport(100);
	SetUIInputMode(ActiveCreationWidget);

	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: Mastery Select screen opened (Stage: Mastery)"));
}

// ---------------------------------------------------------------------------
// Stage 4: City Selection
// ---------------------------------------------------------------------------

void UTainlordGameInstance::OpenCityScreen()
{
	// Resolve widget class
	UClass* WidgetClass = nullptr;
	if (!CitySelectWidgetClass.IsNull())
	{
		WidgetClass = CitySelectWidgetClass.LoadSynchronous();
	}

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UUserWidget>(
			nullptr,
			TEXT("/Game/Tainlord/UI/Creation/WBP_CitySelectScreen.WBP_CitySelectScreen_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Cannot find City Select widget class. "
			"Set CitySelectWidgetClass in DefaultGame.ini or create WBP_CitySelectScreen."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: World not ready - retrying City Select in 0.5s"));
		FTimerHandle DeferredHandle;
		GetTimerManager().SetTimer(DeferredHandle, FTimerDelegate::CreateUObject(
			this, &UTainlordGameInstance::OpenCityScreen), 0.5f, false);
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordGameInstance: No PlayerController - retrying City Select in 0.5s"));
		FTimerHandle DeferredHandle;
		GetTimerManager().SetTimer(DeferredHandle, FTimerDelegate::CreateUObject(
			this, &UTainlordGameInstance::OpenCityScreen), 0.5f, false);
		return;
	}

	ActiveCreationWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!ActiveCreationWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("TainlordGameInstance: Failed to create City Select widget"));
		return;
	}

	// Configure the city select screen
	if (UWTainlordCitySelectScreen* CityScreen = Cast<UWTainlordCitySelectScreen>(ActiveCreationWidget))
	{
		CityScreen->OnCityConfirmed.AddDynamic(this, &UTainlordGameInstance::OnCityConfirmed);
		CityScreen->OnCitySelectCancelled.AddDynamic(this, &UTainlordGameInstance::OnCitySelectCancelled);
		CityScreen->InitCitySelect();
	}

	ActiveCreationWidget->AddToViewport(100);
	SetUIInputMode(ActiveCreationWidget);

	UE_LOG(LogTemp, Log, TEXT("TainlordGameInstance: City Select screen opened (Stage: City)"));
}
