// Copyright Kingdawn. All Rights Reserved.

#include "Data/KDBuildAbilityResolver.h"
#include "Core/KDGameplayTags.h"
#include "Abilities/GameplayAbility.h"

namespace
{
	struct FKDAbilityMapping
	{
		FGameplayTag Tag;
		const TCHAR* TagFallbackName;
		const TCHAR* BlueprintPath;
	};

	FGameplayTag SafeResolveTag(const FGameplayTag& NativeTag, const TCHAR* FallbackName)
	{
		if (NativeTag.IsValid())
		{
			return NativeTag;
		}
		return FGameplayTag::RequestGameplayTag(FName(FallbackName), false);
	}

	/**
	 * Master mapping table: gameplay tag → Blueprint ability class path.
	 * Each entry uses the native tag from FKDGameplayTags first,
	 * falling back to string-based tag resolution if the native tag
	 * isn't registered yet (e.g., during early module init).
	 */
	const TArray<FKDAbilityMapping>& GetAbilityMappings()
	{
		static TArray<FKDAbilityMapping> Mappings;
		if (Mappings.Num() == 0)
		{
			// --- Shared ---
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Combat_Action_Attack_Basic, TEXT("KD.Combat.Action.Attack.Basic")),
				TEXT("KD.Combat.Action.Attack.Basic"),
				TEXT("/Game/KingdawnCombat/Abilities/BP_GA_Attack.BP_GA_Attack_C")
			});
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Combat_Skill_Combo_Light, TEXT("KD.Combat.Skill.Combo.Light")),
				TEXT("KD.Combat.Skill.Combo.Light"),
				TEXT("/Game/KingdawnCombat/Blueprints/BP_GA_ComboAttack.BP_GA_ComboAttack_C")
			});

			// --- BladeShield ---
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Skill_BladeShield_HeavyStrike, TEXT("KD.Skill.BladeShield.HeavyStrike")),
				TEXT("KD.Skill.BladeShield.HeavyStrike"),
				TEXT("/Game/KingdawnCombat/Blueprints/BP_GA_BladeShield_HeavyStrike.BP_GA_BladeShield_HeavyStrike_C")
			});
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Skill_BladeShield_ShieldBash, TEXT("KD.Skill.BladeShield.ShieldBash")),
				TEXT("KD.Skill.BladeShield.ShieldBash"),
				TEXT("/Game/KingdawnCombat/Blueprints/BP_GA_BladeShield_ShieldBash.BP_GA_BladeShield_ShieldBash_C")
			});

			// --- Wizard ---
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Skill_Wizard_ArcaneBolt, TEXT("KD.Skill.Wizard.ArcaneBolt")),
				TEXT("KD.Skill.Wizard.ArcaneBolt"),
				TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_ArcaneBolt.BP_GA_Wizard_ArcaneBolt_C")
			});
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Skill_Wizard_Fireball, TEXT("KD.Skill.Wizard.Fireball")),
				TEXT("KD.Skill.Wizard.Fireball"),
				TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_Fireball.BP_GA_Wizard_Fireball_C")
			});
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Skill_Wizard_FrostNova, TEXT("KD.Skill.Wizard.FrostNova")),
				TEXT("KD.Skill.Wizard.FrostNova"),
				TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_FrostNova.BP_GA_Wizard_FrostNova_C")
			});
			Mappings.Add({
				SafeResolveTag(FKDGameplayTags::Skill_Wizard_ManaShield, TEXT("KD.Skill.Wizard.ManaShield")),
				TEXT("KD.Skill.Wizard.ManaShield"),
				TEXT("/Game/KingdawnCombat/Blueprints/Wizard/BP_GA_Wizard_ManaShield.BP_GA_Wizard_ManaShield_C")
			});

			// Wizard also has a branch-specific basic attack (different from shared shell)
			// This uses the same tag as the shared basic but a Wizard-specific class
			// Note: the Wizard basic is granted separately when the build's BasicAttackAbility
			// is NOT KD.Combat.Action.Attack.Basic (i.e., it's a different tag).
			// For now, the Wizard basic attack uses its own class path:
			// We don't add it here because the shared basic attack tag already maps above.
			// If a Wizard build needs a different basic attack class, it should use a
			// distinct tag in the build definition.
		}
		return Mappings;
	}
}

TSubclassOf<UGameplayAbility> UKDBuildAbilityResolver::ResolveAbilityClass(const FGameplayTag& AbilityTag)
{
	if (!AbilityTag.IsValid())
	{
		return nullptr;
	}

	const TArray<FKDAbilityMapping>& Mappings = GetAbilityMappings();

	for (const FKDAbilityMapping& Mapping : Mappings)
	{
		// Match against stored tag, or re-resolve from fallback name
		FGameplayTag MappingTag = Mapping.Tag;
		if (!MappingTag.IsValid())
		{
			MappingTag = FGameplayTag::RequestGameplayTag(FName(Mapping.TagFallbackName), false);
		}

		if (MappingTag.IsValid() && MappingTag == AbilityTag)
		{
			UClass* LoadedClass = FSoftClassPath(Mapping.BlueprintPath).TryLoadClass<UGameplayAbility>();
			if (LoadedClass)
			{
				return LoadedClass;
			}

			UE_LOG(LogTemp, Warning, TEXT("KDBuildAbilityResolver: Tag %s mapped to %s but class failed to load"),
				*AbilityTag.ToString(), Mapping.BlueprintPath);
			return nullptr;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("KDBuildAbilityResolver: No mapping found for tag %s"), *AbilityTag.ToString());
	return nullptr;
}

void UKDBuildAbilityResolver::ResolveAbilityClasses(
	const TArray<FGameplayTag>& AbilityTags,
	TArray<TSubclassOf<UGameplayAbility>>& OutClasses,
	TArray<FGameplayTag>& OutTags)
{
	OutClasses.Reset();
	OutTags.Reset();

	for (const FGameplayTag& Tag : AbilityTags)
	{
		TSubclassOf<UGameplayAbility> AbilityClass = ResolveAbilityClass(Tag);
		if (AbilityClass)
		{
			OutClasses.Add(AbilityClass);
			OutTags.Add(Tag);
		}
	}
}
