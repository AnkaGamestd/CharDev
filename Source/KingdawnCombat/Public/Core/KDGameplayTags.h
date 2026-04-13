// Copyright Kingdawn. All Rights Reserved.
// Combat gameplay tags for the Kingdawn combat system.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Central registry of all gameplay tags used by the Kingdawn combat system.
 * Call UKDGameplayTags::InitTags() during module startup.
 */
struct KINGDAWNCOMBAT_API FKDGameplayTags
{
	// -- Combat State Tags --
	static FGameplayTag Combat_State_Idle;
	static FGameplayTag Combat_State_Attacking;
	static FGameplayTag Combat_State_Blocking;
	static FGameplayTag Combat_State_Dodging;
	static FGameplayTag Combat_State_HitReacting;
	static FGameplayTag Combat_State_Dead;

	// -- Combat Action Tags --
	static FGameplayTag Combat_Action_Attack_Basic;    // Mouse double-click basic attack
	static FGameplayTag Combat_Action_Attack_Heavy;
	static FGameplayTag Combat_Action_Block;
	
	// -- Combat Skill Tags --
	static FGameplayTag Combat_Skill_Combo_Light;      // Hotbar combo skill (legacy, use branch-specific tags)

	// -- BladeShield Branch Skill Tags --
	static FGameplayTag Skill_BladeShield_BasicAttack;
	static FGameplayTag Skill_BladeShield_ComboLight;
	static FGameplayTag Skill_BladeShield_HeavyStrike;
	static FGameplayTag Skill_BladeShield_ShieldBash;

	// -- Wizard Branch Skill Tags --
	static FGameplayTag Skill_Wizard_ArcaneBolt;
	static FGameplayTag Skill_Wizard_Fireball;
	static FGameplayTag Skill_Wizard_FrostNova;
	static FGameplayTag Skill_Wizard_ManaShield;

	static FGameplayTag Combat_Action_Dodge;
	static FGameplayTag Combat_Action_Death;

	// -- Combo Tags --
	static FGameplayTag Combat_Combo_1;
	static FGameplayTag Combat_Combo_2;
	static FGameplayTag Combat_Combo_3;

	// -- Weapon Type Tags --
	static FGameplayTag Weapon_Type_Unarmed;
	static FGameplayTag Weapon_Type_Sword;
	static FGameplayTag Weapon_Type_Staff;

	// -- Class Family Tags --
	static FGameplayTag Class_Warrior;
	static FGameplayTag Class_Mage;
	static FGameplayTag Class_Rogue;
	static FGameplayTag Class_Archer;
	static FGameplayTag Class_Support;

	// -- Branch Tags (Warrior) --
	static FGameplayTag Branch_BladeShield;
	static FGameplayTag Branch_TwoHandedSword;
	static FGameplayTag Branch_Hammer;
	static FGameplayTag Branch_DualAxe;
	static FGameplayTag Branch_Glaive;
	static FGameplayTag Branch_SpearScythe;

	// -- Branch Tags (Mage) --
	static FGameplayTag Branch_Wizard;
	static FGameplayTag Branch_Warlock;
	static FGameplayTag Branch_Necromancer;

	// -- Branch Tags (Support) --
	static FGameplayTag Branch_Cleric;
	static FGameplayTag Branch_Buffer;

	// -- Branch Tags (Rogue) --
	static FGameplayTag Branch_Dagger;

	// -- Branch Tags (Archer) --
	static FGameplayTag Branch_Archer;

	// -- Damage Type Tags --
	static FGameplayTag Damage_Type_Physical;
	static FGameplayTag Damage_Type_Magic;

	// -- Team Tags --
	static FGameplayTag Team_Player;
	static FGameplayTag Team_Enemy;
	static FGameplayTag Team_Neutral;

	// -- Hit Direction Tags --
	static FGameplayTag Hit_Direction_Front;
	static FGameplayTag Hit_Direction_Back;
	static FGameplayTag Hit_Direction_Left;
	static FGameplayTag Hit_Direction_Right;

	// -- Status Effect Tags (for FrostNova and future CC) --
	static FGameplayTag Status_Effect_Slow;
	static FGameplayTag Status_Effect_Freeze;
	static FGameplayTag Status_Effect_Stun;
	static FGameplayTag Status_Effect_Silence;
	static FGameplayTag Status_Effect_Knockdown;
	static FGameplayTag Status_Effect_Bleed;

	// -- Immunity Tags (applied to entities immune to specific CC) --
	static FGameplayTag Immunity_Knockdown;
	static FGameplayTag Immunity_Stun;
	static FGameplayTag Immunity_Slow;
	static FGameplayTag Immunity_Freeze;

	// -- Enemy Classification Tags (for immunity rules) --
	static FGameplayTag Enemy_Class_Boss;
	static FGameplayTag Enemy_Class_Giant;
	static FGameplayTag Enemy_Class_EventMob;

	// -- SetByCaller Data Tags (for GE magnitude/duration) --
	static FGameplayTag Data_Duration;
	static FGameplayTag Data_Magnitude;

	/** Register all tags with the gameplay tag manager. Call once at module startup. */
	static void InitTags();

private:
	static bool bTagsRegistered;
};




