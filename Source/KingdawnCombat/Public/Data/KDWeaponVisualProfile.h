// Copyright Kingdawn. All Rights Reserved.
// Data asset describing how a weapon family looks and attaches at runtime.
// One profile per weapon family; referenced by UKDBuildDefinition.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Combat/KDWeaponFamilyTypes.h"
#include "KDWeaponVisualProfile.generated.h"

class USkeletalMesh;

/**
 * Visual and attachment profile for a single weapon family.
 *
 * Designers author one UKDWeaponVisualProfile per weapon family (Staff,
 * OneHandedSword, Shield, etc.).  At runtime the combat system reads the
 * profile to decide which mesh to spawn, which bone/socket to parent it to,
 * and what local transform to apply.
 *
 * The reference-socket fallback chain mirrors FKDWeaponAttachProfile:
 *   1. Attach to SocketName on the runtime character mesh.
 *   2. If the runtime mesh lacks that socket and bUseReferenceSocketFallback
 *      is true, load ReferenceAsset and copy the socket transform from
 *      ReferenceSocketName on that mesh.
 *   3. Fall back to AttachBoneName (raw bone) if everything else fails.
 */
UCLASS(BlueprintType)
class KINGDAWNCOMBAT_API UKDWeaponVisualProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UKDWeaponVisualProfile();

	// --- Identity ---

	/** Weapon family this profile describes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Identity")
	EKDWeaponFamily WeaponFamily;

	// --- Attachment ---

	/** Preferred socket name on the character mesh (authored on the skeleton). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Attachment")
	FName SocketName;

	/** Bone on the character skeleton to parent the weapon to when no socket is found. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Attachment")
	FName AttachBoneName;

	/** Family-level local offset from the socket/bone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Attachment")
	FVector RelativeOffset;

	/** Family-level local rotation from the socket/bone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Attachment")
	FRotator RelativeRotation;

	/** Family-level local scale. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Attachment")
	FVector RelativeScale;

	// --- Mesh ---

	/** The skeletal mesh asset to spawn for this weapon family. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Mesh")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	// --- Reference Socket Fallback ---

	/**
	 * If true, attempt to resolve the socket transform from ReferenceAsset
	 * when the runtime character mesh lacks SocketName.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Fallback")
	bool bUseReferenceSocketFallback;

	/**
	 * Reference skeletal mesh that carries the authoritative socket layout.
	 * Used when the runtime character mesh does not contain SocketName.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Fallback",
		meta = (EditCondition = "bUseReferenceSocketFallback"))
	TSoftObjectPtr<USkeletalMesh> ReferenceAsset;

	/** Socket name on ReferenceAsset to copy transforms from. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kingdawn|Fallback",
		meta = (EditCondition = "bUseReferenceSocketFallback"))
	FName ReferenceSocketName;
};
