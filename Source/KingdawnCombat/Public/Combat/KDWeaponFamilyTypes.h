// Copyright Kingdawn. All Rights Reserved.
// Universal weapon-family attachment profiles.
// Each weapon family (Staff, OneHandedSword, Shield, etc.) gets a single
// attachment profile that defines how it mounts to any character skeleton.
// This avoids per-weapon-mesh hand-tuning and scales by family, not per item.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "KDWeaponFamilyTypes.generated.h"

/**
 * Weapon family enum.
 * Groups weapons by how they attach to a character, not by gameplay balance.
 * Example: all one-handed swords share the same grip socket, rotation, and offset.
 */
UENUM(BlueprintType)
enum class EKDWeaponFamily : uint8
{
	None UMETA(DisplayName = "None"),
	Staff UMETA(DisplayName = "Staff"),
	OneHandedSword UMETA(DisplayName = "One-Handed Sword"),
	Shield UMETA(DisplayName = "Shield"),
	TwoHandedSword UMETA(DisplayName = "Two-Handed Sword"),
	Bow UMETA(DisplayName = "Bow"),
	Dagger UMETA(DisplayName = "Dagger"),
	Axe UMETA(DisplayName = "Axe"),
	Hammer UMETA(DisplayName = "Hammer"),
	Wand UMETA(DisplayName = "Wand"),
};

/**
 * Attachment profile for a weapon family.
 * Defines the socket, local transform, and optional reference-mesh fallback
 * for attaching any weapon of this family to a character skeleton.
 *
 * The reference-mesh fallback works like this:
 *   1. Try to attach to SocketName on the runtime character mesh.
 *   2. If the runtime mesh has no such socket, load ReferenceMeshPath
 *      and copy the socket transform from ReferenceSocketName on that mesh.
 *   3. Fall back to BoneName (raw bone) if everything else fails.
 */
USTRUCT(BlueprintType)
struct KINGDAWNCOMBAT_API FKDWeaponAttachProfile
{
	GENERATED_BODY()

	/** Bone on the character skeleton to parent the weapon to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FName BoneName;

	/** Preferred socket name on the character mesh (authored on the skeleton). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FName SocketName;

	/** Family-level local offset from the socket/bone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FVector RelativeOffset;

	/** Family-level local rotation from the socket/bone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FRotator RelativeRotation;

	/** Family-level local scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FVector RelativeScale;

	/**
	 * Optional reference skeletal mesh that has the authoritative socket.
	 * Used when the runtime character mesh doesn't have SocketName.
	 * e.g. /Game/Witch_Animation_Set/Animation/Skeleton/SK_Mannequin.SK_Mannequin
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment|Fallback")
	FSoftObjectPath ReferenceMeshPath;

	/** Socket name on the reference mesh to copy transforms from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment|Fallback")
	FName ReferenceSocketName;

	FKDWeaponAttachProfile()
		: BoneName(NAME_None)
		, SocketName(NAME_None)
		, RelativeOffset(FVector::ZeroVector)
		, RelativeRotation(FRotator::ZeroRotator)
		, RelativeScale(FVector::OneVector)
		, ReferenceSocketName(NAME_None)
	{
	}

	FKDWeaponAttachProfile(
		FName InBone, FName InSocket,
		const FVector& InOffset, const FRotator& InRotation, const FVector& InScale,
		const FSoftObjectPath& InRefMesh = FSoftObjectPath(), FName InRefSocket = NAME_None)
		: BoneName(InBone)
		, SocketName(InSocket)
		, RelativeOffset(InOffset)
		, RelativeRotation(InRotation)
		, RelativeScale(InScale)
		, ReferenceMeshPath(InRefMesh)
		, ReferenceSocketName(InRefSocket)
	{
	}
};
