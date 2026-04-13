// Copyright Kingdawn. All Rights Reserved.
// Player controller for Silkroad-style MMO combat.
// Owns KDClickInputHandler for click-to-move/click-to-target.
// Owns KDHotbarComponent for ability slot management (1-9 keys, F1-F4 bars).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KDPlayerController.generated.h"

class UKDClickInputHandler;
class UKDHotbarComponent;
class AKDCombatCharacter;

/**
 * Player controller for Kingdawn MMO combat.
 * Owns KDClickInputHandler for Silkroad-style click-to-move/click-to-target.
 * Owns KDHotbarComponent for ability slot activation via number keys.
 * Right-click hold = camera orbit. Left-click = target/move.
 * 1-9 = hotbar slots on active bar. F1-F4 = switch active hotbar bar/page.
 */
UCLASS()
class KINGDAWNCOMBAT_API AKDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AKDPlayerController();

	/** Get the possessed combat character (null if not a KDCombatCharacter). */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	AKDCombatCharacter* GetKDCombatCharacter() const;

	/** Get the hotbar component. */
	UFUNCTION(BlueprintPure, Category = "Kingdawn")
	UKDHotbarComponent* GetHotbar() const { return Hotbar; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	/** Click input handler component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDClickInputHandler> ClickHandler;

	/** Hotbar component for ability slot management. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdawn|Components")
	TObjectPtr<UKDHotbarComponent> Hotbar;

private:
	/** Input callbacks. */
	void OnLeftClickPressed();
	void OnRightClickPressed();
	void OnRightClickReleased();

	/** Camera input (only active while right-click held). */
	void OnCameraYawInput(float Value);
	void OnCameraPitchInput(float Value);

	/** Target cycling (Tab key). */
	void OnCycleTargetPressed();

	/** Clear target (Escape key). */
	void OnClearTargetPressed();

	/** Hotbar slot activation (keys 1-9). */
	void OnHotbarSlot1() { OnHotbarSlotPressed(0); }
	void OnHotbarSlot2() { OnHotbarSlotPressed(1); }
	void OnHotbarSlot3() { OnHotbarSlotPressed(2); }
	void OnHotbarSlot4() { OnHotbarSlotPressed(3); }
	void OnHotbarSlot5() { OnHotbarSlotPressed(4); }
	void OnHotbarSlot6() { OnHotbarSlotPressed(5); }
	void OnHotbarSlot7() { OnHotbarSlotPressed(6); }
	void OnHotbarSlot8() { OnHotbarSlotPressed(7); }
	void OnHotbarSlot9() { OnHotbarSlotPressed(8); }
	void OnHotbarSlotPressed(int32 SlotIndex);

	/** Hotbar bar/page switching (F1-F4). */
	void OnHotbarBar1() { OnHotbarBarPressed(0); }
	void OnHotbarBar2() { OnHotbarBarPressed(1); }
	void OnHotbarBar3() { OnHotbarBarPressed(2); }
	void OnHotbarBar4() { OnHotbarBarPressed(3); }
	void OnHotbarBarPressed(int32 BarIndex);

	/** Right-click state for camera control. */
	bool bRightClickHeld;
};
