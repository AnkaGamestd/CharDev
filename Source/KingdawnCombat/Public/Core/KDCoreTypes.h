// Copyright Kingdawn. All Rights Reserved.
// Adapted from AscentCoreInterfaces/ACFCoreTypes.h

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "KDCoreTypes.generated.h"

/**
 * Base struct for tag-identified gameplay objects.
 */
USTRUCT(BlueprintType)
struct KINGDAWNCOMBAT_API FKDStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdawn")
	FGameplayTag TagName;

	FORCEINLINE bool operator==(const FGameplayTag& Other) const { return TagName == Other; }
	FORCEINLINE bool operator!=(const FGameplayTag& Other) const { return TagName != Other; }
	FORCEINLINE bool operator==(const FKDStruct& Other) const { return TagName == Other.TagName; }
	FORCEINLINE bool operator!=(const FKDStruct& Other) const { return TagName != Other.TagName; }
};

/**
 * 8-way directional enum used for hit directions and dodge directions.
 */
UENUM(BlueprintType)
enum class EKDDirection : uint8
{
	Front = 0,
	Back = 1,
	Left = 2,
	Right = 3,
	FrontRight = 4,
	BackRight = 5,
	BackLeft = 6,
	FrontLeft = 7
};
