// Copyright Kingdawn. All Rights Reserved.

#include "Attributes/KDCombatAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

UKDCombatAttributeSet::UKDCombatAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitMana(100.f);
	InitMaxMana(100.f);
	InitMoveSpeed(600.f); // UE default walk speed
	InitIncomingDamage(0.f);
	InitStaminaCost(0.f);
	InitManaCost(0.f);
}

void UKDCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp Health to [0, MaxHealth]
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	// Clamp Stamina to [0, MaxStamina]
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	// Clamp Mana to [0, MaxMana]
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	// Clamp MoveSpeed to [0, max reasonable]
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 2000.f);

		// Drive CharacterMovementComponent::MaxWalkSpeed from the attribute
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			if (AActor* OwnerActor = ASC->GetAvatarActor())
			{
				if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
				{
					if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
					{
						CMC->MaxWalkSpeed = NewValue;
					}
				}
			}
		}
	}
}

void UKDCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Process IncomingDamage meta attribute: subtract from Health
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		if (LocalDamage > 0.f)
		{
			const float NewHealth = GetHealth() - LocalDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		}
	}

	// Process StaminaCost meta attribute: subtract from Stamina
	if (Data.EvaluatedData.Attribute == GetStaminaCostAttribute())
	{
		const float LocalCost = GetStaminaCost();
		SetStaminaCost(0.f);

		if (LocalCost > 0.f)
		{
			const float NewStamina = GetStamina() - LocalCost;
			SetStamina(FMath::Clamp(NewStamina, 0.f, GetMaxStamina()));
		}
	}

	// Process ManaCost meta attribute: subtract from Mana
	if (Data.EvaluatedData.Attribute == GetManaCostAttribute())
	{
		const float LocalCost = GetManaCost();
		SetManaCost(0.f);

		if (LocalCost > 0.f)
		{
			const float NewMana = GetMana() - LocalCost;
			SetMana(FMath::Clamp(NewMana, 0.f, GetMaxMana()));
		}
	}
}

void UKDCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UKDCombatAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKDCombatAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKDCombatAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKDCombatAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKDCombatAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKDCombatAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKDCombatAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}

void UKDCombatAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKDCombatAttributeSet, Health, OldValue);
}

void UKDCombatAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKDCombatAttributeSet, MaxHealth, OldValue);
}

void UKDCombatAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKDCombatAttributeSet, Stamina, OldValue);
}

void UKDCombatAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKDCombatAttributeSet, MaxStamina, OldValue);
}

void UKDCombatAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKDCombatAttributeSet, Mana, OldValue);
}

void UKDCombatAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKDCombatAttributeSet, MaxMana, OldValue);
}

void UKDCombatAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKDCombatAttributeSet, MoveSpeed, OldValue);
}
