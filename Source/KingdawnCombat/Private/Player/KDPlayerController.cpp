// Copyright Kingdawn. All Rights Reserved.

#include "Player/KDPlayerController.h"
#include "Components/KDClickInputHandler.h"
#include "Components/KDHotbarComponent.h"
#include "Characters/KDCombatCharacter.h"
#include "GameFramework/SpringArmComponent.h"

AKDPlayerController::AKDPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	bRightClickHeld = false;

	// Create click handler component
	ClickHandler = CreateDefaultSubobject<UKDClickInputHandler>(TEXT("ClickHandler"));

	// Create hotbar component
	Hotbar = CreateDefaultSubobject<UKDHotbarComponent>(TEXT("Hotbar"));
}

void AKDPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AKDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		// Left click (for targeting / moving)
		InputComponent->BindAction("LeftClick", IE_Pressed, this, &AKDPlayerController::OnLeftClickPressed);

		// Right click (for camera control)
		InputComponent->BindAction("RightClick", IE_Pressed, this, &AKDPlayerController::OnRightClickPressed);
		InputComponent->BindAction("RightClick", IE_Released, this, &AKDPlayerController::OnRightClickReleased);

		// Camera axes (only active when right-click held)
		InputComponent->BindAxis("CameraYaw", this, &AKDPlayerController::OnCameraYawInput);
		InputComponent->BindAxis("CameraPitch", this, &AKDPlayerController::OnCameraPitchInput);

		// Target cycling
		InputComponent->BindAction("CycleTarget", IE_Pressed, this, &AKDPlayerController::OnCycleTargetPressed);
		InputComponent->BindAction("ClearTarget", IE_Pressed, this, &AKDPlayerController::OnClearTargetPressed);

		// Hotbar slots (keys 1-9)
		InputComponent->BindAction("HotbarSlot1", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot1);
		InputComponent->BindAction("HotbarSlot2", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot2);
		InputComponent->BindAction("HotbarSlot3", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot3);
		InputComponent->BindAction("HotbarSlot4", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot4);
		InputComponent->BindAction("HotbarSlot5", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot5);
		InputComponent->BindAction("HotbarSlot6", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot6);
		InputComponent->BindAction("HotbarSlot7", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot7);
		InputComponent->BindAction("HotbarSlot8", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot8);
		InputComponent->BindAction("HotbarSlot9", IE_Pressed, this, &AKDPlayerController::OnHotbarSlot9);

		// Hotbar bar/page switching (F1-F4)
		InputComponent->BindAction("HotbarBar1", IE_Pressed, this, &AKDPlayerController::OnHotbarBar1);
		InputComponent->BindAction("HotbarBar2", IE_Pressed, this, &AKDPlayerController::OnHotbarBar2);
		InputComponent->BindAction("HotbarBar3", IE_Pressed, this, &AKDPlayerController::OnHotbarBar3);
		InputComponent->BindAction("HotbarBar4", IE_Pressed, this, &AKDPlayerController::OnHotbarBar4);
	}
}

void AKDPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Force click handler to refresh its cached character reference
	if (ClickHandler)
	{
		ClickHandler->GetPossessedCharacter();
	}
}

AKDCombatCharacter* AKDPlayerController::GetKDCombatCharacter() const
{
	return Cast<AKDCombatCharacter>(GetPawn());
}

// --- Input Callbacks ---

void AKDPlayerController::OnLeftClickPressed()
{
	if (ClickHandler)
	{
		ClickHandler->OnLeftClickPressed();
	}
}

void AKDPlayerController::OnRightClickPressed()
{
	bRightClickHeld = true;
}

void AKDPlayerController::OnRightClickReleased()
{
	bRightClickHeld = false;
}

void AKDPlayerController::OnCameraYawInput(float Value)
{
	// Only rotate camera when right-click is held (Silkroad-style)
	if (bRightClickHeld && Value != 0.f)
	{
		AddYawInput(Value);
	}
}

void AKDPlayerController::OnCameraPitchInput(float Value)
{
	// Only rotate camera when right-click is held (Silkroad-style)
	if (bRightClickHeld && Value != 0.f)
	{
		AddPitchInput(Value);
	}
}

void AKDPlayerController::OnCycleTargetPressed()
{
	AKDCombatCharacter* KDChar = GetKDCombatCharacter();
	if (KDChar)
	{
		KDChar->OnCycleTargetInputPressed();
	}
}

void AKDPlayerController::OnClearTargetPressed()
{
	AKDCombatCharacter* KDChar = GetKDCombatCharacter();
	if (KDChar)
	{
		KDChar->OnClearTargetInputPressed();
	}
}

void AKDPlayerController::OnHotbarSlotPressed(int32 SlotIndex)
{
	if (Hotbar)
	{
		Hotbar->ActivateSlot(SlotIndex);
	}
}

void AKDPlayerController::OnHotbarBarPressed(int32 BarIndex)
{
	if (Hotbar)
	{
		Hotbar->SetActiveBar(BarIndex);
		UE_LOG(LogTemp, Log, TEXT("KDPlayerController: Switched to hotbar bar %d"), BarIndex);
	}
}
