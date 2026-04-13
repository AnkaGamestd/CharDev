// Copyright Kingdawn. All Rights Reserved.

#include "Core/KDGameplayTags.h"
#include "GameplayTagsManager.h"

bool FKDGameplayTags::bTagsRegistered = false;

// -- Combat State Tags --
FGameplayTag FKDGameplayTags::Combat_State_Idle;
FGameplayTag FKDGameplayTags::Combat_State_Attacking;
FGameplayTag FKDGameplayTags::Combat_State_Blocking;
FGameplayTag FKDGameplayTags::Combat_State_Dodging;
FGameplayTag FKDGameplayTags::Combat_State_HitReacting;
FGameplayTag FKDGameplayTags::Combat_State_Dead;

// -- Combat Action Tags --
FGameplayTag FKDGameplayTags::Combat_Action_Attack_Basic;
FGameplayTag FKDGameplayTags::Combat_Action_Attack_Heavy;
FGameplayTag FKDGameplayTags::Combat_Action_Block;

// -- Combat Skill Tags --
FGameplayTag FKDGameplayTags::Combat_Skill_Combo_Light;

// -- BladeShield Branch Skill Tags --
FGameplayTag FKDGameplayTags::Skill_BladeShield_BasicAttack;
FGameplayTag FKDGameplayTags::Skill_BladeShield_ComboLight;
FGameplayTag FKDGameplayTags::Skill_BladeShield_HeavyStrike;
FGameplayTag FKDGameplayTags::Skill_BladeShield_ShieldBash;

// -- Wizard Branch Skill Tags --
FGameplayTag FKDGameplayTags::Skill_Wizard_ArcaneBolt;
FGameplayTag FKDGameplayTags::Skill_Wizard_Fireball;
FGameplayTag FKDGameplayTags::Skill_Wizard_FrostNova;
FGameplayTag FKDGameplayTags::Skill_Wizard_ManaShield;

FGameplayTag FKDGameplayTags::Combat_Action_Dodge;
FGameplayTag FKDGameplayTags::Combat_Action_Death;

// -- Combo Tags --
FGameplayTag FKDGameplayTags::Combat_Combo_1;
FGameplayTag FKDGameplayTags::Combat_Combo_2;
FGameplayTag FKDGameplayTags::Combat_Combo_3;

// -- Weapon Type Tags --
FGameplayTag FKDGameplayTags::Weapon_Type_Unarmed;
FGameplayTag FKDGameplayTags::Weapon_Type_Sword;
FGameplayTag FKDGameplayTags::Weapon_Type_Staff;

// -- Class Family Tags --
FGameplayTag FKDGameplayTags::Class_Warrior;
FGameplayTag FKDGameplayTags::Class_Mage;
FGameplayTag FKDGameplayTags::Class_Rogue;
FGameplayTag FKDGameplayTags::Class_Archer;
FGameplayTag FKDGameplayTags::Class_Support;

// -- Branch Tags (Warrior) --
FGameplayTag FKDGameplayTags::Branch_BladeShield;
FGameplayTag FKDGameplayTags::Branch_TwoHandedSword;
FGameplayTag FKDGameplayTags::Branch_Hammer;
FGameplayTag FKDGameplayTags::Branch_DualAxe;
FGameplayTag FKDGameplayTags::Branch_Glaive;
FGameplayTag FKDGameplayTags::Branch_SpearScythe;

// -- Branch Tags (Mage) --
FGameplayTag FKDGameplayTags::Branch_Wizard;
FGameplayTag FKDGameplayTags::Branch_Warlock;
FGameplayTag FKDGameplayTags::Branch_Necromancer;

// -- Branch Tags (Support) --
FGameplayTag FKDGameplayTags::Branch_Cleric;
FGameplayTag FKDGameplayTags::Branch_Buffer;

// -- Branch Tags (Rogue) --
FGameplayTag FKDGameplayTags::Branch_Dagger;

// -- Branch Tags (Archer) --
FGameplayTag FKDGameplayTags::Branch_Archer;

// -- Damage Type Tags --
FGameplayTag FKDGameplayTags::Damage_Type_Physical;
FGameplayTag FKDGameplayTags::Damage_Type_Magic;

// -- Team Tags --
FGameplayTag FKDGameplayTags::Team_Player;
FGameplayTag FKDGameplayTags::Team_Enemy;
FGameplayTag FKDGameplayTags::Team_Neutral;

// -- Hit Direction Tags --
FGameplayTag FKDGameplayTags::Hit_Direction_Front;
FGameplayTag FKDGameplayTags::Hit_Direction_Back;
FGameplayTag FKDGameplayTags::Hit_Direction_Left;
FGameplayTag FKDGameplayTags::Hit_Direction_Right;

// -- Status Effect Tags --
FGameplayTag FKDGameplayTags::Status_Effect_Slow;
FGameplayTag FKDGameplayTags::Status_Effect_Freeze;
FGameplayTag FKDGameplayTags::Status_Effect_Stun;
FGameplayTag FKDGameplayTags::Status_Effect_Silence;
FGameplayTag FKDGameplayTags::Status_Effect_Knockdown;
FGameplayTag FKDGameplayTags::Status_Effect_Bleed;

// -- Immunity Tags --
FGameplayTag FKDGameplayTags::Immunity_Knockdown;
FGameplayTag FKDGameplayTags::Immunity_Stun;
FGameplayTag FKDGameplayTags::Immunity_Slow;
FGameplayTag FKDGameplayTags::Immunity_Freeze;

// -- Enemy Classification Tags --
FGameplayTag FKDGameplayTags::Enemy_Class_Boss;
FGameplayTag FKDGameplayTags::Enemy_Class_Giant;
FGameplayTag FKDGameplayTags::Enemy_Class_EventMob;

// -- SetByCaller Data Tags --
FGameplayTag FKDGameplayTags::Data_Duration;
FGameplayTag FKDGameplayTags::Data_Magnitude;

void FKDGameplayTags::InitTags()
{
	if (bTagsRegistered)
	{
		return;
	}

	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

	// Combat States
	Combat_State_Idle = TagManager.AddNativeGameplayTag(FName("KD.Combat.State.Idle"), FString("Character is idle"));
	Combat_State_Attacking = TagManager.AddNativeGameplayTag(FName("KD.Combat.State.Attacking"), FString("Character is attacking"));
	Combat_State_Blocking = TagManager.AddNativeGameplayTag(FName("KD.Combat.State.Blocking"), FString("Character is blocking"));
	Combat_State_Dodging = TagManager.AddNativeGameplayTag(FName("KD.Combat.State.Dodging"), FString("Character is dodging"));
	Combat_State_HitReacting = TagManager.AddNativeGameplayTag(FName("KD.Combat.State.HitReacting"), FString("Character is reacting to hit"));
	Combat_State_Dead = TagManager.AddNativeGameplayTag(FName("KD.Combat.State.Dead"), FString("Character is dead"));

	// Combat Actions
	Combat_Action_Attack_Basic = TagManager.AddNativeGameplayTag(FName("KD.Combat.Action.Attack.Basic"), FString("Basic attack (mouse double-click)"));
	Combat_Action_Attack_Heavy = TagManager.AddNativeGameplayTag(FName("KD.Combat.Action.Attack.Heavy"), FString("Heavy attack action"));
	Combat_Action_Block = TagManager.AddNativeGameplayTag(FName("KD.Combat.Action.Block"), FString("Block action"));
	
	// Combat Skills
	Combat_Skill_Combo_Light = TagManager.AddNativeGameplayTag(FName("KD.Combat.Skill.Combo.Light"), FString("Light combo skill (hotbar, legacy)"));

	// BladeShield Branch Skills
	Skill_BladeShield_BasicAttack = TagManager.AddNativeGameplayTag(FName("KD.Skill.BladeShield.BasicAttack"), FString("Blade & Shield Basic Attack"));
	Skill_BladeShield_ComboLight = TagManager.AddNativeGameplayTag(FName("KD.Skill.BladeShield.ComboLight"), FString("Blade & Shield Combo Light"));
	Skill_BladeShield_HeavyStrike = TagManager.AddNativeGameplayTag(FName("KD.Skill.BladeShield.HeavyStrike"), FString("Blade & Shield Heavy Strike"));
	Skill_BladeShield_ShieldBash = TagManager.AddNativeGameplayTag(FName("KD.Skill.BladeShield.ShieldBash"), FString("Blade & Shield Shield Bash"));

	// Wizard Branch Skills
	Skill_Wizard_ArcaneBolt = TagManager.AddNativeGameplayTag(FName("KD.Skill.Wizard.ArcaneBolt"), FString("Wizard Arcane Bolt"));
	Skill_Wizard_Fireball = TagManager.AddNativeGameplayTag(FName("KD.Skill.Wizard.Fireball"), FString("Wizard Fireball"));
	Skill_Wizard_FrostNova = TagManager.AddNativeGameplayTag(FName("KD.Skill.Wizard.FrostNova"), FString("Wizard Frost Nova"));
	Skill_Wizard_ManaShield = TagManager.AddNativeGameplayTag(FName("KD.Skill.Wizard.ManaShield"), FString("Wizard Mana Shield"));

	Combat_Action_Dodge = TagManager.AddNativeGameplayTag(FName("KD.Combat.Action.Dodge"), FString("Dodge action"));
	Combat_Action_Death = TagManager.AddNativeGameplayTag(FName("KD.Combat.Action.Death"), FString("Death action"));

	// Combo
	Combat_Combo_1 = TagManager.AddNativeGameplayTag(FName("KD.Combat.Combo.1"), FString("First combo hit"));
	Combat_Combo_2 = TagManager.AddNativeGameplayTag(FName("KD.Combat.Combo.2"), FString("Second combo hit"));
	Combat_Combo_3 = TagManager.AddNativeGameplayTag(FName("KD.Combat.Combo.3"), FString("Third combo hit"));

	// Weapon Types
	Weapon_Type_Unarmed = TagManager.AddNativeGameplayTag(FName("KD.Weapon.Type.Unarmed"), FString("Unarmed combat"));
	Weapon_Type_Sword = TagManager.AddNativeGameplayTag(FName("KD.Weapon.Type.Sword"), FString("Sword weapon type"));
	Weapon_Type_Staff = TagManager.AddNativeGameplayTag(FName("KD.Weapon.Type.Staff"), FString("Staff weapon type"));

	// Class Families
	Class_Warrior = TagManager.AddNativeGameplayTag(FName("KD.Class.Warrior"), FString("Warrior class family"));
	Class_Mage = TagManager.AddNativeGameplayTag(FName("KD.Class.Mage"), FString("Mage class family"));
	Class_Rogue = TagManager.AddNativeGameplayTag(FName("KD.Class.Rogue"), FString("Rogue class family"));
	Class_Archer = TagManager.AddNativeGameplayTag(FName("KD.Class.Archer"), FString("Archer class family"));
	Class_Support = TagManager.AddNativeGameplayTag(FName("KD.Class.Support"), FString("Support class family"));

	// Branches (Warrior)
	Branch_BladeShield = TagManager.AddNativeGameplayTag(FName("KD.Branch.BladeShield"), FString("Blade & Shield branch"));
	Branch_TwoHandedSword = TagManager.AddNativeGameplayTag(FName("KD.Branch.TwoHandedSword"), FString("Two-Handed Sword branch"));
	Branch_Hammer = TagManager.AddNativeGameplayTag(FName("KD.Branch.Hammer"), FString("Hammer branch"));
	Branch_DualAxe = TagManager.AddNativeGameplayTag(FName("KD.Branch.DualAxe"), FString("Dual Axe branch"));
	Branch_Glaive = TagManager.AddNativeGameplayTag(FName("KD.Branch.Glaive"), FString("Glaive branch"));
	Branch_SpearScythe = TagManager.AddNativeGameplayTag(FName("KD.Branch.SpearScythe"), FString("Spear/Scythe branch"));

	// Branches (Mage)
	Branch_Wizard = TagManager.AddNativeGameplayTag(FName("KD.Branch.Wizard"), FString("Wizard branch"));
	Branch_Warlock = TagManager.AddNativeGameplayTag(FName("KD.Branch.Warlock"), FString("Warlock branch"));
	Branch_Necromancer = TagManager.AddNativeGameplayTag(FName("KD.Branch.Necromancer"), FString("Necromancer branch"));

	// Branches (Support)
	Branch_Cleric = TagManager.AddNativeGameplayTag(FName("KD.Branch.Cleric"), FString("Cleric branch"));
	Branch_Buffer = TagManager.AddNativeGameplayTag(FName("KD.Branch.Buffer"), FString("Buffer branch"));

	// Branches (Rogue)
	Branch_Dagger = TagManager.AddNativeGameplayTag(FName("KD.Branch.Dagger"), FString("Dagger branch"));

	// Branches (Archer)
	Branch_Archer = TagManager.AddNativeGameplayTag(FName("KD.Branch.Archer"), FString("Archer branch"));

	// Damage Types
	Damage_Type_Physical = TagManager.AddNativeGameplayTag(FName("KD.Damage.Type.Physical"), FString("Physical damage"));
	Damage_Type_Magic = TagManager.AddNativeGameplayTag(FName("KD.Damage.Type.Magic"), FString("Magic damage"));

	// Teams
	Team_Player = TagManager.AddNativeGameplayTag(FName("KD.Team.Player"), FString("Player team"));
	Team_Enemy = TagManager.AddNativeGameplayTag(FName("KD.Team.Enemy"), FString("Enemy team"));
	Team_Neutral = TagManager.AddNativeGameplayTag(FName("KD.Team.Neutral"), FString("Neutral team"));

	// Hit Directions
	Hit_Direction_Front = TagManager.AddNativeGameplayTag(FName("KD.Hit.Direction.Front"), FString("Hit from front"));
	Hit_Direction_Back = TagManager.AddNativeGameplayTag(FName("KD.Hit.Direction.Back"), FString("Hit from back"));
	Hit_Direction_Left = TagManager.AddNativeGameplayTag(FName("KD.Hit.Direction.Left"), FString("Hit from left"));
	Hit_Direction_Right = TagManager.AddNativeGameplayTag(FName("KD.Hit.Direction.Right"), FString("Hit from right"));

	// Status Effects (for FrostNova and future CC)
	Status_Effect_Slow = TagManager.AddNativeGameplayTag(FName("KD.Status.Effect.Slow"), FString("Movement speed reduced"));
	Status_Effect_Freeze = TagManager.AddNativeGameplayTag(FName("KD.Status.Effect.Freeze"), FString("Frozen in place, cannot act"));
	Status_Effect_Stun = TagManager.AddNativeGameplayTag(FName("KD.Status.Effect.Stun"), FString("Stunned, cannot act"));
	Status_Effect_Silence = TagManager.AddNativeGameplayTag(FName("KD.Status.Effect.Silence"), FString("Cannot cast spells"));
	Status_Effect_Knockdown = TagManager.AddNativeGameplayTag(FName("KD.Status.Effect.Knockdown"), FString("Knocked down"));
	Status_Effect_Bleed = TagManager.AddNativeGameplayTag(FName("KD.Status.Effect.Bleed"), FString("Damage over time"));

	// Immunity Tags (grant to entities immune to specific effects)
	Immunity_Knockdown = TagManager.AddNativeGameplayTag(FName("KD.Immunity.Knockdown"), FString("Immune to knockdown"));
	Immunity_Stun = TagManager.AddNativeGameplayTag(FName("KD.Immunity.Stun"), FString("Immune to stun"));
	Immunity_Slow = TagManager.AddNativeGameplayTag(FName("KD.Immunity.Slow"), FString("Immune to slow"));
	Immunity_Freeze = TagManager.AddNativeGameplayTag(FName("KD.Immunity.Freeze"), FString("Immune to freeze"));

	// Enemy Classification (grant to enemies for immunity rules)
	Enemy_Class_Boss = TagManager.AddNativeGameplayTag(FName("KD.Enemy.Class.Boss"), FString("Boss enemy"));
	Enemy_Class_Giant = TagManager.AddNativeGameplayTag(FName("KD.Enemy.Class.Giant"), FString("Giant enemy"));
	Enemy_Class_EventMob = TagManager.AddNativeGameplayTag(FName("KD.Enemy.Class.EventMob"), FString("Event mob enemy"));

	// SetByCaller Data Tags (used by GE specs for duration/magnitude)
	Data_Duration = TagManager.AddNativeGameplayTag(FName("KD.Data.Duration"), FString("SetByCaller duration parameter"));
	Data_Magnitude = TagManager.AddNativeGameplayTag(FName("KD.Data.Magnitude"), FString("SetByCaller magnitude parameter"));

	bTagsRegistered = true;
}




