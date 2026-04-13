// Copyright Kingdawn. All Rights Reserved.
// Single runtime applier for character appearance.
// Per Arch.md: owns appearance application only.
// Must not: decide mastery/build, save/load profiles, manage save slots, derive combat tags.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCharacterAppearanceComponent.generated.h"

class UTainlordCharacterCustomizationCatalog;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

/**
 * Single runtime applier for character appearance.
 * Creation preview and runtime spawn use the same ApplyAppearance() entry point.
 *
 * Per Arch.md architectural boundary:
 * - Appearance touches visuals only
 * - Build/mastery touches gameplay only
 * - This component must never derive or modify combat state
 *
 * Profile context (Gender/Race) is validated on every apply call.
 * Entries whose filters don't match the active context are rejected.
 */
UCLASS(ClassGroup=(Customization), meta=(BlueprintSpawnableComponent))
class TAINLORD_API UTainlordCharacterAppearanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTainlordCharacterAppearanceComponent();

	// --- Catalog reference ---

	/** The catalog to resolve IDs against. Must be set before applying appearance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<UTainlordCharacterCustomizationCatalog> Catalog;

	// --- Mesh component references (set by owning character or Blueprint) ---

	/** Skeletal mesh component used for the head. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<USkeletalMeshComponent> HeadMeshComponent;

	/** Skeletal mesh component used for the body. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<USkeletalMeshComponent> BodyMeshComponent;

	/** Skeletal mesh component used for hair. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<USkeletalMeshComponent> HairMeshComponent;

	/** Skeletal mesh component used for beard. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<USkeletalMeshComponent> BeardMeshComponent;

	/** Skeletal mesh component used for modular arms. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<USkeletalMeshComponent> ArmsMeshComponent;

	/** Skeletal mesh component used for modular legs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<USkeletalMeshComponent> LegsMeshComponent;

	/** Skeletal mesh component for shoulders accessory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes|Accessories")
	TObjectPtr<USkeletalMeshComponent> ShouldersMeshComponent;

	/** Skeletal mesh component for left bracer accessory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes|Accessories")
	TObjectPtr<USkeletalMeshComponent> LeftBracerMeshComponent;

	/** Skeletal mesh component for right bracer accessory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes|Accessories")
	TObjectPtr<USkeletalMeshComponent> RightBracerMeshComponent;

	/**
	 * Static mesh component for the left eye.
	 * After head mesh changes, this will be attached to the "Eye_L" socket.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<UStaticMeshComponent> LeftEyeMeshComponent;

	/**
	 * Static mesh component for the right eye.
	 * After head mesh changes, this will be attached to the "Eye_R" socket.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Meshes")
	TObjectPtr<UStaticMeshComponent> RightEyeMeshComponent;

	// --- Profile context ---

	/**
	 * Set the active profile context (gender and race).
	 * This context is used to validate that catalog entries are allowed
	 * for the current character before applying them.
	 * Must be called before ApplyAppearance or individual apply functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	void SetProfileContext(ECharacterGender Gender, ECharacterRace Race);

	/** Get the active gender context. */
	UFUNCTION(BlueprintPure, Category = "Customization|Appearance")
	ECharacterGender GetActiveGender() const { return ActiveGender; }

	/** Get the active race context. */
	UFUNCTION(BlueprintPure, Category = "Customization|Appearance")
	ECharacterRace GetActiveRace() const { return ActiveRace; }

	// --- Apply functions ---

	/**
	 * Apply a complete appearance data payload with profile context validation.
	 * This is the shared entry point for both creation preview and runtime spawn.
	 * Per Arch.md: There must not be a separate preview-only appearance system.
	 *
	 * Each entry is validated against the active gender/race context.
	 * Entries that don't match the context are rejected with a warning log.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	void ApplyAppearance(const FTainlordAppearanceData& AppearanceData);

	/**
	 * Apply a complete appearance with explicit profile context.
	 * Convenience overload that sets context and applies in one call.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	void ApplyAppearanceWithContext(const FTainlordAppearanceData& AppearanceData, ECharacterGender Gender, ECharacterRace Race);

	/** Apply head mesh from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyHead(FName HeadId);

	/** Apply hair mesh from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyHair(FName HairId);

	/** Apply beard mesh from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyBeard(FName BeardId);

	/** Apply modular arms mesh from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyArms(FName ArmsId);

	/** Apply modular legs mesh from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyLegs(FName LegsId);

	/** Apply skin tone from a stable ID. Skin tones are not gender/race gated. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplySkinTone(FName SkinToneId);

	/** Apply shoulders accessory from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyShoulders(FName ShouldersId);

	/** Apply left bracer accessory from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyLeftBracer(FName LeftBracerId);

	/** Apply right bracer accessory from a stable ID. Validates against active profile context. */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	bool ApplyRightBracer(FName RightBracerId);

	/** Clear all appearance overrides (reset to default/empty meshes). */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	void ClearAppearance();

	/**
	 * Re-attach eye components to sockets on the current head mesh.
	 * Looks for "Eye_L" and "Eye_R" sockets on HeadMeshComponent.
	 * If a socket is missing, logs a warning but does not crash.
	 * Called automatically after ApplyHead(). Can also be called manually.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Appearance")
	void AttachEyesToHeadSockets();

	// --- Query ---

	/** Get the last applied appearance data. */
	UFUNCTION(BlueprintPure, Category = "Customization|Appearance")
	const FTainlordAppearanceData& GetCurrentAppearance() const { return CurrentAppearance; }

	/** Get the loaded catalog (loads synchronously if needed). */
	UFUNCTION(BlueprintPure, Category = "Customization|Appearance")
	UTainlordCharacterCustomizationCatalog* GetLoadedCatalog() const;

protected:
	virtual void BeginPlay() override;

private:
	/** Cache of the last applied appearance data. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Customization|Appearance", meta = (AllowPrivateAccess = "true"))
	FTainlordAppearanceData CurrentAppearance;

	/** Default meshes captured from the owning actor/Blueprint at BeginPlay. */
	UPROPERTY(Transient)
	TObjectPtr<USkeletalMesh> DefaultHeadMeshAsset;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMesh> DefaultArmsMeshAsset;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMesh> DefaultLegsMeshAsset;

	/** Active gender context for validation. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Customization|Appearance", meta = (AllowPrivateAccess = "true"))
	ECharacterGender ActiveGender = ECharacterGender::Male;

	/** Active race context for validation. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Customization|Appearance", meta = (AllowPrivateAccess = "true"))
	ECharacterRace ActiveRace = ECharacterRace::Human;

	/** Cached dynamic material instances for skin tone application. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> SkinToneMIDs;

	/** The parameter name used when skin tone was last applied (for symmetric reset). */
	UPROPERTY(Transient)
	FName LastAppliedSkinToneParamName;

	/** Shared eye mesh fallback resolved at runtime so eye rendering does not depend on BP defaults. */
	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> ResolvedEyeStaticMeshAsset;

	/** Shared eye material fallback resolved at runtime so eye rendering does not depend on BP defaults. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> ResolvedEyeMaterialAsset;

	/** Blueprint/editor authored eye offsets captured at runtime and re-applied after socket attach. */
	UPROPERTY(Transient)
	FVector DefaultLeftEyeRelativeLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector DefaultRightEyeRelativeLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator DefaultLeftEyeRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FRotator DefaultRightEyeRelativeRotation = FRotator::ZeroRotator;

	/** The currently resolved head entry; used for per-head eye calibration after socket snap. */
	UPROPERTY(Transient)
	FTainlordHeadEntry CurrentHeadEntry;

	/** Timer handle for deferred eye attachment (bone transforms need a frame to initialize). */
	FTimerHandle EyeAttachTimerHandle;

	/** Pending flag to indicate eye attachment is deferred. */
	bool bPendingEyeAttach = false;

	/** Load the catalog if not already loaded. Returns null on failure. */
	UTainlordCharacterCustomizationCatalog* EnsureCatalogLoaded() const;

public:
	/**
	 * Re-establish leader-pose relationships for all follower mesh components.
	 * Called in BeginPlay() and after any mesh swap to prevent leader pose
	 * from being lost when SetSkeletalMesh() invalidates the bind.
	 * Also called after animation mode changes on the body mesh.
	 */
	void RebindLeaderPose();

private:

	/** Create or update dynamic material instances on the target mesh for skin tone. */
	void ApplySkinToneToMesh(USkeletalMeshComponent* MeshComp, const FTainlordSkinToneEntry& Entry);

	/** Reset skin tone on a mesh using the tracked parameter name. */
	void ResetSkinToneOnMesh(USkeletalMeshComponent* MeshComp) const;

	/** Collect all material instances that should receive skin tone. */
	void CollectSkinToneMaterials();

	/** Ensure eye components have a valid mesh/material and preview-safe render settings. */
	void EnsureEyeRenderAssets();
};
