#include "Items/TainlordItemRuleLibrary.h"

FTainlordItemDefinition UTainlordItemRuleLibrary::MakeStarterWeaponDefinition(const ETainlordArchetype Archetype)
{
	FTainlordItemDefinition Item;
	Item.RequiredLevel = 1;
	Item.EquipmentSlot = ETainlordEquipmentSlot::Weapon;
	Item.Tier = ETainlordItemTier::Common;
	Item.GemSlotCount = 0;

	switch (Archetype)
	{
	case ETainlordArchetype::Warrior:
		Item.ItemId = "starter_warrior_blade";
		Item.DisplayName = NSLOCTEXT("Tainlord", "StarterWarriorBlade", "Recruit Blade");
		Item.GrantedStats.AttackPower = 6;
		Item.GrantedStats.Strength = 2;
		break;
	case ETainlordArchetype::Mage:
		Item.ItemId = "starter_mage_focus";
		Item.DisplayName = NSLOCTEXT("Tainlord", "StarterMageFocus", "Apprentice Focus");
		Item.GrantedStats.SpellPower = 8;
		Item.GrantedStats.Intelligence = 2;
		break;
	case ETainlordArchetype::Cleric:
		Item.ItemId = "starter_cleric_mace";
		Item.DisplayName = NSLOCTEXT("Tainlord", "StarterClericMace", "Sanctified Mace");
		Item.GrantedStats.SpellPower = 4;
		Item.GrantedStats.Willpower = 2;
		Item.GrantedStats.Defense = 2;
		break;
	default:
		Item.ItemId = "starter_generic_weapon";
		Item.DisplayName = NSLOCTEXT("Tainlord", "StarterGenericWeapon", "Starter Weapon");
		Item.GrantedStats.AttackPower = 4;
		break;
	}

	return Item;
}

bool UTainlordItemRuleLibrary::CanEquipItem(const FTainlordCharacterState& CharacterState, const FTainlordItemDefinition& ItemDefinition)
{
	return CharacterState.Progression.Level >= ItemDefinition.RequiredLevel;
}

FTainlordEquipmentLoadout UTainlordItemRuleLibrary::EquipItem(const FTainlordEquipmentLoadout& CurrentLoadout, const FTainlordInventoryEntry& InventoryEntry)
{
	FTainlordEquipmentLoadout Result = CurrentLoadout;
	Result.EquippedItems.FindOrAdd(InventoryEntry.Item.EquipmentSlot) = InventoryEntry;
	return Result;
}
