// Copyright Kingdawn. All Rights Reserved.
// Combat attribute set for Health, Stamina, and meta damage attributes.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "KDCombatAttributeSet.generated.h"

// Macro for boilerplate attribute accessors
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Combat-specific attribute set. Provides Health, Stamina, and meta attributes
 * for damage application.
 */
UCLASS()
class KINGDAWNCOMBAT_API UKDCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UKDCombatAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Primary Attributes ---

	/** Current health. When this reaches 0, the entity dies. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, Health)

	/** Maximum health. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, MaxHealth)

	/** Current stamina. Consumed by attacks, blocks, and dodges. Regenerates over time. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, Stamina)

	/** Maximum stamina. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, MaxStamina)

	/** Current mana. Consumed by spells and magical abilities. Regenerates over time. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, Mana)

	/** Maximum mana. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, MaxMana)

	/** Current max walk speed. Modified by Slow/Freeze GEs. Drives CMC::MaxWalkSpeed. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, MoveSpeed)

	// --- Meta Attributes (transient, not replicated) ---

	/** Incoming damage. This is a meta attribute used by damage calculations, not persisted. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, IncomingDamage)

	/** Stamina cost of the current action. Meta attribute consumed by abilities. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes|Meta")
	FGameplayAttributeData StaminaCost;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, StaminaCost)

	/** Mana cost of the current action. Meta attribute consumed by spells. */
	UPROPERTY(BlueprintReadOnly, Category = "Kingdawn|Attributes|Meta")
	FGameplayAttributeData ManaCost;
	ATTRIBUTE_ACCESSORS(UKDCombatAttributeSet, ManaCost)

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
};
