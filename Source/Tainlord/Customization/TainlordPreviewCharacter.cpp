// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordPreviewCharacter.h"
#include "Customization/TainlordCharacterAppearanceComponent.h"
#include "Customization/TainlordCharacterCustomizationCatalog.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkinnedAssetCommon.h"

namespace
{
	static const TCHAR* CanonicalMaleBodyMeshPath = TEXT("/Game/ModularPrep/Male01/Body.Body");

	UAnimSequence* LoadPreviewIdleAnimation()
	{
		static const TCHAR* IdleAnimPath = TEXT("/Game/DynamicSwordAnimset/Animations/InPlace/Idle_Eqip_01.Idle_Eqip_01");
		return LoadObject<UAnimSequence>(nullptr, IdleAnimPath);
	}

	USkeletalMesh* LoadCanonicalPreviewBodyMesh()
	{
		return LoadObject<USkeletalMesh>(nullptr, CanonicalMaleBodyMeshPath);
	}
}

ATainlordPreviewCharacter::ATainlordPreviewCharacter()
{
	// --- Create modular mesh components ---

	// Body = ACharacter's built-in GetMesh() — no need to create.

	// Head: separate skeletal mesh, attached to body, follows body skeleton via leader pose.
	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh());
	HeadMesh->SetLeaderPoseComponent(GetMesh());
	HeadMesh->bUseAttachParentBound = true;
	HeadMesh->SetBoundsScale(4.0f);

	// Arms: separate skeletal mesh, attached to body, follows body skeleton via leader pose.
	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmsMesh->SetupAttachment(GetMesh());
	ArmsMesh->SetLeaderPoseComponent(GetMesh());
	ArmsMesh->bUseAttachParentBound = true;
	ArmsMesh->SetBoundsScale(4.0f);

	// Legs: separate skeletal mesh, attached to body, follows body skeleton via leader pose.
	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetLeaderPoseComponent(GetMesh());
	LegsMesh->bUseAttachParentBound = true;
	LegsMesh->SetBoundsScale(4.0f);

	// Hair: separate skeletal mesh, attached to head, follows head skeleton via leader pose.
	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMesh->SetupAttachment(HeadMesh);
	HairMesh->SetLeaderPoseComponent(HeadMesh);
	HairMesh->bUseAttachParentBound = true;
	HairMesh->SetBoundsScale(4.0f);

	// Beard: separate skeletal mesh, attached to head, follows head skeleton via leader pose.
	BeardMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeardMesh"));
	BeardMesh->SetupAttachment(HeadMesh);
	BeardMesh->SetLeaderPoseComponent(HeadMesh);
	BeardMesh->bUseAttachParentBound = true;
	BeardMesh->SetBoundsScale(4.0f);

	// Shoulders: separate skeletal mesh, attached to body, follows body skeleton via leader pose (accessory).
	ShouldersMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShouldersMesh"));
	ShouldersMesh->SetupAttachment(GetMesh());
	ShouldersMesh->SetLeaderPoseComponent(GetMesh());
	ShouldersMesh->bUseAttachParentBound = true;
	ShouldersMesh->SetBoundsScale(4.0f);

	// Left bracer: separate skeletal mesh, attached to body, follows body skeleton via leader pose (accessory).
	LeftBracerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftBracerMesh"));
	LeftBracerMesh->SetupAttachment(GetMesh());
	LeftBracerMesh->SetLeaderPoseComponent(GetMesh());
	LeftBracerMesh->bUseAttachParentBound = true;
	LeftBracerMesh->SetBoundsScale(4.0f);

	// Right bracer: separate skeletal mesh, attached to body, follows body skeleton via leader pose (accessory).
	RightBracerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightBracerMesh"));
	RightBracerMesh->SetupAttachment(GetMesh());
	RightBracerMesh->SetLeaderPoseComponent(GetMesh());
	RightBracerMesh->bUseAttachParentBound = true;
	RightBracerMesh->SetBoundsScale(4.0f);

	// Eyes: two separate static mesh components attached to HeadMesh via sockets.
	// Each head mesh must define "Eye_L" and "Eye_R" sockets at the correct positions.
	// The eye components are initially attached to HeadMesh; ApplyHead() will
	// re-attach them to the correct sockets after the head mesh is loaded.
	LeftEyeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftEyeMesh"));
	LeftEyeMesh->SetupAttachment(HeadMesh);
	LeftEyeMesh->bUseAttachParentBound = true;
	LeftEyeMesh->SetBoundsScale(4.0f);

	RightEyeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightEyeMesh"));
	RightEyeMesh->SetupAttachment(HeadMesh);
	RightEyeMesh->bUseAttachParentBound = true;
	RightEyeMesh->SetBoundsScale(4.0f);

	// --- Create and wire appearance component ---
	AppearanceComponent = CreateDefaultSubobject<UTainlordCharacterAppearanceComponent>(TEXT("AppearanceComponent"));

	// Wire mesh component references.
	// Body = the ACharacter default mesh.
	// Head/Hair/Beard = separate modular components created above.
	// Eye components are wired so AppearanceComponent can re-attach them to sockets.
	AppearanceComponent->BodyMeshComponent = GetMesh();
	AppearanceComponent->HeadMeshComponent = HeadMesh;
	AppearanceComponent->ArmsMeshComponent = ArmsMesh;
	AppearanceComponent->LegsMeshComponent = LegsMesh;
	AppearanceComponent->HairMeshComponent = HairMesh;
	AppearanceComponent->BeardMeshComponent = BeardMesh;
	AppearanceComponent->LeftEyeMeshComponent = LeftEyeMesh;
	AppearanceComponent->RightEyeMeshComponent = RightEyeMesh;
	AppearanceComponent->ShouldersMeshComponent = ShouldersMesh;
	AppearanceComponent->LeftBracerMeshComponent = LeftBracerMesh;
	AppearanceComponent->RightBracerMeshComponent = RightBracerMesh;
}

void ATainlordPreviewCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* BodyMesh = GetMesh())
	{
		if (!BodyMesh->GetSkeletalMeshAsset())
		{
			if (USkeletalMesh* FallbackBodyMesh = LoadCanonicalPreviewBodyMesh())
			{
				BodyMesh->SetSkeletalMesh(FallbackBodyMesh);
				BodyMesh->SetVisibility(true);
				BodyMesh->RefreshBoneTransforms();
				BodyMesh->UpdateBounds();
				UE_LOG(LogTemp, Warning, TEXT("TainlordPreviewCharacter: Body mesh was empty - applied canonical preview fallback '%s'"),
					*FallbackBodyMesh->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TainlordPreviewCharacter: Failed to load canonical preview body fallback '%s'"),
					CanonicalMaleBodyMeshPath);
			}
		}
	}

	// --- Debug: log current mesh asset names before any runtime logic ---
	UE_LOG(LogTemp, Log, TEXT("TainlordPreviewCharacter::BeginPlay — Initial mesh assets: "
		"Body=%s, Head=%s, Arms=%s, Legs=%s"),
		(GetMesh() && GetMesh()->GetSkeletalMeshAsset()) ? *GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("null"),
		(HeadMesh && HeadMesh->GetSkeletalMeshAsset()) ? *HeadMesh->GetSkeletalMeshAsset()->GetName() : TEXT("null"),
		(ArmsMesh && ArmsMesh->GetSkeletalMeshAsset()) ? *ArmsMesh->GetSkeletalMeshAsset()->GetName() : TEXT("null"),
		(LegsMesh && LegsMesh->GetSkeletalMeshAsset()) ? *LegsMesh->GetSkeletalMeshAsset()->GetName() : TEXT("null"));

	// Copy the catalog reference from the Blueprint-editable property
	// to the appearance component at runtime.
	if (AppearanceComponent && !CatalogAsset.IsNull())
	{
		AppearanceComponent->Catalog = CatalogAsset;
		UE_LOG(LogTemp, Log, TEXT("TainlordPreviewCharacter: Catalog set to '%s'"),
			*CatalogAsset.GetLongPackageName());
	}
	
	// Resolve shared eye assets robustly. If the Blueprint-authored properties come in null
	// at runtime, fall back to the known project assets so preview does not silently lose eyes.
	UStaticMesh* ResolvedEyeMesh = EyeStaticMeshAsset;
	if (!ResolvedEyeMesh)
	{
		ResolvedEyeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Man/Eyes/Eye/SM_PhotorealEyeGeo.SM_PhotorealEyeGeo"));
	}

	UMaterialInterface* ResolvedEyeMaterial = EyeMaterialOverride;
	if (!ResolvedEyeMaterial)
	{
		ResolvedEyeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Man/Eyes/MI_EyeRefractive_Bust23.MI_EyeRefractive_Bust23"));
	}

	// Apply the shared eye static mesh asset to both eye components.
	if (ResolvedEyeMesh)
	{
		if (LeftEyeMesh)
		{
			LeftEyeMesh->SetStaticMesh(ResolvedEyeMesh);
			LeftEyeMesh->SetWorldScale3D(FVector(1.0f));
		}
		if (RightEyeMesh)
		{
			RightEyeMesh->SetStaticMesh(ResolvedEyeMesh);
			RightEyeMesh->SetWorldScale3D(FVector(1.0f));
		}

		// Apply optional material override
		if (ResolvedEyeMaterial)
		{
			if (LeftEyeMesh)
			{
				LeftEyeMesh->SetMaterial(0, ResolvedEyeMaterial);
			}
			if (RightEyeMesh)
			{
				RightEyeMesh->SetMaterial(0, ResolvedEyeMaterial);
			}
		}
	}

	// If the Blueprint does not provide an AnimBP, fall back to a looping idle on Body.
	// Follower meshes must stay on leader pose; do not play animation on them directly.
	if (USkeletalMeshComponent* BodyMesh = GetMesh())
	{
		if (UAnimSequence* PreviewIdle = LoadPreviewIdleAnimation())
		{
			if (BodyMesh->GetAnimClass() == nullptr)
			{
				BodyMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
				BodyMesh->PlayAnimation(PreviewIdle, true);
				UE_LOG(LogTemp, Log, TEXT("TainlordPreviewCharacter: Applied fallback idle animation '%s' to Body"),
					*PreviewIdle->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("TainlordPreviewCharacter: Body has AnimBP, skipping fallback idle setup"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TainlordPreviewCharacter: Failed to load fallback idle animation"));
		}
	}

	// --- Force visibility on all modular components ---
	// This ensures that even if Blueprint defaults had visibility off,
	// we explicitly turn them on at runtime.
	if (HeadMesh) { HeadMesh->SetVisibility(true); }
	if (ArmsMesh) { ArmsMesh->SetVisibility(true); }
	if (LegsMesh) { LegsMesh->SetVisibility(true); }
	if (HairMesh) { HairMesh->SetVisibility(true); }
	if (BeardMesh) { BeardMesh->SetVisibility(true); }
	if (LeftEyeMesh) { LeftEyeMesh->SetVisibility(true); }
	if (RightEyeMesh) { RightEyeMesh->SetVisibility(true); }
	// Accessories start hidden — visibility is set by ApplyShoulders/ApplyLeftBracer/ApplyRightBracer
	// when a valid catalog entry is applied. No forced visibility here.

	UE_LOG(LogTemp, Log, TEXT("TainlordPreviewCharacter: BeginPlay complete. "
		"Components: Body=%s, Head=%s, Arms=%s, Legs=%s, Hair=%s, Beard=%s, Shoulders=%s, BracerL=%s, BracerR=%s"),
		GetMesh() ? TEXT("OK") : TEXT("null"),
		HeadMesh ? TEXT("OK") : TEXT("null"),
		ArmsMesh ? TEXT("OK") : TEXT("null"),
		LegsMesh ? TEXT("OK") : TEXT("null"),
		HairMesh ? TEXT("OK") : TEXT("null"),
		BeardMesh ? TEXT("OK") : TEXT("null"),
		ShouldersMesh ? TEXT("OK") : TEXT("null"),
		LeftBracerMesh ? TEXT("OK") : TEXT("null"),
		RightBracerMesh ? TEXT("OK") : TEXT("null"));
}

#if WITH_EDITOR
void ATainlordPreviewCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// --- Editor Preview Leader Pose Rebind ---
	// When a Blueprint is compiled or properties change in editor,
	// the leader pose binding from the constructor may be lost.
	// Re-establish it here to ensure modular meshes render in editor preview.
	
	USkeletalMeshComponent* BodyMesh = GetMesh();
	if (!BodyMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("TainlordPreviewCharacter::OnConstruction - Body mesh is null"));
		return;
	}
	if (!BodyMesh->GetSkeletalMeshAsset())
	{
		if (USkeletalMesh* FallbackBodyMesh = LoadCanonicalPreviewBodyMesh())
		{
			BodyMesh->SetSkeletalMesh(FallbackBodyMesh);
			BodyMesh->SetVisibility(true);
			BodyMesh->RefreshBoneTransforms();
			BodyMesh->UpdateBounds();
			UE_LOG(LogTemp, Warning, TEXT("TainlordPreviewCharacter::OnConstruction - Body mesh was empty, applied canonical preview fallback '%s'"),
				*FallbackBodyMesh->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TainlordPreviewCharacter::OnConstruction - failed to load canonical preview body fallback '%s'"),
				CanonicalMaleBodyMeshPath);
			return;
		}
	}

	// Helper lambda to rebind a follower component to its leader
	auto RebindFollower = [](USkeletalMeshComponent* Follower, USkeletalMeshComponent* Leader, const TCHAR* ComponentName)
	{
		if (!Follower)
		{
			return;
		}

		// Ensure attachment
		if (Follower->GetAttachParent() != Leader)
		{
			Follower->AttachToComponent(Leader, FAttachmentTransformRules::KeepRelativeTransform);
		}

		// Set leader pose for animation following
		if (Follower->GetSkeletalMeshAsset())
		{
			Follower->SetLeaderPoseComponent(Leader);
			Follower->RefreshBoneTransforms();
			Follower->UpdateBounds();
			Follower->SetVisibility(true);
			
			UE_LOG(LogTemp, Log, TEXT("TainlordPreviewCharacter::OnConstruction - Rebound %s to leader, mesh=%s"),
				ComponentName,
				*Follower->GetSkeletalMeshAsset()->GetName());
		}
	};

	// Rebind all modular components that follow the body
	RebindFollower(HeadMesh, BodyMesh, TEXT("HeadMesh"));
	RebindFollower(ArmsMesh, BodyMesh, TEXT("ArmsMesh"));
	RebindFollower(LegsMesh, BodyMesh, TEXT("LegsMesh"));
	RebindFollower(ShouldersMesh, BodyMesh, TEXT("ShouldersMesh"));
	RebindFollower(LeftBracerMesh, BodyMesh, TEXT("LeftBracerMesh"));
	RebindFollower(RightBracerMesh, BodyMesh, TEXT("RightBracerMesh"));

	// Hair and beard follow the head mesh
	if (HeadMesh && HeadMesh->GetSkeletalMeshAsset())
	{
		RebindFollower(HairMesh, HeadMesh, TEXT("HairMesh"));
		RebindFollower(BeardMesh, HeadMesh, TEXT("BeardMesh"));
	}

	// Eyes attach to head sockets - ensure they have valid meshes
	if (LeftEyeMesh)
	{
		LeftEyeMesh->AttachToComponent(HeadMesh, FAttachmentTransformRules::KeepRelativeTransform);
		LeftEyeMesh->SetVisibility(true);
	}
	if (RightEyeMesh)
	{
		RightEyeMesh->AttachToComponent(HeadMesh, FAttachmentTransformRules::KeepRelativeTransform);
		RightEyeMesh->SetVisibility(true);
	}

	UE_LOG(LogTemp, Log, TEXT("TainlordPreviewCharacter::OnConstruction - Leader pose rebind complete"));
}
#endif
