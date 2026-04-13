// Copyright Kingdawn. All Rights Reserved.
// Lightweight tag-to-ability-class resolver for build-driven ability granting.
// Maps known gameplay tags to their Blueprint ability class paths.
// This is intentionally a simple static lookup, not a full registry system.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "KDBuildAbilityResolver.generated.h"

class UGameplayAbility;

/**
 * Static resolver that maps gameplay tags to ability Blueprint classes.
 *
 * This keeps the tag→class mapping in one place so that build-driven
 * ability granting does not need to duplicate FSoftClassPath strings
 * across multiple files.
 *
 * The resolver is intentionally small and pragmatic. It covers the
 * currently known branches (BladeShield, Wizard) and their abilities.
 * New branches add entries to the internal table without changing callers.
 */
UCLASS()
class KINGDAWNCOMBAT_API UKDBuildAbilityResolver : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Resolve a gameplay tag to its corresponding ability class.
	 * Returns nullptr if the tag is unknown or the asset cannot be loaded.
	 *
	 * @param AbilityTag The gameplay tag identifying the ability
	 * @return The loaded ability class, or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Kingdawn|Build")
	static TSubclassOf<UGameplayAbility> ResolveAbilityClass(const FGameplayTag& AbilityTag);

	/**
	 * Resolve multiple tags in one call.
	 * Unknown tags are silently skipped (logged at Warning level).
	 *
	 * @param AbilityTags Array of gameplay tags to resolve
	 * @param OutClasses Output: resolved ability classes (same order, unknown tags omitted)
	 * @param OutTags Output: corresponding tags for each resolved class
	 */
	static void ResolveAbilityClasses(
		const TArray<FGameplayTag>& AbilityTags,
		TArray<TSubclassOf<UGameplayAbility>>& OutClasses,
		TArray<FGameplayTag>& OutTags);
};
