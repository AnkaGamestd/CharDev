// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCharacterAppearanceComponent.h"
#include "Customization/TainlordCharacterCustomizationCatalog.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

namespace
{
	float GetForwardExtentX(const USkeletalMesh* Mesh)
	{
		if (!Mesh)
		{
			return 0.0f;
		}

		const FBoxSphereBounds Bounds = Mesh->GetBounds();
		return Bounds.Origin.X + Bounds.BoxExtent.X;
	}
}

UTainlordCharacterAppearanceComponent::UTainlordCharacterAppearanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	LastAppliedSkinToneParamName = FName(TEXT("SkinTone")); // Default fallback
}

void UTainlordCharacterAppearanceComponent::BeginPlay()
{
	Super::BeginPlay();

	// Capture the blueprint-authored modular defaults once so preview/runtime refreshes
	// can reliably restore them even if a later apply pass comes in with NAME_None.
	DefaultHeadMeshAsset = HeadMeshComponent ? HeadMeshComponent->GetSkeletalMeshAsset() : nullptr;
	DefaultArmsMeshAsset = ArmsMeshComponent ? ArmsMeshComponent->GetSkeletalMeshAsset() : nullptr;
	DefaultLegsMeshAsset = LegsMeshComponent ? LegsMeshComponent->GetSkeletalMeshAsset() : nullptr;
	DefaultLeftEyeRelativeLocation = LeftEyeMeshComponent ? LeftEyeMeshComponent->GetRelativeLocation() : FVector::ZeroVector;
	DefaultRightEyeRelativeLocation = RightEyeMeshComponent ? RightEyeMeshComponent->GetRelativeLocation() : FVector::ZeroVector;
	DefaultLeftEyeRelativeRotation = LeftEyeMeshComponent ? LeftEyeMeshComponent->GetRelativeRotation() : FRotator::ZeroRotator;
	DefaultRightEyeRelativeRotation = RightEyeMeshComponent ? RightEyeMeshComponent->GetRelativeRotation() : FRotator::ZeroRotator;

	// --- Leader pose rebind at runtime ---
	// SetLeaderPoseComponent() called in the CDO constructor may not survive
	// Blueprint reinstancing, hot-reload, or mesh assignment changes.
	// Re-establish the leader-follower relationships here to guarantee
	// that Head/Arms/Legs follow the Body skeleton at runtime.
	RebindLeaderPose();
	EnsureEyeRenderAssets();

	// --- Debug: log mesh asset names ---
	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance::BeginPlay — Default captures: "
		"Head=%s, Arms=%s, Legs=%s, Body=%s"),
		DefaultHeadMeshAsset ? *DefaultHeadMeshAsset->GetName() : TEXT("null"),
		DefaultArmsMeshAsset ? *DefaultArmsMeshAsset->GetName() : TEXT("null"),
		DefaultLegsMeshAsset ? *DefaultLegsMeshAsset->GetName() : TEXT("null"),
		(BodyMeshComponent && BodyMeshComponent->GetSkeletalMeshAsset())
			? *BodyMeshComponent->GetSkeletalMeshAsset()->GetName() : TEXT("null"));
}

UTainlordCharacterCustomizationCatalog* UTainlordCharacterAppearanceComponent::GetLoadedCatalog() const
{
	return EnsureCatalogLoaded();
}

UTainlordCharacterCustomizationCatalog* UTainlordCharacterAppearanceComponent::EnsureCatalogLoaded() const
{
	if (Catalog.IsNull())
	{
		return nullptr;
	}

	return Catalog.LoadSynchronous();
}

void UTainlordCharacterAppearanceComponent::SetProfileContext(ECharacterGender Gender, ECharacterRace Race)
{
	ActiveGender = Gender;
	ActiveRace = Race;
	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Profile context set (Gender=%d, Race=%d)"),
		static_cast<int32>(Gender), static_cast<int32>(Race));
}

void UTainlordCharacterAppearanceComponent::ApplyAppearance(const FTainlordAppearanceData& AppearanceData)
{
	CurrentAppearance = AppearanceData;

	// Apply each component with context validation. Individual functions log warnings on rejection.
	ApplyHead(AppearanceData.HeadId);
	ApplyHair(AppearanceData.HairId);
	ApplyBeard(AppearanceData.BeardId);
	ApplyArms(AppearanceData.ArmsId);
	ApplyLegs(AppearanceData.LegsId);
	ApplySkinTone(AppearanceData.SkinToneId);

	// Apply accessory slots
	ApplyShoulders(AppearanceData.ShouldersId);
	ApplyLeftBracer(AppearanceData.LeftBracerId);
	ApplyRightBracer(AppearanceData.RightBracerId);

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: ApplyAppearance complete (Head=%s, Hair=%s, Beard=%s, Arms=%s, Legs=%s, SkinTone=%s, Shoulders=%s, BracerL=%s, BracerR=%s, Context=Gender:%d Race:%d)"),
		*AppearanceData.HeadId.ToString(),
		*AppearanceData.HairId.ToString(),
		*AppearanceData.BeardId.ToString(),
		*AppearanceData.ArmsId.ToString(),
		*AppearanceData.LegsId.ToString(),
		*AppearanceData.SkinToneId.ToString(),
		*AppearanceData.ShouldersId.ToString(),
		*AppearanceData.LeftBracerId.ToString(),
		*AppearanceData.RightBracerId.ToString(),
		static_cast<int32>(ActiveGender),
		static_cast<int32>(ActiveRace));
}

void UTainlordCharacterAppearanceComponent::ApplyAppearanceWithContext(const FTainlordAppearanceData& AppearanceData, ECharacterGender Gender, ECharacterRace Race)
{
	SetProfileContext(Gender, Race);
	ApplyAppearance(AppearanceData);
}

bool UTainlordCharacterAppearanceComponent::ApplyHead(FName HeadId)
{
	CurrentAppearance.HeadId = HeadId;

	if (HeadId.IsNone())
	{
		if (HeadMeshComponent && DefaultHeadMeshAsset)
		{
			// CRITICAL FIX: Do NOT reset CurrentHeadEntry for default head.
			// The catalog entry for the default head (e.g., "Male01") must be
			// resolved so that eye calibration offsets are applied correctly.
			// If we reset to empty struct, AttachEyesToHeadSockets will use
			// zero offsets and eyes will appear at socket position only,
			// losing the editor-authored calibration.
			
			// Try to find the default head entry in catalog by mesh asset name
			UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
			if (LoadedCatalog)
			{
				const FString DefaultMeshName = DefaultHeadMeshAsset->GetName();
				for (const FTainlordHeadEntry& Entry : LoadedCatalog->Heads)
				{
					if (Entry.MeshAsset.IsNull())
					{
						continue;
					}
					
					if (USkeletalMesh* EntryMesh = Entry.MeshAsset.LoadSynchronous())
					{
						if (EntryMesh->GetName() == DefaultMeshName)
						{
							CurrentHeadEntry = Entry;
							break;
						}
					}
				}
			}
			
			// If catalog lookup failed, fall back to empty entry (uses default captured offsets)
			if (CurrentHeadEntry.Id.IsNone())
			{
				CurrentHeadEntry = FTainlordHeadEntry();
			}
			
			HeadMeshComponent->SetSkeletalMesh(DefaultHeadMeshAsset);
			HeadMeshComponent->SetUsingAbsoluteLocation(false);
			HeadMeshComponent->SetUsingAbsoluteRotation(false);
			HeadMeshComponent->SetUsingAbsoluteScale(false);
			if (BodyMeshComponent)
			{
				HeadMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			HeadMeshComponent->SetRelativeLocation(FVector::ZeroVector);
			HeadMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
			HeadMeshComponent->SetRelativeScale3D(FVector::OneVector);
			EnsureEyeRenderAssets();
			AttachEyesToHeadSockets();
			RebindLeaderPose();
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply head '%s' - no catalog loaded"), *HeadId.ToString());
		return false;
	}

	// Context-aware lookup validates gender/race filter
	const FTainlordHeadEntry* Entry = LoadedCatalog->FindHeadEntryForContext(HeadId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Head entry '%s' not found or not allowed for active context (Gender=%d, Race=%d)"),
			*HeadId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Head entry '%s' is invalid"), *HeadId.ToString());
		return false;
	}

	if (!HeadMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply head '%s' - HeadMeshComponent is null"), *HeadId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load head mesh for '%s'"), *HeadId.ToString());
		return false;
	}

	HeadMeshComponent->SetSkeletalMesh(LoadedMesh);
	CurrentHeadEntry = *Entry;
	HeadMeshComponent->SetUsingAbsoluteLocation(false);
	HeadMeshComponent->SetUsingAbsoluteRotation(false);
	HeadMeshComponent->SetUsingAbsoluteScale(false);
	if (BodyMeshComponent)
	{
		HeadMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	// Apply data-driven transform offset from catalog entry
	HeadMeshComponent->SetRelativeLocation(Entry->HeadRelativeLocation);
	HeadMeshComponent->SetRelativeRotation(Entry->HeadRelativeRotation);
	HeadMeshComponent->SetRelativeScale3D(Entry->HeadRelativeScale3D);

	// Keep all heads at roughly the same camera-facing depth as the default Male01 head.
	// Some imported heads protrude further forward even with identical catalog offsets,
	// which causes near-clip slicing in the preview scene when the camera approaches.
	if (DefaultHeadMeshAsset && LoadedMesh != DefaultHeadMeshAsset)
	{
		const float DefaultForwardX = GetForwardExtentX(DefaultHeadMeshAsset);
		const float LoadedForwardX = GetForwardExtentX(LoadedMesh);
		const float ForwardDeltaX = LoadedForwardX - DefaultForwardX;

		if (ForwardDeltaX > KINDA_SMALL_NUMBER)
		{
			FVector AdjustedLocation = HeadMeshComponent->GetRelativeLocation();
			AdjustedLocation.X -= ForwardDeltaX;
			HeadMeshComponent->SetRelativeLocation(AdjustedLocation);

			UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied automatic head depth compensation for '%s' (deltaX=%.3f)"),
				*HeadId.ToString(), ForwardDeltaX);
		}
	}

	const bool bHasOffset = !Entry->HeadRelativeLocation.IsNearlyZero() ||
	                         !Entry->HeadRelativeRotation.IsNearlyZero() ||
	                         !Entry->HeadRelativeScale3D.Equals(FVector::OneVector);

	if (bHasOffset)
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied head '%s' (mesh=%s, offset=L:%s R:%s S:%s)"),
			*HeadId.ToString(), *LoadedMesh->GetName(),
			*Entry->HeadRelativeLocation.ToString(),
			*Entry->HeadRelativeRotation.ToString(),
			*Entry->HeadRelativeScale3D.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied head '%s' (mesh=%s)"), *HeadId.ToString(), *LoadedMesh->GetName());
	}

	// Force visibility on in case the component was hidden.
	HeadMeshComponent->SetVisibility(true);

	EnsureEyeRenderAssets();
	AttachEyesToHeadSockets();
	RebindLeaderPose();

	return true;
}

bool UTainlordCharacterAppearanceComponent::ApplyHair(FName HairId)
{
	CurrentAppearance.HairId = HairId;

	if (HairId.IsNone())
	{
		if (HairMeshComponent)
		{
			HairMeshComponent->SetSkeletalMesh(nullptr);
			HairMeshComponent->SetVisibility(false);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply hair '%s' - no catalog loaded"), *HairId.ToString());
		return false;
	}

	const FTainlordHairEntry* Entry = LoadedCatalog->FindHairEntryForContext(HairId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Hair entry '%s' not found or not allowed for active context (Gender=%d, Race=%d)"),
			*HairId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Hair entry '%s' is invalid"), *HairId.ToString());
		return false;
	}

	if (!HairMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply hair '%s' - HairMeshComponent is null"), *HairId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load hair mesh for '%s'"), *HairId.ToString());
		return false;
	}

	HairMeshComponent->SetSkeletalMesh(LoadedMesh);
	HairMeshComponent->SetLeaderPoseComponent(HeadMeshComponent);
	HairMeshComponent->RefreshBoneTransforms();
	HairMeshComponent->UpdateBounds();
	HairMeshComponent->SetVisibility(true);
	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied hair '%s' (mesh=%s)"), *HairId.ToString(), *LoadedMesh->GetName());
	return true;
}

bool UTainlordCharacterAppearanceComponent::ApplyBeard(FName BeardId)
{
	CurrentAppearance.BeardId = BeardId;

	if (BeardId.IsNone())
	{
		if (BeardMeshComponent)
		{
			BeardMeshComponent->SetSkeletalMesh(nullptr);
			BeardMeshComponent->SetVisibility(false);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply beard '%s' - no catalog loaded"), *BeardId.ToString());
		return false;
	}

	const FTainlordBeardEntry* Entry = LoadedCatalog->FindBeardEntryForContext(BeardId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Beard entry '%s' not found or not allowed for active context (Gender=%d, Race=%d)"),
			*BeardId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Beard entry '%s' is invalid"), *BeardId.ToString());
		return false;
	}

	if (!BeardMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply beard '%s' - BeardMeshComponent is null"), *BeardId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load beard mesh for '%s'"), *BeardId.ToString());
		return false;
	}

	BeardMeshComponent->SetSkeletalMesh(LoadedMesh);
	
	// Apply data-driven transform offset from catalog entry to fix depth sorting issues
	// (e.g., mustache meshes rendering behind the face)
	BeardMeshComponent->SetRelativeLocation(Entry->BeardRelativeLocation);
	BeardMeshComponent->SetRelativeRotation(Entry->BeardRelativeRotation);
	BeardMeshComponent->SetRelativeScale3D(Entry->BeardRelativeScale3D);
	
	BeardMeshComponent->SetLeaderPoseComponent(HeadMeshComponent);
	BeardMeshComponent->RefreshBoneTransforms();
	BeardMeshComponent->UpdateBounds();
	BeardMeshComponent->SetVisibility(true);
	
	const bool bHasOffset = !Entry->BeardRelativeLocation.IsNearlyZero() ||
	                         !Entry->BeardRelativeRotation.IsNearlyZero() ||
	                         !Entry->BeardRelativeScale3D.Equals(FVector::OneVector);
	
	if (bHasOffset)
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied beard '%s' (mesh=%s, offset=L:%s R:%s S:%s)"),
			*BeardId.ToString(), *LoadedMesh->GetName(),
			*Entry->BeardRelativeLocation.ToString(),
			*Entry->BeardRelativeRotation.ToString(),
			*Entry->BeardRelativeScale3D.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied beard '%s' (mesh=%s)"), *BeardId.ToString(), *LoadedMesh->GetName());
	}
	return true;
}

bool UTainlordCharacterAppearanceComponent::ApplyArms(FName ArmsId)
{
	CurrentAppearance.ArmsId = ArmsId;

	if (ArmsId.IsNone())
	{
		if (ArmsMeshComponent && DefaultArmsMeshAsset)
		{
			ArmsMeshComponent->SetSkeletalMesh(DefaultArmsMeshAsset);
			ArmsMeshComponent->SetUsingAbsoluteLocation(false);
			ArmsMeshComponent->SetUsingAbsoluteRotation(false);
			ArmsMeshComponent->SetUsingAbsoluteScale(false);
			if (BodyMeshComponent)
			{
				ArmsMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			ArmsMeshComponent->SetRelativeLocation(FVector::ZeroVector);
			ArmsMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
			ArmsMeshComponent->SetRelativeScale3D(FVector::OneVector);
			// Rebind leader pose after restoring default mesh.
			RebindLeaderPose();
			ArmsMeshComponent->RefreshBoneTransforms();
			ArmsMeshComponent->UpdateBounds();
			ArmsMeshComponent->SetVisibility(true);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply arms '%s' - no catalog loaded"), *ArmsId.ToString());
		return false;
	}

	const FTainlordArmsEntry* Entry = LoadedCatalog->FindArmsEntryForContext(ArmsId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Arms entry '%s' not found or not allowed for active context (Gender=%d, Race=%d)"),
			*ArmsId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Arms entry '%s' is invalid"), *ArmsId.ToString());
		return false;
	}

	if (!ArmsMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply arms '%s' - ArmsMeshComponent is null"), *ArmsId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load arms mesh for '%s'"), *ArmsId.ToString());
		return false;
	}

	ArmsMeshComponent->SetSkeletalMesh(LoadedMesh);
	ArmsMeshComponent->SetUsingAbsoluteLocation(false);
	ArmsMeshComponent->SetUsingAbsoluteRotation(false);
	ArmsMeshComponent->SetUsingAbsoluteScale(false);
	if (BodyMeshComponent)
	{
		ArmsMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	ArmsMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	ArmsMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
	ArmsMeshComponent->SetRelativeScale3D(FVector::OneVector);
	RebindLeaderPose();
	ArmsMeshComponent->RefreshBoneTransforms();
	ArmsMeshComponent->UpdateBounds();

	// Force visibility on in case the component was hidden.
	ArmsMeshComponent->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied arms '%s' (mesh=%s)"), *ArmsId.ToString(), *LoadedMesh->GetName());
	return true;
}

bool UTainlordCharacterAppearanceComponent::ApplyLegs(FName LegsId)
{
	CurrentAppearance.LegsId = LegsId;

	if (LegsId.IsNone())
	{
		if (LegsMeshComponent && DefaultLegsMeshAsset)
		{
			LegsMeshComponent->SetSkeletalMesh(DefaultLegsMeshAsset);
			LegsMeshComponent->SetUsingAbsoluteLocation(false);
			LegsMeshComponent->SetUsingAbsoluteRotation(false);
			LegsMeshComponent->SetUsingAbsoluteScale(false);
			if (BodyMeshComponent)
			{
				LegsMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			LegsMeshComponent->SetRelativeLocation(FVector::ZeroVector);
			LegsMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
			LegsMeshComponent->SetRelativeScale3D(FVector::OneVector);
			// Rebind leader pose after restoring default mesh.
			RebindLeaderPose();
			LegsMeshComponent->RefreshBoneTransforms();
			LegsMeshComponent->UpdateBounds();
			LegsMeshComponent->SetVisibility(true);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply legs '%s' - no catalog loaded"), *LegsId.ToString());
		return false;
	}

	const FTainlordLegsEntry* Entry = LoadedCatalog->FindLegsEntryForContext(LegsId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Legs entry '%s' not found or not allowed for active context (Gender=%d, Race=%d)"),
			*LegsId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Legs entry '%s' is invalid"), *LegsId.ToString());
		return false;
	}

	if (!LegsMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply legs '%s' - LegsMeshComponent is null"), *LegsId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load legs mesh for '%s'"), *LegsId.ToString());
		return false;
	}

	LegsMeshComponent->SetSkeletalMesh(LoadedMesh);
	LegsMeshComponent->SetUsingAbsoluteLocation(false);
	LegsMeshComponent->SetUsingAbsoluteRotation(false);
	LegsMeshComponent->SetUsingAbsoluteScale(false);
	if (BodyMeshComponent)
	{
		LegsMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	LegsMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	LegsMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
	LegsMeshComponent->SetRelativeScale3D(FVector::OneVector);
	RebindLeaderPose();
	LegsMeshComponent->RefreshBoneTransforms();
	LegsMeshComponent->UpdateBounds();

	// Force visibility on in case the component was hidden.
	LegsMeshComponent->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied legs '%s' (mesh=%s)"), *LegsId.ToString(), *LoadedMesh->GetName());
	return true;
}

bool UTainlordCharacterAppearanceComponent::ApplySkinTone(FName SkinToneId)
{
	CurrentAppearance.SkinToneId = SkinToneId;

	if (SkinToneId.IsNone())
	{
		// Reset skin tone using the last known parameter name for symmetry
		if (HeadMeshComponent)
		{
			ResetSkinToneOnMesh(HeadMeshComponent);
		}
		if (BodyMeshComponent)
		{
			ResetSkinToneOnMesh(BodyMeshComponent);
		}
		if (ArmsMeshComponent)
		{
			ResetSkinToneOnMesh(ArmsMeshComponent);
		}
		if (LegsMeshComponent)
		{
			ResetSkinToneOnMesh(LegsMeshComponent);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply skin tone '%s' - no catalog loaded"), *SkinToneId.ToString());
		return false;
	}

	// Skin tones are not gender/race gated
	const FTainlordSkinToneEntry* Entry = LoadedCatalog->FindSkinToneEntryForContext(SkinToneId, ActiveGender, ActiveRace);
	if (!Entry || !Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Skin tone entry '%s' not found or invalid"), *SkinToneId.ToString());
		return false;
	}

	// Track the parameter name from this entry for symmetric reset
	LastAppliedSkinToneParamName = Entry->MaterialParameterName;

	// Collect materials if not already done
	if (SkinToneMIDs.Num() == 0)
	{
		CollectSkinToneMaterials();
	}

	// Apply skin tone directly to mesh components
	if (HeadMeshComponent)
	{
		ApplySkinToneToMesh(HeadMeshComponent, *Entry);
	}
	if (BodyMeshComponent)
	{
		ApplySkinToneToMesh(BodyMeshComponent, *Entry);
	}
	if (ArmsMeshComponent)
	{
		ApplySkinToneToMesh(ArmsMeshComponent, *Entry);
	}
	if (LegsMeshComponent)
	{
		ApplySkinToneToMesh(LegsMeshComponent, *Entry);
	}

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied skin tone '%s' (color=%s, param=%s)"),
		*SkinToneId.ToString(), *Entry->Color.ToString(), *Entry->MaterialParameterName.ToString());
	return true;
}

// ---------------------------------------------------------------------------
// Accessory apply functions
// ---------------------------------------------------------------------------

bool UTainlordCharacterAppearanceComponent::ApplyShoulders(FName ShouldersId)
{
	CurrentAppearance.ShouldersId = ShouldersId;

	if (ShouldersId.IsNone())
	{
		if (ShouldersMeshComponent)
		{
			ShouldersMeshComponent->SetSkeletalMesh(nullptr);
			ShouldersMeshComponent->SetVisibility(false);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply shoulders '%s' - no catalog loaded"), *ShouldersId.ToString());
		return false;
	}

	const FTainlordShouldersEntry* Entry = LoadedCatalog->FindShouldersEntryForContext(ShouldersId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Shoulders entry '%s' not found or not allowed for context (Gender=%d, Race=%d)"),
			*ShouldersId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Shoulders entry '%s' is invalid"), *ShouldersId.ToString());
		return false;
	}

	if (!ShouldersMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply shoulders '%s' - ShouldersMeshComponent is null"), *ShouldersId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load shoulders mesh for '%s'"), *ShouldersId.ToString());
		return false;
	}

	ShouldersMeshComponent->SetSkeletalMesh(LoadedMesh);
	ShouldersMeshComponent->SetUsingAbsoluteLocation(false);
	ShouldersMeshComponent->SetUsingAbsoluteRotation(false);
	ShouldersMeshComponent->SetUsingAbsoluteScale(false);
	if (BodyMeshComponent)
	{
		ShouldersMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	ShouldersMeshComponent->SetRelativeLocation(Entry->RelativeLocation);
	ShouldersMeshComponent->SetRelativeRotation(Entry->RelativeRotation);
	ShouldersMeshComponent->SetRelativeScale3D(Entry->RelativeScale3D);
	
	ShouldersMeshComponent->RefreshBoneTransforms();
	ShouldersMeshComponent->UpdateBounds();
	ShouldersMeshComponent->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied shoulders '%s' (mesh=%s)"), *ShouldersId.ToString(), *LoadedMesh->GetName());
	return true;
}

bool UTainlordCharacterAppearanceComponent::ApplyLeftBracer(FName LeftBracerId)
{
	CurrentAppearance.LeftBracerId = LeftBracerId;

	if (LeftBracerId.IsNone())
	{
		if (LeftBracerMeshComponent)
		{
			LeftBracerMeshComponent->SetSkeletalMesh(nullptr);
			LeftBracerMeshComponent->SetVisibility(false);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply left bracer '%s' - no catalog loaded"), *LeftBracerId.ToString());
		return false;
	}

	const FTainlordBracerEntry* Entry = LoadedCatalog->FindBracersLeftEntryForContext(LeftBracerId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Left bracer entry '%s' not found or not allowed for context (Gender=%d, Race=%d)"),
			*LeftBracerId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Left bracer entry '%s' is invalid"), *LeftBracerId.ToString());
		return false;
	}

	if (!LeftBracerMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply left bracer '%s' - LeftBracerMeshComponent is null"), *LeftBracerId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load left bracer mesh for '%s'"), *LeftBracerId.ToString());
		return false;
	}

	LeftBracerMeshComponent->SetSkeletalMesh(LoadedMesh);
	LeftBracerMeshComponent->SetUsingAbsoluteLocation(false);
	LeftBracerMeshComponent->SetUsingAbsoluteRotation(false);
	LeftBracerMeshComponent->SetUsingAbsoluteScale(false);
	if (BodyMeshComponent)
	{
		LeftBracerMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	LeftBracerMeshComponent->SetRelativeLocation(Entry->RelativeLocation);
	LeftBracerMeshComponent->SetRelativeRotation(Entry->RelativeRotation);
	LeftBracerMeshComponent->SetRelativeScale3D(Entry->RelativeScale3D);
	
	LeftBracerMeshComponent->RefreshBoneTransforms();
	LeftBracerMeshComponent->UpdateBounds();
	LeftBracerMeshComponent->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied left bracer '%s' (mesh=%s)"), *LeftBracerId.ToString(), *LoadedMesh->GetName());
	return true;
}

bool UTainlordCharacterAppearanceComponent::ApplyRightBracer(FName RightBracerId)
{
	CurrentAppearance.RightBracerId = RightBracerId;

	if (RightBracerId.IsNone())
	{
		if (RightBracerMeshComponent)
		{
			RightBracerMeshComponent->SetSkeletalMesh(nullptr);
			RightBracerMeshComponent->SetVisibility(false);
		}
		return true;
	}

	UTainlordCharacterCustomizationCatalog* LoadedCatalog = EnsureCatalogLoaded();
	if (!LoadedCatalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply right bracer '%s' - no catalog loaded"), *RightBracerId.ToString());
		return false;
	}

	const FTainlordBracerEntry* Entry = LoadedCatalog->FindBracersRightEntryForContext(RightBracerId, ActiveGender, ActiveRace);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Right bracer entry '%s' not found or not allowed for context (Gender=%d, Race=%d)"),
			*RightBracerId.ToString(), static_cast<int32>(ActiveGender), static_cast<int32>(ActiveRace));
		return false;
	}

	if (!Entry->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Right bracer entry '%s' is invalid"), *RightBracerId.ToString());
		return false;
	}

	if (!RightBracerMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Cannot apply right bracer '%s' - RightBracerMeshComponent is null"), *RightBracerId.ToString());
		return false;
	}

	USkeletalMesh* LoadedMesh = Entry->MeshAsset.LoadSynchronous();
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance: Failed to load right bracer mesh for '%s'"), *RightBracerId.ToString());
		return false;
	}

	RightBracerMeshComponent->SetSkeletalMesh(LoadedMesh);
	RightBracerMeshComponent->SetUsingAbsoluteLocation(false);
	RightBracerMeshComponent->SetUsingAbsoluteRotation(false);
	RightBracerMeshComponent->SetUsingAbsoluteScale(false);
	if (BodyMeshComponent)
	{
		RightBracerMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	RightBracerMeshComponent->SetRelativeLocation(Entry->RelativeLocation);
	RightBracerMeshComponent->SetRelativeRotation(Entry->RelativeRotation);
	RightBracerMeshComponent->SetRelativeScale3D(Entry->RelativeScale3D);
	
	RightBracerMeshComponent->RefreshBoneTransforms();
	RightBracerMeshComponent->UpdateBounds();
	RightBracerMeshComponent->SetVisibility(true);

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Applied right bracer '%s' (mesh=%s)"), *RightBracerId.ToString(), *LoadedMesh->GetName());
	return true;
}

void UTainlordCharacterAppearanceComponent::ClearAppearance()
{
	CurrentAppearance = FTainlordAppearanceData();

	if (HeadMeshComponent)
	{
		HeadMeshComponent->SetSkeletalMesh(nullptr);
	}
	if (HairMeshComponent)
	{
		HairMeshComponent->SetSkeletalMesh(nullptr);
	}
	if (BeardMeshComponent)
	{
		BeardMeshComponent->SetSkeletalMesh(nullptr);
	}
	if (ArmsMeshComponent)
	{
		ArmsMeshComponent->SetSkeletalMesh(nullptr);
	}
	if (LegsMeshComponent)
	{
		LegsMeshComponent->SetSkeletalMesh(nullptr);
	}
	if (ShouldersMeshComponent)
	{
		ShouldersMeshComponent->SetSkeletalMesh(nullptr);
		ShouldersMeshComponent->SetVisibility(false);
	}
	if (LeftBracerMeshComponent)
	{
		LeftBracerMeshComponent->SetSkeletalMesh(nullptr);
		LeftBracerMeshComponent->SetVisibility(false);
	}
	if (RightBracerMeshComponent)
	{
		RightBracerMeshComponent->SetSkeletalMesh(nullptr);
		RightBracerMeshComponent->SetVisibility(false);
	}

	// Reset skin tone using tracked parameter name
	if (HeadMeshComponent)
	{
		ResetSkinToneOnMesh(HeadMeshComponent);
	}
	if (BodyMeshComponent)
	{
		ResetSkinToneOnMesh(BodyMeshComponent);
	}
	if (ArmsMeshComponent)
	{
		ResetSkinToneOnMesh(ArmsMeshComponent);
	}
	if (LegsMeshComponent)
	{
		ResetSkinToneOnMesh(LegsMeshComponent);
	}

	SkinToneMIDs.Empty();

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance: Cleared all appearance overrides"));
}

void UTainlordCharacterAppearanceComponent::ApplySkinToneToMesh(USkeletalMeshComponent* MeshComp, const FTainlordSkinToneEntry& Entry)
{
	if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
	{
		return;
	}

	const int32 MaterialCount = MeshComp->GetNumMaterials();
	for (int32 MatIdx = 0; MatIdx < MaterialCount; ++MatIdx)
	{
		UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(MatIdx);
		if (!BaseMaterial)
		{
			continue;
		}

		// Create or reuse dynamic material instance.
		// We apply the parameter unconditionally — materials that don't have it
		// will silently ignore SetVectorParameterValue.
		UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(BaseMaterial);
		if (!MID)
		{
			MID = UMaterialInstanceDynamic::Create(BaseMaterial, MeshComp);
			if (!MID)
			{
				continue;
			}
			MeshComp->SetMaterial(MatIdx, MID);
		}

		MID->SetVectorParameterValue(Entry.MaterialParameterName, Entry.Color);
	}
}

void UTainlordCharacterAppearanceComponent::ResetSkinToneOnMesh(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
	{
		return;
	}

	const int32 MaterialCount = MeshComp->GetNumMaterials();
	for (int32 MatIdx = 0; MatIdx < MaterialCount; ++MatIdx)
	{
		UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(MatIdx);
		if (!BaseMaterial)
		{
			continue;
		}

		UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(BaseMaterial);
		if (!MID)
		{
			MID = UMaterialInstanceDynamic::Create(BaseMaterial, MeshComp);
			if (!MID)
			{
				continue;
			}
			MeshComp->SetMaterial(MatIdx, MID);
		}

		// Reset using the tracked parameter name (symmetric with apply)
		MID->SetVectorParameterValue(LastAppliedSkinToneParamName, FLinearColor::White);
	}
}

void UTainlordCharacterAppearanceComponent::CollectSkinToneMaterials()
{
	SkinToneMIDs.Empty();

	// Collect dynamic material instances from head and body meshes
	TArray<USkeletalMeshComponent*> MeshesToCheck = { HeadMeshComponent, BodyMeshComponent, ArmsMeshComponent, LegsMeshComponent };

	for (USkeletalMeshComponent* MeshComp : MeshesToCheck)
	{
		if (!MeshComp)
		{
			continue;
		}

		const int32 MaterialCount = MeshComp->GetNumMaterials();
		for (int32 MatIdx = 0; MatIdx < MaterialCount; ++MatIdx)
		{
			if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(MeshComp->GetMaterial(MatIdx)))
			{
				SkinToneMIDs.Add(MID);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Leader pose rebind
// ---------------------------------------------------------------------------

void UTainlordCharacterAppearanceComponent::RebindLeaderPose()
{
	// Body is the leader for head, arms, legs, and accessories.
	// Hair and beard follow the head mesh.
	if (!BodyMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordAppearance::RebindLeaderPose - BodyMeshComponent is null, cannot rebind"));
		return;
	}

	int32 RebindCount = 0;

	if (HeadMeshComponent && HeadMeshComponent->GetSkeletalMeshAsset())
	{
		if (HeadMeshComponent->GetAttachParent() != BodyMeshComponent)
		{
			HeadMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		HeadMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		HeadMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	if (ArmsMeshComponent && ArmsMeshComponent->GetSkeletalMeshAsset())
	{
		if (ArmsMeshComponent->GetAttachParent() != BodyMeshComponent)
		{
			ArmsMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		ArmsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ArmsMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	if (LegsMeshComponent && LegsMeshComponent->GetSkeletalMeshAsset())
	{
		if (LegsMeshComponent->GetAttachParent() != BodyMeshComponent)
		{
			LegsMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		LegsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		LegsMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	if (HairMeshComponent && HeadMeshComponent)
	{
		if (HairMeshComponent->GetAttachParent() != HeadMeshComponent)
		{
			HairMeshComponent->AttachToComponent(HeadMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		HairMeshComponent->SetLeaderPoseComponent(HeadMeshComponent);
		HairMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	if (BeardMeshComponent && HeadMeshComponent)
	{
		if (BeardMeshComponent->GetAttachParent() != HeadMeshComponent)
		{
			BeardMeshComponent->AttachToComponent(HeadMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		BeardMeshComponent->SetLeaderPoseComponent(HeadMeshComponent);
		BeardMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	if (ShouldersMeshComponent && ShouldersMeshComponent->GetSkeletalMeshAsset())
	{
		if (ShouldersMeshComponent->GetAttachParent() != BodyMeshComponent)
		{
			ShouldersMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		ShouldersMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShouldersMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	if (LeftBracerMeshComponent && LeftBracerMeshComponent->GetSkeletalMeshAsset())
	{
		if (LeftBracerMeshComponent->GetAttachParent() != BodyMeshComponent)
		{
			LeftBracerMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		LeftBracerMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		LeftBracerMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	if (RightBracerMeshComponent && RightBracerMeshComponent->GetSkeletalMeshAsset())
	{
		if (RightBracerMeshComponent->GetAttachParent() != BodyMeshComponent)
		{
			RightBracerMeshComponent->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		RightBracerMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		RightBracerMeshComponent->SetVisibility(true);
		++RebindCount;
	}

	UE_LOG(LogTemp, Log, TEXT("TainlordAppearance::RebindLeaderPose - Rebound %d components to leader pose"), RebindCount);
}

// ---------------------------------------------------------------------------
// Eye socket attachment
// ---------------------------------------------------------------------------

/** Standard socket names that every head mesh must define. */
static const FName EYE_SOCKET_LEFT  = FName(TEXT("Eye_L"));
static const FName EYE_SOCKET_RIGHT = FName(TEXT("Eye_R"));

void UTainlordCharacterAppearanceComponent::AttachEyesToHeadSockets()
{
	if (!HeadMeshComponent || !HeadMeshComponent->GetSkeletalMeshAsset())
	{
		return;
	}

	USkeletalMesh* HeadMeshAsset = HeadMeshComponent->GetSkeletalMeshAsset();
	USkeleton* HeadSkeleton = HeadMeshAsset ? HeadMeshAsset->GetSkeleton() : nullptr;
	EnsureEyeRenderAssets();

	const auto FindMeshOnlySocket = [HeadMeshAsset](const FName SocketName) -> const USkeletalMeshSocket*
	{
		if (!HeadMeshAsset)
		{
			return nullptr;
		}

		for (const USkeletalMeshSocket* Socket : HeadMeshAsset->GetMeshOnlySocketList())
		{
			if (Socket && Socket->SocketName == SocketName)
			{
				return Socket;
			}
		}

		return nullptr;
	};

	const auto ResolveSocket = [HeadMeshAsset, HeadSkeleton, &FindMeshOnlySocket](const FName SocketName) -> const USkeletalMeshSocket*
	{
		const USkeletalMeshSocket* MeshSocket = FindMeshOnlySocket(SocketName);
		const USkeletalMeshSocket* SkeletonSocket = HeadSkeleton ? HeadSkeleton->FindSocket(SocketName) : nullptr;

		if (MeshSocket && SkeletonSocket)
		{
			const bool bMeshIdentity = MeshSocket->RelativeLocation.IsNearlyZero() &&
				MeshSocket->RelativeRotation.IsNearlyZero() &&
				MeshSocket->RelativeScale.Equals(FVector::OneVector);
			const bool bSkeletonMeaningful = !SkeletonSocket->RelativeLocation.IsNearlyZero() ||
				!SkeletonSocket->RelativeRotation.IsNearlyZero() ||
				!SkeletonSocket->RelativeScale.Equals(FVector::OneVector);

			if (bMeshIdentity && bSkeletonMeaningful)
			{
				return SkeletonSocket;
			}

			return MeshSocket;
		}

		if (MeshSocket)
		{
			return MeshSocket;
		}

		return SkeletonSocket;
	};

	auto AttachEye = [this, &ResolveSocket](UStaticMeshComponent* EyeComponent, const FName SocketName)
	{
		if (!EyeComponent)
		{
			return;
		}

		if (const USkeletalMeshSocket* Socket = ResolveSocket(SocketName))
		{
			const FTransform SocketWorldTransform = Socket->GetSocketTransform(HeadMeshComponent);
			const FVector SocketWorldLoc = SocketWorldTransform.GetLocation();
			const FQuat SocketWorldRot = SocketWorldTransform.GetRotation();

			FVector Offset = FVector::ZeroVector;
			FRotator Rotation = FRotator::ZeroRotator;

			if (SocketName == EYE_SOCKET_LEFT)
			{
				Offset = !CurrentHeadEntry.LeftEyeRelativeLocation.IsNearlyZero()
					? CurrentHeadEntry.LeftEyeRelativeLocation
					: FVector::ZeroVector;
				Rotation = !CurrentHeadEntry.LeftEyeRelativeRotation.IsNearlyZero()
					? CurrentHeadEntry.LeftEyeRelativeRotation
					: FRotator::ZeroRotator;
			}
			else
			{
				Offset = !CurrentHeadEntry.RightEyeRelativeLocation.IsNearlyZero()
					? CurrentHeadEntry.RightEyeRelativeLocation
					: FVector::ZeroVector;
				Rotation = !CurrentHeadEntry.RightEyeRelativeRotation.IsNearlyZero()
					? CurrentHeadEntry.RightEyeRelativeRotation
					: FRotator::ZeroRotator;
			}

			if (Socket->RelativeLocation.IsNearlyZero() && Offset.IsNearlyZero())
			{
				const FVector HeadForward = HeadMeshComponent->GetForwardVector();
				const FVector HeadRight = HeadMeshComponent->GetRightVector();
				const FVector HeadUp = HeadMeshComponent->GetUpVector();
				const float SideSign = (SocketName == EYE_SOCKET_LEFT) ? -1.0f : 1.0f;
				Offset = (HeadForward * 12.0f) + (HeadUp * 11.0f) + (HeadRight * SideSign * 3.5f);
			}

			const FVector FinalWorldPos = SocketWorldLoc + SocketWorldRot.RotateVector(Offset);
			const FRotator FinalWorldRot = (SocketWorldRot * Rotation.Quaternion()).Rotator();

			EyeComponent->AttachToComponent(HeadMeshComponent, FAttachmentTransformRules::KeepWorldTransform, SocketName);
			EyeComponent->SetWorldLocation(FinalWorldPos);
			EyeComponent->SetWorldRotation(FinalWorldRot);
			EyeComponent->SetVisibility(true);
		}
		else
		{
			EyeComponent->SetVisibility(false);
		}
	};

	AttachEye(LeftEyeMeshComponent, EYE_SOCKET_LEFT);
	AttachEye(RightEyeMeshComponent, EYE_SOCKET_RIGHT);
}

void UTainlordCharacterAppearanceComponent::EnsureEyeRenderAssets()
{
	if (!ResolvedEyeStaticMeshAsset)
	{
		ResolvedEyeStaticMeshAsset = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Man/Eyes/Eye/SM_PhotorealEyeGeo.SM_PhotorealEyeGeo"));
	}

	if (!ResolvedEyeMaterialAsset)
	{
		ResolvedEyeMaterialAsset = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Man/Eyes/MI_EyeRefractive_Bust23.MI_EyeRefractive_Bust23"));
	}

	const auto ApplyEyeSetup = [this](UStaticMeshComponent* EyeComponent)
	{
		if (!EyeComponent)
		{
			return;
		}

		if (ResolvedEyeStaticMeshAsset && EyeComponent->GetStaticMesh() != ResolvedEyeStaticMeshAsset)
		{
			EyeComponent->SetStaticMesh(ResolvedEyeStaticMeshAsset);
		}

		if (ResolvedEyeMaterialAsset)
		{
			EyeComponent->SetMaterial(0, ResolvedEyeMaterialAsset);
		}

		EyeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EyeComponent->SetCastShadow(false);
		EyeComponent->SetVisibility(true);
		EyeComponent->TranslucencySortPriority = 10;
	};

	ApplyEyeSetup(LeftEyeMeshComponent);
	ApplyEyeSetup(RightEyeMeshComponent);
}

