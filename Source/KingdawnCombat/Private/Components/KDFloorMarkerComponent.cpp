// Copyright Kingdawn. All Rights Reserved.

#include "Components/KDFloorMarkerComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UKDFloorMarkerComponent::UKDFloorMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Defaults
	MarkerMaterial = nullptr;
	MarkerSize = FVector2D(10.f, 10.f); // Small single-point marker
	MarkerLifetime = 3.0f;
	FadeOutDuration = 0.5f;
}

void UKDFloorMarkerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UKDFloorMarkerComponent::SpawnMarkerAtLocation(const FVector& WorldLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Clear previous markers (only one marker at a time for click-to-move)
	ClearAllMarkers();

	// Create a new marker actor
	AActor* MarkerActor = CreateMarkerActor(WorldLocation);
	if (MarkerActor)
	{
		ActiveMarkers.Add(MarkerActor);

		// Set up auto-destroy timer
		if (MarkerLifetime > 0.f)
		{
			MarkerActor->SetLifeSpan(MarkerLifetime);
		}
	}
}

void UKDFloorMarkerComponent::ClearAllMarkers()
{
	for (int32 i = ActiveMarkers.Num() - 1; i >= 0; --i)
	{
		if (ActiveMarkers[i] && IsValid(ActiveMarkers[i]))
		{
			ActiveMarkers[i]->Destroy();
		}
	}
	ActiveMarkers.Empty();
}

AActor* UKDFloorMarkerComponent::CreateMarkerActor(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Spawn an empty actor to hold the decal
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
	if (!MarkerActor)
	{
		return nullptr;
	}

	// Create root component
	USceneComponent* RootComp = NewObject<USceneComponent>(MarkerActor, TEXT("MarkerRoot"));
	MarkerActor->SetRootComponent(RootComp);
	RootComp->RegisterComponent();
	RootComp->SetWorldLocation(Location);

	// Create a decal component on the marker actor
	UDecalComponent* Decal = NewObject<UDecalComponent>(MarkerActor, TEXT("MarkerDecal"));
	Decal->SetupAttachment(RootComp);
	Decal->RegisterComponent();

	// Position the decal slightly above the ground to avoid z-fighting
	FVector DecalLocation = Location + FVector(0.f, 0.f, 5.f);
	Decal->SetWorldLocation(DecalLocation);

	// Decal faces downward (rotated -90 degrees on X for floor projection)
	Decal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));

	// Set decal size
	// UDecalComponent::DecalSize is (X=ProjectionDepth, Y=Width, Z=Height)
	// All values are half-extents, so the visible size is 2x these values
	const float HalfWidth = MarkerSize.X;
	const float HalfHeight = MarkerSize.Y;
	const float ProjectionDepth = 128.f; // How deep the decal projects into surfaces
	Decal->DecalSize = FVector(ProjectionDepth, HalfWidth, HalfHeight);

	// Apply material if available
	if (MarkerMaterial)
	{
		Decal->SetDecalMaterial(MarkerMaterial);
	}

	// Set up fade out
	if (FadeOutDuration > 0.f && MarkerLifetime > 0.f)
	{
		const float FadeStartDelay = FMath::Max(0.f, MarkerLifetime - FadeOutDuration);
		Decal->SetFadeScreenSize(0.f);
		Decal->SetFadeOut(FadeStartDelay, FadeOutDuration, /*bDestroyOwnerAfterFadeOut=*/ false);
	}

	UE_LOG(LogTemp, Log, TEXT("KDFloorMarker: Spawned marker at (%.1f, %.1f, %.1f)"),
		Location.X, Location.Y, Location.Z);

	return MarkerActor;
}
