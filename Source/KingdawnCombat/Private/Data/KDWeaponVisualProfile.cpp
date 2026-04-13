// Copyright Kingdawn. All Rights Reserved.

#include "Data/KDWeaponVisualProfile.h"

UKDWeaponVisualProfile::UKDWeaponVisualProfile()
	: WeaponFamily(EKDWeaponFamily::None)
	, SocketName(NAME_None)
	, AttachBoneName(NAME_None)
	, RelativeOffset(FVector::ZeroVector)
	, RelativeRotation(FRotator::ZeroRotator)
	, RelativeScale(FVector::OneVector)
	, bUseReferenceSocketFallback(false)
	, ReferenceSocketName(NAME_None)
{
}
