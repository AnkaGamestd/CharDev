// Copyright Kingdawn. All Rights Reserved.
// Adapted from CollisionsManager/ACMTypes.h - stripped ACF dependencies

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayEffectTypes.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include <GameFramework/DamageType.h>
#include "KDCollisionTypes.generated.h"

class UParticleSystemComponent;
class UAudioComponent;
class UGameplayEffect;

/** Debug trace display mode. */
UENUM(BlueprintType)
enum class EKDDamageType : uint8
{
	Point UMETA(DisplayName = "Point Damage"),
	Area UMETA(DisplayName = "Area Damage"),
};

/** Debug trace display mode. */
UENUM(BlueprintType)
enum class EKDDDebugType : uint8
{
	None = 0 UMETA(DisplayName = "Don't Show Debug Info"),
	DuringSwing = 1 UMETA(DisplayName = "Show Info During Swing"),
	Always = 2 UMETA(DisplayName = "Always Show Debug Info"),
};

/** FX location spawn mode. */
UENUM(BlueprintType)
enum class EKDSpawnFXLocation : uint8
{
	OnActorLocation UMETA(DisplayName = "Attached to Actor"),
	OnSocketOrBone UMETA(DisplayName = "Attached to Socket / Bone"),
	OnProvidedTransform UMETA(DisplayName = "Spawn On Provided Transform")
};

/** Base FX struct for sounds and particles. */
USTRUCT(BlueprintType)
struct FKDBaseFX
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn")
	TObjectPtr<UNiagaraSystem> NiagaraParticle;

	FKDBaseFX()
		: Sound(nullptr)
		, NiagaraParticle(nullptr)
	{}
};

/** Tracks actors already hit by a single trace to prevent multi-hit. */
USTRUCT(BlueprintType)
struct FKDHitActors
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Kingdawn")
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
};

/** Area damage configuration. */
USTRUCT(BlueprintType)
struct FKDAreaDamageInfo
{
	GENERATED_BODY()

public:
	FKDAreaDamageInfo()
		: Radius(0.f)
		, Location(FVector::ZeroVector)
		, bIsActive(false)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	float Radius;

	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	FVector Location;

	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn")
	bool bIsActive;

	UPROPERTY()
	FTimerHandle AreaLoopTimer;
};

/** Base trace info shared by all trace types. */
USTRUCT(BlueprintType)
struct FKDBaseTraceInfo
{
	GENERATED_BODY()

public:
	FKDBaseTraceInfo()
		: DamageTypeClass(UDamageType::StaticClass())
		, BaseDamage(0.f)
		, DamageType(EKDDamageType::Point)
	{}

	/** Damage type class applied on hit. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kingdawn")
	TSubclassOf<UDamageType> DamageTypeClass;

	/** Base damage applied on hit. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kingdawn")
	float BaseDamage;

	/** Point vs Area damage. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Kingdawn")
	EKDDamageType DamageType;

	/** GameplayEffect applied on hit (optional). */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Kingdawn")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;
};

/** Full trace configuration for weapon collision sweeps. */
USTRUCT(BlueprintType)
struct FKDTraceInfo : public FKDBaseTraceInfo
{
	GENERATED_BODY()

public:
	FKDTraceInfo()
		: Radius(10.f)
		, TrailLength(1.f)
		, AttackSound(nullptr)
		, NiagaraTrail(nullptr)
		, StartSocket(TEXT("start"))
		, EndSocket(TEXT("end"))
		, bCrossframeAccuracy(true)
		, bIsFirstFrame(true)
		, OldEndSocketPos(FVector::ZeroVector)
	{}

	/** Sphere sweep radius. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kingdawn")
	float Radius;

	/** Trail effect length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn")
	float TrailLength;

	/** Sound played when trace activates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn")
	TObjectPtr<USoundBase> AttackSound;

	/** Niagara trail effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn")
	TObjectPtr<UNiagaraSystem> NiagaraTrail;

	/** Start socket name on the damage mesh. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kingdawn")
	FName StartSocket;

	/** End socket name on the damage mesh. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kingdawn")
	FName EndSocket;

	/** Whether to sweep between previous and current frame positions. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kingdawn")
	bool bCrossframeAccuracy;

	// --- Transient (not serialized) ---
	bool bIsFirstFrame;
	FVector OldEndSocketPos;
};
