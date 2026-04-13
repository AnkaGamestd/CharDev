#pragma once

#include "CoreMinimal.h"
#include "TainlordGameplayTypes.generated.h"

UENUM(BlueprintType)
enum class ETainlordArchetype : uint8
{
	Warrior UMETA(DisplayName = "Warrior"),
	Mage UMETA(DisplayName = "Mage"),
	Cleric UMETA(DisplayName = "Cleric"),
	Buffer UMETA(DisplayName = "Buffer"),
	Archer UMETA(DisplayName = "Archer"),
	Dagger UMETA(DisplayName = "Dagger"),
	Warlock UMETA(DisplayName = "Warlock")
};

UENUM(BlueprintType)
enum class ETainlordStatOrientation : uint8
{
	Strength UMETA(DisplayName = "Strength"),
	Intelligence UMETA(DisplayName = "Intelligence"),
	Hybrid UMETA(DisplayName = "Hybrid")
};

UENUM(BlueprintType)
enum class ETainlordElement : uint8
{
	Fire UMETA(DisplayName = "Fire"),
	Water UMETA(DisplayName = "Water"),
	Wind UMETA(DisplayName = "Wind"),
	Earth UMETA(DisplayName = "Earth")
};

UENUM(BlueprintType)
enum class ETainlordCurrencyType : uint8
{
	SoftCurrency UMETA(DisplayName = "Soft Currency"),
	Coin UMETA(DisplayName = "Coin")
};

UENUM(BlueprintType)
enum class ETainlordItemTier : uint8
{
	Common UMETA(DisplayName = "Common"),
	Planet UMETA(DisplayName = "Planet"),
	Star UMETA(DisplayName = "Star"),
	Galaxy UMETA(DisplayName = "Galaxy")
};

UENUM(BlueprintType)
enum class ETainlordGemSize : uint8
{
	Mini UMETA(DisplayName = "Mini"),
	Small UMETA(DisplayName = "Small"),
	Medium UMETA(DisplayName = "Medium"),
	Large UMETA(DisplayName = "Large")
};

UENUM(BlueprintType)
enum class ETainlordGemModifierType : uint8
{
	PhysicalPower UMETA(DisplayName = "Physical Power"),
	SpellPower UMETA(DisplayName = "Spell Power"),
	Defense UMETA(DisplayName = "Defense"),
	Vitality UMETA(DisplayName = "Vitality"),
	Willpower UMETA(DisplayName = "Willpower")
};

UENUM(BlueprintType)
enum class ETainlordEquipmentSlot : uint8
{
	Weapon UMETA(DisplayName = "Weapon"),
	Head UMETA(DisplayName = "Head"),
	Chest UMETA(DisplayName = "Chest"),
	Hands UMETA(DisplayName = "Hands"),
	Legs UMETA(DisplayName = "Legs"),
	Feet UMETA(DisplayName = "Feet"),
	Accessory UMETA(DisplayName = "Accessory")
};

UENUM(BlueprintType)
enum class ETainlordReputationAxis : uint8
{
	City UMETA(DisplayName = "City Reputation"),
	Mercenary UMETA(DisplayName = "Mercenary Reputation"),
	Infamy UMETA(DisplayName = "Infamy"),
	Political UMETA(DisplayName = "Political Standing")
};

UENUM(BlueprintType)
enum class ETainlordAllegiance : uint8
{
	City UMETA(DisplayName = "City"),
	Exile UMETA(DisplayName = "Exile")
};

UENUM(BlueprintType)
enum class ETainlordCombatRole : uint8
{
	Frontline UMETA(DisplayName = "Frontline"),
	RangedDamage UMETA(DisplayName = "Ranged Damage"),
	SustainSupport UMETA(DisplayName = "Sustain Support"),
	TempoSupport UMETA(DisplayName = "Tempo Support"),
	Control UMETA(DisplayName = "Control")
};

UENUM(BlueprintType)
enum class ETainlordScalingAttribute : uint8
{
	Strength UMETA(DisplayName = "Strength"),
	Intelligence UMETA(DisplayName = "Intelligence"),
	Vitality UMETA(DisplayName = "Vitality"),
	Willpower UMETA(DisplayName = "Willpower")
};

UENUM(BlueprintType)
enum class ETainlordActionType : uint8
{
	BasicAttack UMETA(DisplayName = "Basic Attack"),
	Skill UMETA(DisplayName = "Skill"),
	Support UMETA(DisplayName = "Support")
};

USTRUCT(BlueprintType)
struct FDomainOwnershipNotes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ownership")
	bool bRulesOwnedInCode = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ownership")
	bool bValuesExpectedFromDataAssets = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ownership")
	bool bBlueprintGlueAllowed = true;
};

USTRUCT(BlueprintType)
struct FTainlordStatBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Strength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Intelligence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Vitality = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Willpower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 AttackPower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 SpellPower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Defense = 0;
};

USTRUCT(BlueprintType)
struct FTainlordElementLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elements")
	ETainlordElement PrimaryElement = ETainlordElement::Fire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elements")
	TArray<ETainlordElement> EquippedElements;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elements")
	int32 MaxEquippedElements = 2;
};

USTRUCT(BlueprintType)
struct FTainlordCurrencyWallet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	int32 SoftCurrency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	int32 Coin = 0;
};

USTRUCT(BlueprintType)
struct FTainlordReputationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	int32 CityReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	int32 MercenaryReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	int32 Infamy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	int32 PoliticalStanding = 0;
};

USTRUCT(BlueprintType)
struct FTainlordSkillSelectionRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	int32 MaxLearnedSkills = 24;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	int32 MaxEquippedSkills = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	int32 MaxElementSpecializations = 2;
};

USTRUCT(BlueprintType)
struct FTainlordCustomizationBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName RaceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName BodySetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName HeadPresetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FName HairPresetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	FLinearColor SecondaryColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FTainlordCombatResources
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 MaxHealth = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 CurrentHealth = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 MaxMana = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 CurrentMana = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 MaxStamina = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 CurrentStamina = 100;
};

USTRUCT(BlueprintType)
struct FTainlordAttackProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ETainlordActionType ActionType = ETainlordActionType::BasicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ETainlordScalingAttribute ScalingAttribute = ETainlordScalingAttribute::Strength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ETainlordElement Element = ETainlordElement::Fire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BasePower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ScalingMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float CooldownSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ResourceCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float RangeMeters = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bStarterAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bSignatureAction = false;
};

USTRUCT(BlueprintType)
struct FTainlordCombatBudget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ETainlordArchetype Archetype = ETainlordArchetype::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<FTainlordAttackProfile> StarterActions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 StarterSkillCount = 0;
};

USTRUCT(BlueprintType)
struct FTainlordCombatResolution
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 FinalMagnitude = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsHealing = false;
};

USTRUCT(BlueprintType)
struct FTainlordGemDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FName GemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	ETainlordGemSize Size = ETainlordGemSize::Mini;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	ETainlordGemModifierType ModifierType = ETainlordGemModifierType::PhysicalPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	float ModifierPercent = 5.0f;
};

USTRUCT(BlueprintType)
struct FTainlordItemDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	ETainlordItemTier Tier = ETainlordItemTier::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	ETainlordEquipmentSlot EquipmentSlot = ETainlordEquipmentSlot::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 GemSlotCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FTainlordStatBlock GrantedStats;
};

USTRUCT(BlueprintType)
struct FTainlordInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FName EntryId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FTainlordItemDefinition Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FTainlordGemDefinition> SocketedGems;
};

USTRUCT(BlueprintType)
struct FTainlordEquipmentLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TMap<ETainlordEquipmentSlot, FTainlordInventoryEntry> EquippedItems;
};

USTRUCT(BlueprintType)
struct FTainlordClassDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	ETainlordArchetype Archetype = ETainlordArchetype::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	ETainlordCombatRole PrimaryRole = ETainlordCombatRole::Frontline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	ETainlordStatOrientation DefaultOrientation = ETainlordStatOrientation::Strength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	TArray<ETainlordStatOrientation> AllowedOrientations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	TArray<ETainlordElement> RecommendedElements;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	bool bIncludedInFundingDemo = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	FText ClassSummary;
};

USTRUCT(BlueprintType)
struct FTainlordProgressionState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 Experience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 ExperienceToNextLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	ETainlordArchetype Archetype = ETainlordArchetype::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	ETainlordStatOrientation StatOrientation = ETainlordStatOrientation::Strength;
};

USTRUCT(BlueprintType)
struct FTainlordAlchemyCombineResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alchemy")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alchemy")
	ETainlordGemSize ResultSize = ETainlordGemSize::Mini;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alchemy")
	int32 ConsumedCount = 0;
};

USTRUCT(BlueprintType)
struct FTainlordCharacterState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	ETainlordAllegiance Allegiance = ETainlordAllegiance::City;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordProgressionState Progression;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordCustomizationBinding Customization;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordStatBlock DerivedStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordCombatResources CombatResources;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordElementLoadout Elements;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordCurrencyWallet Wallet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordReputationState Reputation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordSkillSelectionRules SkillRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FTainlordEquipmentLoadout Equipment;
};
