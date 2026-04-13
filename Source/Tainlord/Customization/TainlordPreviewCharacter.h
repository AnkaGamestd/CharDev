// Copyright Kingdawn. All Rights Reserved.
// Preview character for character creation scene.
// Has TainlordCharacterAppearanceComponent and modular mesh components built-in.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TainlordPreviewCharacter.generated.h"

class UTainlordCharacterAppearanceComponent;
class UTainlordCharacterCustomizationCatalog;
class USkeletalMeshComponent;
class UStaticMeshComponent;

/**
 * Preview character for character creation.
 *
 * Modular mesh component hierarchy (all created in constructor):
 *   GetMesh()      → BodyMeshComponent (the ACharacter default skeletal mesh)
 *   HeadMesh       → attached to GetMesh(), follows body skeleton
 *     ├─ HairMesh    → attached to HeadMesh, follows head skeleton
 *     ├─ BeardMesh   → attached to HeadMesh, follows head skeleton
 *     ├─ LeftEyeMesh → attached to HeadMesh at "Eye_L" socket
 *     └─ RightEyeMesh→ attached to HeadMesh at "Eye_R" socket
 *   ArmsMesh       → attached to GetMesh(), follows body skeleton
 *   LegsMesh       → attached to GetMesh(), follows body skeleton
 *
 * Eye system:
 *   Eyes are placed via sockets on the head skeletal mesh. Each head mesh
 *   must define "Eye_L" and "Eye_R" sockets at the correct eye positions.
 *   The eye components read their positions from these sockets automatically
 *   whenever the head mesh changes. No manual offsets or catalog fields needed.
 *
 * AppearanceComponent is auto-wired to these mesh components.
 * CatalogAsset is copied to AppearanceComponent on BeginPlay.
 *
 * Usage:
 * 1. Reparent BP_CreationPreviewCharacter to this class
 * 2. Set CatalogAsset to DA_CharacterCustomizationCatalog
 * 3. Place in level — ready for preview
 */
UCLASS(Blueprintable)
class TAINLORD_API ATainlordPreviewCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATainlordPreviewCharacter();

	// --- Components ---

	/** The appearance component — auto-created, auto-wired to mesh components. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
	TObjectPtr<UTainlordCharacterAppearanceComponent> AppearanceComponent;

	/** Separate skeletal mesh for the head. Attached to body mesh (leader pose). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance|Meshes")
	TObjectPtr<USkeletalMeshComponent> HeadMesh;

	/** Separate skeletal mesh for modular arms. Attached to body mesh (leader pose). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance|Meshes")
	TObjectPtr<USkeletalMeshComponent> ArmsMesh;

	/** Separate skeletal mesh for modular legs. Attached to body mesh (leader pose). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance|Meshes")
	TObjectPtr<USkeletalMeshComponent> LegsMesh;

	/** Separate skeletal mesh for hair. Attached to head mesh (leader pose). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance|Meshes")
	TObjectPtr<USkeletalMeshComponent> HairMesh;

	/** Separate skeletal mesh for beard. Attached to head mesh (leader pose). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance|Meshes")
	TObjectPtr<USkeletalMeshComponent> BeardMesh;

	/**
	 * Static mesh for the left eye. Attached to HeadMesh at "Eye_L" socket.
	 * Position is driven entirely by the socket on the active head mesh.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance|Meshes")
	TObjectPtr<UStaticMeshComponent> LeftEyeMesh;

	/**
	 * Static mesh for the right eye. Attached to HeadMesh at "Eye_R" socket.
	 * Position is driven entirely by the socket on the active head mesh.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance|Meshes")
	TObjectPtr<UStaticMeshComponent> RightEyeMesh;

	// --- Eye mesh asset ---

	/**
	 * The shared static mesh asset used for both eyes.
	 * Set this in Blueprint defaults. Both LeftEyeMesh and RightEyeMesh use this asset.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Eyes")
	TObjectPtr<UStaticMesh> EyeStaticMeshAsset;

	/**
	 * Optional material override for eye meshes.
	 * If set, both eye components will use this material at slot 0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Eyes")
	TObjectPtr<UMaterialInterface> EyeMaterialOverride;

	// --- Configuration ---

	/** Catalog asset reference. Set this in Blueprint defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TSoftObjectPtr<UTainlordCharacterCustomizationCatalog> CatalogAsset;

protected:
	virtual void BeginPlay() override;
};
