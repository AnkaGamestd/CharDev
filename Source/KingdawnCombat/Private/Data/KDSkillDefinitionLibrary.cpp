// Copyright Kingdawn. All Rights Reserved.

#include "Data/KDSkillDefinitionLibrary.h"
#include "Data/KDAbilityDefinition.h"
#include "Core/KDGameplayTags.h"
#include "UObject/SoftObjectPath.h"

UKDAbilityDefinition* UKDSkillDefinitionLibrary::CreateBladeShieldSkill(UObject* Outer, const FString& Name)
{
	UKDAbilityDefinition* Def = NewObject<UKDAbilityDefinition>(Outer, FName(*Name));
	if (!Def)
	{
		return nullptr;
	}

	// Mastery-first: write to primary mastery fields, then sync legacy mirror
	Def->Compatibility.RequiredMasteryClass = EKDMasteryClass::Warrior;
	Def->Compatibility.AllowedMasteryBranches = { EKDMasteryBranch::BladeShield };
	Def->Compatibility.RequiredWeapons = { EKDWeaponType::OneHandedSword, EKDWeaponType::Shield };
	Def->Compatibility.SyncLegacyFromMastery();

	return Def;
}

UKDAbilityDefinition* UKDSkillDefinitionLibrary::CreateWizardSkill(UObject* Outer, const FString& Name)
{
	UKDAbilityDefinition* Def = NewObject<UKDAbilityDefinition>(Outer, FName(*Name));
	if (!Def)
	{
		return nullptr;
	}

	// Mastery-first: write to primary mastery fields, then sync legacy mirror
	Def->Compatibility.RequiredMasteryClass = EKDMasteryClass::Mage;
	Def->Compatibility.AllowedMasteryBranches = { EKDMasteryBranch::Wizard };
	Def->Compatibility.RequiredWeapons = { EKDWeaponType::Staff };
	Def->Compatibility.SyncLegacyFromMastery();

	return Def;
}

TArray<UKDAbilityDefinition*> UKDSkillDefinitionLibrary::CreateBladeShieldPackage(UObject* Outer)
{
	TArray<UKDAbilityDefinition*> Package;

	// --- 1. Basic Attack ---
	// The fundamental melee attack for Blade & Shield.
	// Mouse double-click triggers this. Low stamina, no cooldown.
	// Gameplay role: bread-and-butter auto-attack for damage rotation.
	{
		UKDAbilityDefinition* BasicAttack = CreateBladeShieldSkill(Outer, TEXT("DA_BladeShield_BasicAttack"));
		BasicAttack->DisplayName = FText::FromString(TEXT("Blade Strike"));
		BasicAttack->Description = FText::FromString(TEXT("A swift sword strike. The basic attack for Blade & Shield warriors."));
		BasicAttack->DefinitionType = EKDAbilityDefinitionType::ActiveSkill;
		BasicAttack->SkillTier = EKDSkillTier::Common;
		BasicAttack->AbilityTag = FKDGameplayTags::Skill_BladeShield_BasicAttack.IsValid()
			? FKDGameplayTags::Skill_BladeShield_BasicAttack
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.BladeShield.BasicAttack")));
		BasicAttack->StaminaCost = 5.f;
		BasicAttack->ManaCost = 0.f;
		BasicAttack->CooldownDuration = 0.f;
		BasicAttack->bCommitOnActivate = true;
		BasicAttack->bEndOnMontageComplete = true;
		BasicAttack->MontagePlayRate = 1.f;
		BasicAttack->bRequiresTarget = true;
		BasicAttack->MinRange = 0.f;
		BasicAttack->MaxRange = 150.f;
		BasicAttack->bAutoFaceTarget = true;
		BasicAttack->FacingInterpSpeed = 0.f; // instant snap
		BasicAttack->bHotbarAssignable = false; // basic attack is mouse-triggered, not hotbar
		// Basic attack requires both sword and shield
		BasicAttack->Compatibility.RequiredWeapons = { EKDWeaponType::OneHandedSword, EKDWeaponType::Shield };
		Package.Add(BasicAttack);
	}

	// --- 2. Combo Light ---
	// A multi-hit combo sequence activated from the hotbar.
	// Plays a montage with linked sections (Attack_0, Attack_1, Attack_2).
	// Gameplay role: primary hotbar DPS skill, the go-to combat ability.
	{
		UKDAbilityDefinition* ComboLight = CreateBladeShieldSkill(Outer, TEXT("DA_BladeShield_ComboLight"));
		ComboLight->DisplayName = FText::FromString(TEXT("Blade Flurry"));
		ComboLight->Description = FText::FromString(TEXT("A rapid three-hit combo sequence. The primary combat skill for Blade & Shield warriors."));
		ComboLight->DefinitionType = EKDAbilityDefinitionType::ComboSkill;
		ComboLight->SkillTier = EKDSkillTier::Common;
		ComboLight->AbilityTag = FKDGameplayTags::Skill_BladeShield_ComboLight.IsValid()
			? FKDGameplayTags::Skill_BladeShield_ComboLight
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.BladeShield.ComboLight")));
		ComboLight->StaminaCost = 12.f;
		ComboLight->ManaCost = 0.f;
		ComboLight->CooldownDuration = 0.f;
		ComboLight->bCommitOnActivate = true;
		ComboLight->bEndOnMontageComplete = true;
		ComboLight->MontagePlayRate = 1.f;
		ComboLight->bRequiresTarget = true;
		ComboLight->MinRange = 0.f;
		ComboLight->MaxRange = 150.f;
		ComboLight->bAutoFaceTarget = true;
		ComboLight->FacingInterpSpeed = 0.f;
		ComboLight->bHotbarAssignable = true;
		// Combo requires both sword and shield
		ComboLight->Compatibility.RequiredWeapons = { EKDWeaponType::OneHandedSword, EKDWeaponType::Shield };
		Package.Add(ComboLight);
	}

	// --- 3. Heavy Strike (Guard Break) ---
	// A powerful overhead strike that breaks through enemy guards.
	// Higher stamina cost, 8-second cooldown.
	// Gameplay role: burst damage + guard break utility for tactical play.
	{
		UKDAbilityDefinition* HeavyStrike = CreateBladeShieldSkill(Outer, TEXT("DA_BladeShield_HeavyStrike"));
		HeavyStrike->DisplayName = FText::FromString(TEXT("Guard Break"));
		HeavyStrike->Description = FText::FromString(TEXT("A powerful overhead strike that breaks through enemy guards. Higher damage but slow execution."));
		HeavyStrike->DefinitionType = EKDAbilityDefinitionType::ActiveSkill;
		HeavyStrike->SkillTier = EKDSkillTier::Advanced;
		HeavyStrike->AbilityTag = FKDGameplayTags::Skill_BladeShield_HeavyStrike.IsValid()
			? FKDGameplayTags::Skill_BladeShield_HeavyStrike
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.BladeShield.HeavyStrike")));
		HeavyStrike->StaminaCost = 20.f;
		HeavyStrike->ManaCost = 0.f;
		HeavyStrike->CooldownDuration = 8.f;
		HeavyStrike->bCommitOnActivate = true;
		HeavyStrike->bEndOnMontageComplete = true;
		HeavyStrike->MontagePlayRate = 0.85f; // slightly slower
		HeavyStrike->bRequiresTarget = true;
		HeavyStrike->MinRange = 0.f;
		HeavyStrike->MaxRange = 180.f; // slightly longer reach
		HeavyStrike->bAutoFaceTarget = true;
		HeavyStrike->FacingInterpSpeed = 0.f;
		HeavyStrike->bHotbarAssignable = true;
		// Heavy strike only requires sword (can do without shield)
		HeavyStrike->Compatibility.RequiredWeapons = { EKDWeaponType::OneHandedSword };
		Package.Add(HeavyStrike);
	}

	// --- 4. Shield Bash ---
	// Defensive shield-focused skill. Bashes target with shield for damage + brief stun.
	// 12-second cooldown, moderate stamina.
	// Gameplay role: defensive utility, interrupts enemy attacks, creates openings.
	{
		UKDAbilityDefinition* ShieldBash = CreateBladeShieldSkill(Outer, TEXT("DA_BladeShield_ShieldBash"));
		ShieldBash->DisplayName = FText::FromString(TEXT("Shield Bash"));
		ShieldBash->Description = FText::FromString(TEXT("Slams the shield into the target, dealing damage and briefly stunning them. A key defensive tool."));
		ShieldBash->DefinitionType = EKDAbilityDefinitionType::ActiveSkill;
		ShieldBash->SkillTier = EKDSkillTier::Common;
		ShieldBash->AbilityTag = FKDGameplayTags::Skill_BladeShield_ShieldBash.IsValid()
			? FKDGameplayTags::Skill_BladeShield_ShieldBash
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.BladeShield.ShieldBash")));
		ShieldBash->StaminaCost = 15.f;
		ShieldBash->ManaCost = 0.f;
		ShieldBash->CooldownDuration = 12.f;
		ShieldBash->bCommitOnActivate = true;
		ShieldBash->bEndOnMontageComplete = true;
		ShieldBash->MontagePlayRate = 1.1f; // slightly faster
		ShieldBash->bRequiresTarget = true;
		ShieldBash->MinRange = 0.f;
		ShieldBash->MaxRange = 120.f; // shorter range, in-your-face
		ShieldBash->bAutoFaceTarget = true;
		ShieldBash->FacingInterpSpeed = 0.f;
		ShieldBash->bHotbarAssignable = true;
		// Shield bash strictly requires shield
		ShieldBash->Compatibility.RequiredWeapons = { EKDWeaponType::Shield };
		Package.Add(ShieldBash);
	}

	return Package;
}

UKDAbilityDefinition* UKDSkillDefinitionLibrary::LoadDefinitionByPath(const FSoftObjectPath& AssetPath)
{
	return Cast<UKDAbilityDefinition>(AssetPath.TryLoad());
}

TArray<UKDAbilityDefinition*> UKDSkillDefinitionLibrary::LoadBladeShieldPackageFromAssets()
{
	TArray<UKDAbilityDefinition*> Package;
	const FString BasePath = TEXT("/Game/Combat/Skills/BladeShield/");

	const TArray<FString> AssetNames = {
		TEXT("DA_BladeShield_BasicAttack"),
		TEXT("DA_BladeShield_ComboLight"),
		TEXT("DA_BladeShield_HeavyStrike"),
		TEXT("DA_BladeShield_ShieldBash")
	};

	for (const FString& AssetName : AssetNames)
	{
		const FString ObjectPath = BasePath + AssetName + TEXT(".") + AssetName;
		if (UKDAbilityDefinition* Definition = LoadDefinitionByPath(FSoftObjectPath(ObjectPath)))
		{
			Package.Add(Definition);
		}
	}

	return Package;
}

TArray<UKDAbilityDefinition*> UKDSkillDefinitionLibrary::CreateWizardPackage(UObject* Outer)
{
	TArray<UKDAbilityDefinition*> Package;

	// --- 1. Arcane Bolt ---
	// Basic ranged nuke. Fast cast, instant hit, low mana cost.
	// Gameplay role: bread-and-butter spell for damage rotation.
	{
		UKDAbilityDefinition* ArcaneBolt = CreateWizardSkill(Outer, TEXT("DA_Wizard_ArcaneBolt"));
		ArcaneBolt->DisplayName = FText::FromString(TEXT("Arcane Bolt"));
		ArcaneBolt->Description = FText::FromString(TEXT("A quick bolt of arcane energy. The basic ranged attack for Wizards."));
		ArcaneBolt->DefinitionType = EKDAbilityDefinitionType::ActiveSkill;
		ArcaneBolt->SkillTier = EKDSkillTier::Common;
		ArcaneBolt->AbilityTag = FKDGameplayTags::Skill_Wizard_ArcaneBolt.IsValid()
			? FKDGameplayTags::Skill_Wizard_ArcaneBolt
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.Wizard.ArcaneBolt")));
		ArcaneBolt->StaminaCost = 0.f;
		ArcaneBolt->ManaCost = 10.f;
		ArcaneBolt->CooldownDuration = 0.f;
		ArcaneBolt->bCommitOnActivate = true;
		ArcaneBolt->bEndOnMontageComplete = true;
		ArcaneBolt->MontagePlayRate = 1.f;
		ArcaneBolt->bRequiresTarget = true;
		ArcaneBolt->MinRange = 100.f;
		ArcaneBolt->MaxRange = 1500.f;
		ArcaneBolt->bAutoFaceTarget = true;
		ArcaneBolt->FacingInterpSpeed = 0.f;
		ArcaneBolt->bHotbarAssignable = true;
		Package.Add(ArcaneBolt);
	}

	// --- 2. Fireball ---
	// Ranged projectile with higher damage. Slower travel time, higher mana cost.
	// Gameplay role: burst damage spell, hits harder than Arcane Bolt but slower.
	{
		UKDAbilityDefinition* Fireball = CreateWizardSkill(Outer, TEXT("DA_Wizard_Fireball"));
		Fireball->DisplayName = FText::FromString(TEXT("Fireball"));
		Fireball->Description = FText::FromString(TEXT("Hurls a ball of fire that explodes on impact. Higher damage but slower cast time."));
		Fireball->DefinitionType = EKDAbilityDefinitionType::ActiveSkill;
		Fireball->SkillTier = EKDSkillTier::Common;
		Fireball->AbilityTag = FKDGameplayTags::Skill_Wizard_Fireball.IsValid()
			? FKDGameplayTags::Skill_Wizard_Fireball
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.Wizard.Fireball")));
		Fireball->StaminaCost = 0.f;
		Fireball->ManaCost = 25.f;
		Fireball->CooldownDuration = 3.f;
		Fireball->bCommitOnActivate = true;
		Fireball->bEndOnMontageComplete = true;
		Fireball->MontagePlayRate = 0.9f;
		Fireball->bRequiresTarget = true;
		Fireball->MinRange = 100.f;
		Fireball->MaxRange = 1500.f;
		Fireball->bAutoFaceTarget = true;
		Fireball->FacingInterpSpeed = 0.f;
		Fireball->bHotbarAssignable = true;
		Package.Add(Fireball);
	}

	// --- 3. Frost Nova ---
	// AoE slow/freeze around caster. CC utility spell.
	// Gameplay role: crowd control, creates distance, disables nearby enemies.
	{
		UKDAbilityDefinition* FrostNova = CreateWizardSkill(Outer, TEXT("DA_Wizard_FrostNova"));
		FrostNova->DisplayName = FText::FromString(TEXT("Frost Nova"));
		FrostNova->Description = FText::FromString(TEXT("Unleashes a freezing wave around the caster, slowing and damaging nearby enemies."));
		FrostNova->DefinitionType = EKDAbilityDefinitionType::ActiveSkill;
		FrostNova->SkillTier = EKDSkillTier::Advanced;
		FrostNova->AbilityTag = FKDGameplayTags::Skill_Wizard_FrostNova.IsValid()
			? FKDGameplayTags::Skill_Wizard_FrostNova
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.Wizard.FrostNova")));
		FrostNova->StaminaCost = 0.f;
		FrostNova->ManaCost = 30.f;
		FrostNova->CooldownDuration = 12.f;
		FrostNova->bCommitOnActivate = true;
		FrostNova->bEndOnMontageComplete = true;
		FrostNova->MontagePlayRate = 1.f;
		FrostNova->bRequiresTarget = false; // AoE, no target required
		FrostNova->MinRange = 0.f;
		FrostNova->MaxRange = 500.f; // Self-centered radius
		FrostNova->bAutoFaceTarget = false;
		FrostNova->bHotbarAssignable = true;
		Package.Add(FrostNova);
	}

	// --- 4. Mana Shield ---
	// Self-buff that absorbs damage using mana pool.
	// Gameplay role: defensive cooldown, trades mana for health preservation.
	{
		UKDAbilityDefinition* ManaShield = CreateWizardSkill(Outer, TEXT("DA_Wizard_ManaShield"));
		ManaShield->DisplayName = FText::FromString(TEXT("Mana Shield"));
		ManaShield->Description = FText::FromString(TEXT("Surrounds the caster with a magical shield that absorbs damage using mana instead of health."));
		ManaShield->DefinitionType = EKDAbilityDefinitionType::BuffSkill;
		ManaShield->SkillTier = EKDSkillTier::Advanced;
		ManaShield->AbilityTag = FKDGameplayTags::Skill_Wizard_ManaShield.IsValid()
			? FKDGameplayTags::Skill_Wizard_ManaShield
			: FGameplayTag::RequestGameplayTag(FName(TEXT("KD.Skill.Wizard.ManaShield")));
		ManaShield->StaminaCost = 0.f;
		ManaShield->ManaCost = 40.f; // Higher cost for buff
		ManaShield->CooldownDuration = 30.f;
		ManaShield->bCommitOnActivate = true;
		ManaShield->bEndOnMontageComplete = true;
		ManaShield->MontagePlayRate = 1.f;
		ManaShield->bRequiresTarget = false; // Self-cast
		ManaShield->MinRange = 0.f;
		ManaShield->MaxRange = 0.f;
		ManaShield->bAutoFaceTarget = false;
		ManaShield->bHotbarAssignable = true;
		Package.Add(ManaShield);
	}

	return Package;
}

TArray<UKDAbilityDefinition*> UKDSkillDefinitionLibrary::LoadWizardPackageFromAssets()
{
	TArray<UKDAbilityDefinition*> Package;
	const FString BasePath = TEXT("/Game/Combat/Skills/Wizard/");

	const TArray<FString> AssetNames = {
		TEXT("DA_Wizard_ArcaneBolt"),
		TEXT("DA_Wizard_Fireball"),
		TEXT("DA_Wizard_FrostNova"),
		TEXT("DA_Wizard_ManaShield")
	};

	for (const FString& AssetName : AssetNames)
	{
		const FString ObjectPath = BasePath + AssetName + TEXT(".") + AssetName;
		if (UKDAbilityDefinition* Definition = LoadDefinitionByPath(FSoftObjectPath(ObjectPath)))
		{
			Package.Add(Definition);
		}
	}

	return Package;
}

// --- Query Helpers ---

UKDAbilityDefinition* UKDSkillDefinitionLibrary::FindDefinitionByTag(const TArray<UKDAbilityDefinition*>& Definitions, FGameplayTag AbilityTag)
{
	for (UKDAbilityDefinition* Def : Definitions)
	{
		if (Def && Def->AbilityTag == AbilityTag)
		{
			return Def;
		}
	}
	return nullptr;
}

bool UKDSkillDefinitionLibrary::IsSkillCompatible(const UKDAbilityDefinition* Definition, EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon)
{
	if (!Definition)
	{
		return false;
	}
	return Definition->IsCompatibleWith(Class, Branch, Weapon);
}

TArray<UKDAbilityDefinition*> UKDSkillDefinitionLibrary::FilterCompatibleSkills(const TArray<UKDAbilityDefinition*>& Definitions, EKDCombatClass Class, EKDCombatBranch Branch, EKDWeaponType Weapon)
{
	TArray<UKDAbilityDefinition*> Result;
	for (UKDAbilityDefinition* Def : Definitions)
	{
		if (Def && Def->IsCompatibleWith(Class, Branch, Weapon))
		{
			Result.Add(Def);
		}
	}
	return Result;
}

// --- Mastery-Aware Compatibility Queries ---

bool UKDSkillDefinitionLibrary::IsSkillCompatibleByMastery(const UKDAbilityDefinition* Definition, FKDMasteryIdentity MasteryId, EKDWeaponType Weapon)
{
	if (!Definition)
	{
		return false;
	}
	return Definition->IsCompatibleWithMastery(MasteryId, Weapon);
}

TArray<UKDAbilityDefinition*> UKDSkillDefinitionLibrary::FilterCompatibleSkillsByMastery(const TArray<UKDAbilityDefinition*>& Definitions, FKDMasteryIdentity MasteryId, EKDWeaponType Weapon)
{
	TArray<UKDAbilityDefinition*> Result;
	for (UKDAbilityDefinition* Def : Definitions)
	{
		if (Def && Def->IsCompatibleWithMastery(MasteryId, Weapon))
		{
			Result.Add(Def);
		}
	}
	return Result;
}
