// Copyright Kingdawn. All Rights Reserved.
// Adapted from CollisionsManager/ACMCollisionManagerComponent.cpp
// Removed: ACFAnimInstance, ACMCollisionsMasterComponent, ACFRPGFunctionLibrary, ACFTeamManagerSubsystem
// Replaced: Team-based damage gating with simple IKDEntityInterface team check

#include "Components/KDCollisionComponent.h"
#include "Interfaces/KDEntityInterface.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

UKDCollisionComponent::UKDCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bIsStarted = false;
	bSingleTimedTraceStarted = false;
	bAllTimedTraceStarted = false;
	bAllowMultipleHitsPerSwing = false;
	bIgnoreOwner = true;
	bAutoApplyDamage = true;
	ShowDebugInfo = EKDDDebugType::None;
	DebugInactiveColor = FLinearColor::Blue;
	DebugActiveColor = FLinearColor::Red;
}

void UKDCollisionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetStarted(false);
}

void UKDCollisionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GetOwner()) && GetOwner()->HasAuthority())
	{
		StopCurrentAreaDamage();
		StopAllTraces();
	}
	Super::EndPlay(EndPlayReason);
}

void UKDCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsStarted)
	{
		UpdateCollisions();
	}
}

// --- Setup ---

void UKDCollisionComponent::SetupCollisionManager(UMeshComponent* InDamageMesh,
	const TArray<TEnumAsByte<ECollisionChannel>>& DefaultChannels)
{
	DamageMesh = InDamageMesh;

	if (!DamageMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCollisionComponent::SetupCollisionManager - Invalid Damage Mesh!"));
		return;
	}

	for (const TEnumAsByte<ECollisionChannel>& Channel : DefaultChannels)
	{
		CollisionChannels.AddUnique(Channel);
	}

	if (DamageTraces.Num() == 0 && DamageMesh &&
		DamageMesh->DoesSocketExist(TEXT("lowerarm_r")) &&
		DamageMesh->DoesSocketExist(TEXT("hand_r")))
	{
		FKDTraceInfo DefaultTrace;
		DefaultTrace.StartSocket = TEXT("lowerarm_r");
		DefaultTrace.EndSocket = TEXT("hand_r");
		DefaultTrace.Radius = 20.f;
		DefaultTrace.BaseDamage = 10.f;
		DefaultTrace.DamageType = EKDDamageType::Point;
		DamageTraces.Add(TEXT("DefaultMelee"), DefaultTrace);

		UE_LOG(LogTemp, Log, TEXT("KDCollisionComponent: Auto-created DefaultMelee trace using lowerarm_r -> hand_r"));
	}
}

void UKDCollisionComponent::SetActorOwner(AActor* NewOwner)
{
	ActorOwner = NewOwner;
}

AActor* UKDCollisionComponent::GetActorOwner() const
{
	if (ActorOwner)
	{
		return ActorOwner;
	}
	return GetOwner();
}

// --- Trace Control ---

void UKDCollisionComponent::StartAllTraces()
{
	ActivatedTraces.Empty();
	PendingDelete.Empty();

	if (DamageTraces.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("KDCollisionComponent: StartAllTraces called with no configured DamageTraces"));
		return;
	}

	for (const auto& Damage : DamageTraces)
	{
		StartSingleTrace(Damage.Key);
	}
}

void UKDCollisionComponent::StopAllTraces()
{
	PendingDelete.Empty();
	for (const auto& Trace : ActivatedTraces)
	{
		StopSingleTrace(Trace.Key);
	}
}

void UKDCollisionComponent::StartSingleTrace(const FName& Name)
{
	FKDTraceInfo* OutTrace = DamageTraces.Find(Name);
	if (OutTrace)
	{
		if (PendingDelete.Contains(Name))
		{
			PendingDelete.Remove(Name);
		}
		OutTrace->bIsFirstFrame = true;
		ActivatedTraces.Add(Name, *OutTrace);
		SetStarted(true);
		PlayTrails(Name);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCollisionComponent::StartSingleTrace - Invalid Trace Name: %s"), *Name.ToString());
	}
}

void UKDCollisionComponent::StopSingleTrace(const FName& Name)
{
	if (ActivatedTraces.Contains(Name))
	{
		StopTrails(Name);
		PendingDelete.AddUnique(Name);

		FKDHitActors* AlreadyHit = AlreadyHitActors.Find(Name);
		if (AlreadyHit)
		{
			AlreadyHit->AlreadyHitActors.Empty();
		}
	}
}

void UKDCollisionComponent::StartTimedSingleTrace(const FName& TraceName, float Duration)
{
	UWorld* World = GetWorld();
	if (World)
	{
		StartSingleTrace(TraceName);
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;

		TimerDelegate.BindLambda([this, TraceName]() { HandleTimedSingleTraceFinished(TraceName); });
		TraceTimers.Add(TraceName, TimerHandle);
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
	}
}

void UKDCollisionComponent::StartAllTimedTraces(float Duration)
{
	UWorld* World = GetWorld();
	if (World && !bAllTimedTraceStarted)
	{
		StartAllTraces();
		World->GetTimerManager().SetTimer(AllTraceTimer, this,
			&UKDCollisionComponent::HandleAllTimedTraceFinished, Duration, false);
		bAllTimedTraceStarted = true;
	}
}

bool UKDCollisionComponent::IsTraceActive(const FName& TraceName) const
{
	return ActivatedTraces.Contains(TraceName);
}

// --- Single-Shot Traces ---

void UKDCollisionComponent::PerformSwipeTraceShot(const FVector& Start, const FVector& End, float Radius)
{
	FHitResult OutHit;
	PerformSwipeTraceShot_Local(Start, End, Radius, OutHit);
}

bool UKDCollisionComponent::PerformSwipeTraceShot_Local(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit)
{
	if (!GetActorOwner())
	{
		return false;
	}

	FCollisionQueryParams Params;
	if (IgnoredActors.Num() > 0)
	{
		Params.AddIgnoredActors(IgnoredActors);
	}
	if (bIgnoreOwner)
	{
		Params.AddIgnoredActor(GetActorOwner());
		Params.AddIgnoredActor(GetOwner());
	}

	Params.bReturnPhysicalMaterial = true;
	Params.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	for (const TEnumAsByte<ECollisionChannel>& Channel : CollisionChannels)
	{
		if (ObjectParams.IsValidObjectQuery(Channel))
		{
			ObjectParams.AddObjectTypesToQuery(Channel);
		}
	}

	if (!ObjectParams.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("KDCollisionComponent - Invalid Collision Channel"));
		return false;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		const FRotator Orientation = GetLineRotation(Start, End);
		FHitResult OutResult;
		const bool bHit = World->SweepSingleByObjectType(
			OutResult, Start, End, Orientation.Quaternion(), ObjectParams, FCollisionShape::MakeSphere(Radius), Params);

		if (bHit)
		{
			ApplyDamage(OutResult, SwipeTraceInfo);
			OutHit = OutResult;
			OnCollisionDetected.Broadcast(OutResult);
		}

		if (ShowDebugInfo != EKDDDebugType::None)
		{
			ShowDebugTrace(Start, End, Radius, 3.f, FLinearColor::Red);
		}
		return bHit;
	}
	return false;
}

// --- Area Damage ---

void UKDCollisionComponent::StartAreaDamage(const FVector& DamageCenter, float DamageRadius,
	float DamageInterval, TSubclassOf<UDamageType> DamageTypeOverride)
{
	UWorld* World = GetWorld();
	if (World)
	{
		CurrentAreaDamage.Location = DamageCenter;
		CurrentAreaDamage.Radius = DamageRadius;
		CurrentAreaDamage.bIsActive = true;

		PerformAreaDamage_Single(DamageCenter, DamageRadius, DamageTypeOverride);
		World->GetTimerManager().SetTimer(
			CurrentAreaDamage.AreaLoopTimer, this, &UKDCollisionComponent::HandleAreaDamageLooping, DamageInterval, true);
	}
}

void UKDCollisionComponent::StopCurrentAreaDamage()
{
	if (CurrentAreaDamage.bIsActive)
	{
		CurrentAreaDamage.bIsActive = false;
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(CurrentAreaDamage.AreaLoopTimer);
		}
	}
}

void UKDCollisionComponent::PerformAreaDamage_Single(const FVector& DamageCenter, float DamageRadius,
	TSubclassOf<UDamageType> DamageTypeOverride)
{
	FCollisionQueryParams Params;
	if (IgnoredActors.Num() > 0)
	{
		Params.AddIgnoredActors(IgnoredActors);
	}
	if (bIgnoreOwner)
	{
		Params.AddIgnoredActor(GetActorOwner());
	}

	UWorld* World = GetWorld();
	TArray<TObjectPtr<AActor>> AlreadyHitActorsBySphere;
	TArray<FHitResult> OutHits;

	if (World)
	{
		for (const TEnumAsByte<ECollisionChannel>& Channel : CollisionChannels)
		{
			TArray<FHitResult> OutResults;
			const bool bHit = World->SweepMultiByChannel(OutResults, DamageCenter, DamageCenter + FVector(1.f),
				FQuat::Identity, Channel, FCollisionShape::MakeSphere(DamageRadius), Params);
			if (bHit)
			{
				OutHits.Append(OutResults);
			}
		}

		for (const auto& Hit : OutHits)
		{
			if (!AlreadyHitActorsBySphere.Contains(Hit.GetActor()))
			{
				AlreadyHitActorsBySphere.Add(Hit.GetActor());
				FKDBaseTraceInfo DamageInfo = AreaDamageTraceInfo;
				if (DamageTypeOverride)
				{
					DamageInfo.DamageTypeClass = DamageTypeOverride;
				}
				ApplyDamage(Hit, DamageInfo);
			}
		}

		if (ShowDebugInfo != EKDDDebugType::None)
		{
			ShowDebugTrace(DamageCenter, DamageCenter + FVector(1.f), DamageRadius, 3.f, FLinearColor::Red);
		}
	}
}

void UKDCollisionComponent::PerformAreaDamageForDuration(const FVector& DamageCenter, float DamageRadius,
	float Duration, float DamageInterval)
{
	UWorld* World = GetWorld();
	if (World)
	{
		StartAreaDamage(DamageCenter, DamageRadius, DamageInterval);
		World->GetTimerManager().SetTimer(
			AreaDamageTimer, this, &UKDCollisionComponent::HandleAreaDamageFinished,
			Duration, false);
	}
}

// --- Configuration ---

void UKDCollisionComponent::AddActorToIgnore(AActor* IgnoredActor)
{
	IgnoredActors.AddUnique(IgnoredActor);
}

void UKDCollisionComponent::AddCollisionChannel(TEnumAsByte<ECollisionChannel> InTraceChannel)
{
	CollisionChannels.AddUnique(InTraceChannel);
}

void UKDCollisionComponent::AddCollisionChannels(TArray<TEnumAsByte<ECollisionChannel>> InTraceChannels)
{
	for (const TEnumAsByte<ECollisionChannel>& Chan : InTraceChannels)
	{
		AddCollisionChannel(Chan);
	}
}

void UKDCollisionComponent::ClearCollisionChannels()
{
	CollisionChannels.Empty();
}

void UKDCollisionComponent::SetTraceConfig(const FName& TraceName, const FKDTraceInfo& TraceInfo)
{
	DamageTraces.Add(TraceName, TraceInfo);
}

// --- Internal ---

void UKDCollisionComponent::UpdateCollisions()
{
	if (!DamageMesh)
	{
		return;
	}

	DisplayDebugTraces();

	// Process pending deletions
	if (PendingDelete.IsValidIndex(0))
	{
		for (const auto& ToDelete : PendingDelete)
		{
			ActivatedTraces.Remove(ToDelete);
			AlreadyHitActors.Remove(ToDelete);
		}
		PendingDelete.Empty();
	}

	if (ActivatedTraces.Num() == 0)
	{
		SetStarted(false);
		return;
	}

	if (!CollisionChannels.IsValidIndex(0))
	{
		SetStarted(false);
		return;
	}

	for (TPair<FName, FKDTraceInfo>& CurrentTrace : ActivatedTraces)
	{
		if (!DamageMesh->DoesSocketExist(CurrentTrace.Value.StartSocket) ||
			!DamageMesh->DoesSocketExist(CurrentTrace.Value.EndSocket))
		{
			UE_LOG(LogTemp, Warning, TEXT("KDCollisionComponent - Invalid Socket Names for trace: %s"), *CurrentTrace.Key.ToString());
			continue;
		}

		FVector StartPos = DamageMesh->GetSocketLocation(CurrentTrace.Value.StartSocket);
		FVector EndPos = DamageMesh->GetSocketLocation(CurrentTrace.Value.EndSocket);

		if (CurrentTrace.Key == TEXT("DefaultMelee") && GetActorOwner())
		{
			const FVector Forward2D = GetActorOwner()->GetActorForwardVector().GetSafeNormal2D();
			StartPos += Forward2D * 15.f;
			EndPos += Forward2D * 140.f;
		}

		const FRotator Orientation = GetLineRotation(StartPos, EndPos);

		FCollisionQueryParams Params;
		if (IgnoredActors.Num() > 0)
		{
			Params.AddIgnoredActors(IgnoredActors);
		}
		if (bIgnoreOwner)
		{
			Params.AddIgnoredActor(GetActorOwner());
			Params.AddIgnoredActor(GetOwner());
		}
		Params.bReturnPhysicalMaterial = true;
		Params.bTraceComplex = true;

		FCollisionObjectQueryParams ObjectParams;
		for (const TEnumAsByte<ECollisionChannel>& Channel : CollisionChannels)
		{
			if (ObjectParams.IsValidObjectQuery(Channel))
			{
				ObjectParams.AddObjectTypesToQuery(Channel);
			}
		}

		if (!bAllowMultipleHitsPerSwing)
		{
			FKDHitActors* HitResAct = AlreadyHitActors.Find(CurrentTrace.Key);
			if (HitResAct && HitResAct->AlreadyHitActors.Num() > 0)
			{
				Params.AddIgnoredActors(HitResAct->AlreadyHitActors);
			}
		}

		if (!ObjectParams.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("KDCollisionComponent - Invalid Collision Channel"));
			return;
		}

		UWorld* World = GetWorld();
		if (World)
		{
			FHitResult HitRes;
			bool bHit = World->SweepSingleByObjectType(
				HitRes, StartPos, EndPos, Orientation.Quaternion(), ObjectParams,
				FCollisionShape::MakeSphere(CurrentTrace.Value.Radius), Params);

			// Cross-frame accuracy: sweep from start to previous end position
			if (!bHit && CurrentTrace.Value.bCrossframeAccuracy && !CurrentTrace.Value.bIsFirstFrame)
			{
				const FRotator OldOrient = GetLineRotation(StartPos, CurrentTrace.Value.OldEndSocketPos);
				bHit = World->SweepSingleByObjectType(
					HitRes, StartPos, CurrentTrace.Value.OldEndSocketPos, OldOrient.Quaternion(),
					ObjectParams, FCollisionShape::MakeSphere(CurrentTrace.Value.Radius), Params);
			}

			if (bHit)
			{
				OnCollisionDetected.Broadcast(HitRes);

				if (!bAllowMultipleHitsPerSwing)
				{
					FKDHitActors* HitResAct = AlreadyHitActors.Find(CurrentTrace.Key);
					if (HitResAct)
					{
						HitResAct->AlreadyHitActors.Add(HitRes.GetActor());
					}
					else
					{
						FKDHitActors NewHit;
						NewHit.AlreadyHitActors.Add(HitRes.GetActor());
						AlreadyHitActors.Add(CurrentTrace.Key, NewHit);
					}
				}

				ApplyDamage(HitRes, CurrentTrace.Value);
			}

			CurrentTrace.Value.bIsFirstFrame = false;
			CurrentTrace.Value.OldEndSocketPos = EndPos;
		}
	}
}

void UKDCollisionComponent::SetStarted(bool InStarted)
{
	bIsStarted = InStarted;
	SetComponentTickEnabled(bIsStarted || ShowDebugInfo == EKDDDebugType::Always);
}

FRotator UKDCollisionComponent::GetLineRotation(FVector Start, FVector End) const
{
	const FVector Diff = End - Start;
	return Diff.Rotation();
}

void UKDCollisionComponent::DisplayDebugTraces()
{
	TMap<FName, FKDTraceInfo> DebugTraces;
	FLinearColor DebugColor;

	switch (ShowDebugInfo)
	{
	case EKDDDebugType::Always:
		DebugTraces = DamageTraces;
		DebugColor = bIsStarted ? DebugActiveColor : DebugInactiveColor;
		break;
	case EKDDDebugType::DuringSwing:
		if (bIsStarted)
		{
			DebugTraces = ActivatedTraces;
			DebugColor = DebugActiveColor;
		}
		else
		{
			return;
		}
		break;
	case EKDDDebugType::None:
	default:
		return;
	}

	for (const TPair<FName, FKDTraceInfo>& Box : DebugTraces)
	{
		if (DamageMesh && DamageMesh->DoesSocketExist(Box.Value.StartSocket) && DamageMesh->DoesSocketExist(Box.Value.EndSocket))
		{
			const FVector StartPos = DamageMesh->GetSocketLocation(Box.Value.StartSocket);
			const FVector EndPos = DamageMesh->GetSocketLocation(Box.Value.EndSocket);
			ShowDebugTrace(StartPos, EndPos, Box.Value.Radius, 2.0f, DebugColor);
		}
	}
}

void UKDCollisionComponent::ShowDebugTrace(const FVector& StartPos, const FVector& EndPos,
	float Radius, float Duration, FLinearColor Color)
{
	UWorld* World = GetWorld();
	if (World)
	{
		UKismetSystemLibrary::DrawDebugCylinder(this, StartPos, EndPos, Radius, 12, Color, Duration);
	}
}

void UKDCollisionComponent::ApplyDamage(const FHitResult& HitResult, const FKDBaseTraceInfo& CurrentTrace)
{
	if (IgnoredActors.Contains(HitResult.GetActor()))
	{
		return;
	}

	const AActor* Causer = GetActorOwner();
	const AActor* Victim = HitResult.GetActor();

	if (!bAutoApplyDamage || !CanActorDamageActor(Causer, Victim))
	{
		return;
	}

	switch (CurrentTrace.DamageType)
	{
	case EKDDamageType::Point:
		ApplyPointDamage(HitResult, CurrentTrace);
		break;
	case EKDDamageType::Area:
		ApplyAreaDamage(HitResult, CurrentTrace);
		break;
	default:
		ApplyPointDamage(HitResult, CurrentTrace);
		break;
	}
}

bool UKDCollisionComponent::CanActorDamageActor(const AActor* Attacker, const AActor* Victim) const
{
	if (!Attacker || !Victim)
	{
		return false;
	}

	// No self-damage
	if (Attacker == Victim)
	{
		return false;
	}

	// Simple team-based check using IKDEntityInterface
	const bool bAttackerImplements = Attacker->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass());
	const bool bVictimImplements = Victim->GetClass()->ImplementsInterface(UKDEntityInterface::StaticClass());

	if (bAttackerImplements && bVictimImplements)
	{
		const FGameplayTag AttackerTeam = IKDEntityInterface::Execute_GetEntityCombatTeam(Attacker);
		const FGameplayTag VictimTeam = IKDEntityInterface::Execute_GetEntityCombatTeam(Victim);

		// Same team = no damage
		if (AttackerTeam.IsValid() && VictimTeam.IsValid() && AttackerTeam == VictimTeam)
		{
			return false;
		}
	}

	return true;
}

void UKDCollisionComponent::ApplyPointDamage(const FHitResult& HitResult, const FKDBaseTraceInfo& CurrentTrace)
{
	if (!IsValid(HitResult.GetActor()))
	{
		return;
	}

	const FVector DamagerRelativePos = GetOwner()->GetActorLocation() - HitResult.GetActor()->GetActorLocation();
	const float Damage = CurrentTrace.BaseDamage;

	UE_LOG(LogTemp, VeryVerbose, TEXT("KDCombat: Damage %.1f applied to %s"),
		Damage, *HitResult.GetActor()->GetName());

	FPointDamageEvent DamageInfo;
	DamageInfo.DamageTypeClass = CurrentTrace.DamageTypeClass;
	DamageInfo.Damage = Damage;
	DamageInfo.HitInfo = HitResult;
	DamageInfo.ShotDirection = DamagerRelativePos;

	HitResult.GetActor()->TakeDamage(Damage, DamageInfo,
		GetActorOwner()->GetInstigatorController(), GetActorOwner());

	OnActorDamaged.Broadcast(HitResult.GetActor());
}

void UKDCollisionComponent::ApplyAreaDamage(const FHitResult& HitResult, const FKDBaseTraceInfo& CurrentTrace)
{
	if (!IsValid(HitResult.GetActor()))
	{
		return;
	}

	const float Damage = CurrentTrace.BaseDamage;
	FRadialDamageEvent DamageInfo;
	DamageInfo.DamageTypeClass = CurrentTrace.DamageTypeClass;
	DamageInfo.Params.BaseDamage = Damage;
	DamageInfo.ComponentHits.Add(HitResult);
	DamageInfo.Origin = HitResult.ImpactPoint;

	HitResult.GetActor()->TakeDamage(Damage, DamageInfo,
		GetActorOwner()->GetInstigatorController(), GetActorOwner());

	OnActorDamaged.Broadcast(HitResult.GetActor());
}

void UKDCollisionComponent::HandleTimedSingleTraceFinished(const FName& TraceEnded)
{
	if (IsValid(GetOwner()))
	{
		UWorld* World = GetWorld();
		if (World && TraceTimers.Contains(TraceEnded))
		{
			StopSingleTrace(TraceEnded);
			FTimerHandle* Handle = TraceTimers.Find(TraceEnded);
			World->GetTimerManager().ClearTimer(*Handle);
		}
	}
}

void UKDCollisionComponent::HandleAllTimedTraceFinished()
{
	StopAllTraces();
	if (GetOwner())
	{
		UWorld* World = GetWorld();
		if (World && bAllTimedTraceStarted)
		{
			World->GetTimerManager().ClearTimer(AllTraceTimer);
			bAllTimedTraceStarted = false;
		}
	}
}

void UKDCollisionComponent::HandleAreaDamageFinished()
{
	StopCurrentAreaDamage();
}

void UKDCollisionComponent::HandleAreaDamageLooping()
{
	PerformAreaDamage_Single(CurrentAreaDamage.Location, CurrentAreaDamage.Radius);
}

void UKDCollisionComponent::PlayTrails(const FName& Trail)
{
	if (!DamageTraces.Contains(Trail) || !DamageMesh)
	{
		return;
	}
	const FKDTraceInfo& TraceInfo = *DamageTraces.Find(Trail);

	if (TraceInfo.AttackSound && DamageMesh->DoesSocketExist(TraceInfo.StartSocket))
	{
		UGameplayStatics::SpawnSoundAttached(TraceInfo.AttackSound, DamageMesh, TraceInfo.StartSocket);
	}

	if (TraceInfo.NiagaraTrail)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TraceInfo.NiagaraTrail, DamageMesh, TraceInfo.StartSocket,
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false, true);
		NiagaraSystemComponents.Add(Trail, NiagaraComp);
	}
}

void UKDCollisionComponent::StopTrails(const FName& Trail)
{
	if (NiagaraSystemComponents.Contains(Trail))
	{
		UNiagaraComponent* NiagaraComp = *NiagaraSystemComponents.Find(Trail);
		if (NiagaraComp)
		{
			NiagaraComp->DeactivateImmediate();
			NiagaraComp->DestroyComponent();
		}
		NiagaraSystemComponents.Remove(Trail);
	}
}
