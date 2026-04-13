// Copyright Kingdawn. All Rights Reserved.
// Floor marker component for click-to-move visualization.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "KDFloorMarkerComponent.generated.h"

class UDecalComponent;

/**
 * Floor marker component that spawns a visual decal at clicked locations.
 * Automatically manages decal lifetime and cleanup.
 */
UCLASS(Blueprintable, ClassGroup = (Kingdawn), meta = (BlueprintSpawnableComponent))
class KINGDAWNCOMBAT_API UKDFloorMarkerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDFloorMarkerComponent();

	/**
	 * Spawn a floor marker at the given world location.
	 * The marker will automatically be destroyed after MarkerLifetime seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Movement")
	void SpawnMarkerAtLocation(const FVector& WorldLocation);

	/**
	 * Clear all active floor markers immediately.
	 */
	UFUNCTION(BlueprintCallable, Category = "Kingdawn|Movement")
	void ClearAllMarkers();

	// --- Configuration ---

	/** Decal material to use for the floor marker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Movement|Visual")
	TObjectPtr<UMaterialInterface> MarkerMaterial;

	/** Size of the floor marker decal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Movement|Visual")
	FVector2D MarkerSize;

	/** How long the marker should persist (seconds). 0 = infinite. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Movement|Visual")
	float MarkerLifetime;

	/** Fade out duration (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn|Movement|Visual")
	float FadeOutDuration;

protected:
	virtual void BeginPlay() override;

private:
	/** All active marker actors. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveMarkers;

	/** Create a single marker actor at the given location. */
	AActor* CreateMarkerActor(const FVector& Location);
};
