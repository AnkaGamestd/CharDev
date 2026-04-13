// Copyright Kingdawn. All Rights Reserved.
// Character customization data structures per Arch.md "Character Customization Plan".
// All IDs are FName for the first production pass (stable, not index-based).

#pragma once

#include "CoreMinimal.h"
#include "TainlordCharacterCustomizationTypes.generated.h"

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------

/**
 * Character gender. Initial shipping scope uses Male; system must be ready for Female.
 * Any = available to all genders (use for shared assets).
 */
UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
	Any UMETA(DisplayName = "Any"),
	Male UMETA(DisplayName = "Male"),
	Female UMETA(DisplayName = "Female")
};

/**
 * Character race. First-class profile field; filtering is race-aware.
 * Any = available to all races (use for shared assets).
 */
UENUM(BlueprintType)
enum class ECharacterRace : uint8
{
	Any UMETA(DisplayName = "Any"),
	Human UMETA(DisplayName = "Human"),
	Elf UMETA(DisplayName = "Elf"),
	Dwarf UMETA(DisplayName = "Dwarf"),
	Orc UMETA(DisplayName = "Orc")
};

// ---------------------------------------------------------------------------
// Catalog entry structs
// ---------------------------------------------------------------------------

/** A single head option in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordHeadEntry
{
	GENERATED_BODY()

	/** Stable ID for this head option. Never use array index as identity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	/** Skeletal mesh to apply for this head. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	/** Gender filter. Use Any for assets available to all genders. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterGender GenderFilter = ECharacterGender::Any;

	/** Race filter. Use Any for assets available to all races. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterRace RaceFilter = ECharacterRace::Any;

	/**
	 * Relative location offset applied to the head mesh component after attachment.
	 * Use this to fix vertical seams between head and body meshes.
	 * Default is zero — no offset applied (backward compatible).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector HeadRelativeLocation = FVector::ZeroVector;

	/**
	 * Relative rotation offset applied to the head mesh component.
	 * Default is zero — no rotation offset (backward compatible).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FRotator HeadRelativeRotation = FRotator::ZeroRotator;

	/**
	 * Relative scale offset applied to the head mesh component.
	 * Default is (1,1,1) — no scale change (backward compatible).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector HeadRelativeScale3D = FVector::OneVector;

	/**
	 * Per-head left eye offset applied after socket snap.
	 * Needed because different head meshes do not share the same eyeball depth/pivot.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Eyes")
	FVector LeftEyeRelativeLocation = FVector::ZeroVector;

	/** Per-head right eye offset applied after socket snap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Eyes")
	FVector RightEyeRelativeLocation = FVector::ZeroVector;

	/** Optional per-head left eye rotation tweak after socket snap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Eyes")
	FRotator LeftEyeRelativeRotation = FRotator::ZeroRotator;

	/** Optional per-head right eye rotation tweak after socket snap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Eyes")
	FRotator RightEyeRelativeRotation = FRotator::ZeroRotator;

	bool IsValid() const { return !Id.IsNone() && !MeshAsset.IsNull(); }
};

/** A single hair option in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordHairEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	/** Static or skeletal mesh for this hair style. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	/** Gender filter. Use Any for assets available to all genders. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterGender GenderFilter = ECharacterGender::Any;

	/** Race filter. Use Any for assets available to all races. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterRace RaceFilter = ECharacterRace::Any;

	bool IsValid() const { return !Id.IsNone() && !MeshAsset.IsNull(); }
};

/** A single beard option in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordBeardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	/** Gender filter. Use Any for assets available to all genders. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterGender GenderFilter = ECharacterGender::Any;

	/** Race filter. Use Any for assets available to all races. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterRace RaceFilter = ECharacterRace::Any;

	/**
	 * Relative location offset applied to the beard mesh component after attachment.
	 * Use this to fix depth sorting issues where beard meshes render behind the face.
	 * Default is zero — no offset applied.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector BeardRelativeLocation = FVector::ZeroVector;

	/**
	 * Relative rotation offset applied to the beard mesh component.
	 * Default is zero — no rotation offset.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FRotator BeardRelativeRotation = FRotator::ZeroRotator;

	/**
	 * Relative scale offset applied to the beard mesh component.
	 * Default is (1,1,1) — no scale change.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector BeardRelativeScale3D = FVector::OneVector;

	bool IsValid() const { return !Id.IsNone() && !MeshAsset.IsNull(); }
};

/** A single arms mesh option in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordArmsEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterGender GenderFilter = ECharacterGender::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterRace RaceFilter = ECharacterRace::Any;

	bool IsValid() const { return !Id.IsNone() && !MeshAsset.IsNull(); }
};

/** A single legs mesh option in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordLegsEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterGender GenderFilter = ECharacterGender::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterRace RaceFilter = ECharacterRace::Any;

	bool IsValid() const { return !Id.IsNone() && !MeshAsset.IsNull(); }
};

// ---------------------------------------------------------------------------
// Armor / accessory catalog entries
// ---------------------------------------------------------------------------

/** A single shoulders accessory option in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordShouldersEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterGender GenderFilter = ECharacterGender::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterRace RaceFilter = ECharacterRace::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector RelativeScale3D = FVector::OneVector;

	bool IsValid() const { return !Id.IsNone() && !MeshAsset.IsNull(); }
};

/** A single bracer accessory option (left or right) in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordBracerEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TSoftObjectPtr<USkeletalMesh> MeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterGender GenderFilter = ECharacterGender::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	ECharacterRace RaceFilter = ECharacterRace::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization|Transform")
	FVector RelativeScale3D = FVector::OneVector;

	bool IsValid() const { return !Id.IsNone() && !MeshAsset.IsNull(); }
};

/** A single skin tone option in the customization catalog. */
USTRUCT(BlueprintType)
struct FTainlordSkinToneEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName Id = NAME_None;

	/** The linear color to apply as skin tone via dynamic material instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FLinearColor Color = FLinearColor::White;

	/** Material parameter name to override for skin tone. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName MaterialParameterName = FName(TEXT("SkinTone"));

	bool IsValid() const { return !Id.IsNone(); }
};

// ---------------------------------------------------------------------------
// Appearance data (player selection payload)
// ---------------------------------------------------------------------------

/**
 * Owns appearance choice payload only.
 * Contains stable IDs, not list indices.
 * Per Arch.md: BodyId is not required unless body variants are introduced later.
 */
USTRUCT(BlueprintType)
struct FTainlordAppearanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FName HeadId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FName HairId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FName BeardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FName ArmsId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FName LegsId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FName SkinToneId = NAME_None;

	// --- Armor / accessory selections ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Accessories")
	FName ShouldersId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Accessories")
	FName LeftBracerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Accessories")
	FName RightBracerId = NAME_None;

	/** Returns true if at least one field is set to a non-default value. */
	bool HasAnySelection() const
	{
		return !HeadId.IsNone() || !HairId.IsNone() || !BeardId.IsNone() ||
			!ArmsId.IsNone() || !LegsId.IsNone() || !SkinToneId.IsNone() ||
			!ShouldersId.IsNone() || !LeftBracerId.IsNone() || !RightBracerId.IsNone();
	}
};

// ---------------------------------------------------------------------------
// Creation stage enum — tracks which stages the player has completed
// ---------------------------------------------------------------------------

/**
 * Enum for creation flow stages.
 * Used by validation to enforce stage-gated requirements.
 * The creation UI advances through stages in order; validation checks
 * whether the current stage's requirements are met before allowing progress.
 */
UENUM(BlueprintType)
enum class ECreationStage : uint8
{
	/** Race & gender selection (stage 1). */
	RaceSelect UMETA(DisplayName = "Race Select"),

	/** Appearance customization (stage 2). */
	Appearance UMETA(DisplayName = "Appearance"),

	/** Mastery / class selection (stage 3). */
	Mastery UMETA(DisplayName = "Mastery"),

	/** Starting city selection (stage 4). */
	City UMETA(DisplayName = "City"),

	/** Character naming (stage 5). */
	Name UMETA(DisplayName = "Name"),

	/** Final confirmation — all stages must pass (stage 6). */
	Confirm UMETA(DisplayName = "Confirm")
};

// ---------------------------------------------------------------------------
// Validation result — structured error info for UI
// ---------------------------------------------------------------------------

/**
 * Structured validation result.
 * Contains a success flag, human-readable reason, and which stage failed.
 * UI can use FailedStage to highlight the relevant section.
 */
USTRUCT(BlueprintType)
struct FProfileValidationResult
{
	GENERATED_BODY()

	/** True if validation passed for the requested stage. */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bIsValid = true;

	/** Human-readable reason if validation failed. Empty on success. */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString Reason;

	/** Which creation stage produced the failure (only meaningful when bIsValid is false). */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	ECreationStage FailedStage = ECreationStage::RaceSelect;

	/** Helper to construct a failure result. */
	static FProfileValidationResult Fail(ECreationStage Stage, const FString& InReason)
	{
		FProfileValidationResult R;
		R.bIsValid = false;
		R.Reason = InReason;
		R.FailedStage = Stage;
		return R;
	}

	/** Helper to construct a success result. */
	static FProfileValidationResult Success()
	{
		FProfileValidationResult R;
		R.bIsValid = true;
		R.Reason.Empty();
		return R;
	}
};

// ---------------------------------------------------------------------------
// Profile data (long-lived character profile)
// ---------------------------------------------------------------------------

/**
 * Owns long-lived character profile data.
 * Per Arch.md: future-safe extension points include Level, XP, LastMap.
 *
 * Creation flow populates this incrementally:
 *   Stage 1 (RaceSelect): Gender, Race
 *   Stage 2 (Appearance): AppearanceData
 *   Stage 3 (Mastery):    SelectedMasteryId
 *   Stage 4 (City):       SelectedCityId
 *   Stage 5 (Name):       CharacterName
 *   Stage 6 (Confirm):    All fields validated, profile saved
 */
USTRUCT(BlueprintType)
struct FTainlordProfileData
{
	GENERATED_BODY()

	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	ECharacterGender Gender = ECharacterGender::Male;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	ECharacterRace Race = ECharacterRace::Human;

	// --- Appearance ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FTainlordAppearanceData AppearanceData;

	// --- Gameplay selections ---

	/**
	 * Stable ID of the selected build (class archetype).
	 * Resolved against the build catalog at runtime.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Gameplay")
	FName SelectedBuildId = NAME_None;

	/**
	 * Stable ID of the selected mastery (specialization tree / class identity).
	 * Populated during creation stage 3 (Mastery).
	 * Resolved against a mastery catalog at runtime.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Gameplay")
	FName SelectedMasteryId = NAME_None;

	/**
	 * Stable ID of the starting city.
	 * Populated during creation stage 4 (City).
	 * Determines the player's initial spawn location and faction affinity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Gameplay")
	FName SelectedCityId = NAME_None;

	// --- Future-safe extension points (add when needed) ---
	// UPROPERTY(...) int32 Level = 1;
	// UPROPERTY(...) int32 XP = 0;
	// UPROPERTY(...) FName LastMap;

	// --- Utility ---

	/** Returns true if the profile has enough data to be considered "active" (not blank). */
	bool HasAnyData() const
	{
		return !CharacterName.IsEmpty() || AppearanceData.HasAnySelection() ||
			!SelectedBuildId.IsNone() || !SelectedMasteryId.IsNone() || !SelectedCityId.IsNone();
	}
};
