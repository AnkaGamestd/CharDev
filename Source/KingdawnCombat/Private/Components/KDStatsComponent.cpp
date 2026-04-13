// Copyright Kingdawn. All Rights Reserved.

#include "Components/KDStatsComponent.h"
#include "Attributes/KDCombatAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"
#include "Engine/World.h"

UKDStatsComponent::UKDStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	StaminaRegenRate = 10.f;
	StaminaRegenDelay = 2.f;
	bStaminaRegenEnabled = true;

	ManaRegenRate = 5.f;
	ManaRegenDelay = 3.f;
	bManaRegenEnabled = true;

	TimeSinceLastStaminaUse = 0.f;
	TimeSinceLastManaUse = 0.f;
	bInitialized = false;
}

void UKDStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Do NOT auto-initialize here. The owning character's InitializeAbilitySystem()
	// will call InitializeWithAbilitySystem() after creating the AttributeSet.
	// Auto-init here fails because the AttributeSet hasn't been created yet.
}

void UKDStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized)
	{
		return;
	}

	// Stamina regeneration with delay
	if (bStaminaRegenEnabled)
	{
		TimeSinceLastStaminaUse += DeltaTime;
		if (TimeSinceLastStaminaUse >= StaminaRegenDelay)
		{
			RegenerateStamina(StaminaRegenRate * DeltaTime);
		}
	}

	// Mana regeneration with delay
	if (bManaRegenEnabled)
	{
		TimeSinceLastManaUse += DeltaTime;
		if (TimeSinceLastManaUse >= ManaRegenDelay)
		{
			RegenerateMana(ManaRegenRate * DeltaTime);
		}
	}
}

void UKDStatsComponent::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		UE_LOG(LogTemp, Error, TEXT("KDStatsComponent::InitializeWithAbilitySystem - Invalid ASC"));
		return;
	}

	AbilitySystemComponent = InASC;

	// Find the combat attribute set
	const UAttributeSet* AttributeSet = AbilitySystemComponent->GetAttributeSet(UKDCombatAttributeSet::StaticClass());
	CombatAttributeSet = const_cast<UKDCombatAttributeSet*>(Cast<UKDCombatAttributeSet>(AttributeSet));

	if (!CombatAttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("KDStatsComponent::InitializeWithAbilitySystem - KDCombatAttributeSet not found on ASC"));
		return;
	}

	BindAttributeChangeDelegates();
	bInitialized = true;
	SetComponentTickEnabled(true);
}

void UKDStatsComponent::BindAttributeChangeDelegates()
{
	if (!AbilitySystemComponent || !CombatAttributeSet)
	{
		return;
	}

	// Bind to health changes
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		CombatAttributeSet->GetHealthAttribute()).AddUObject(this, &UKDStatsComponent::OnHealthChanged);

	// Bind to stamina changes
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		CombatAttributeSet->GetStaminaAttribute()).AddUObject(this, &UKDStatsComponent::OnStaminaChanged);

	// Bind to mana changes
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		CombatAttributeSet->GetManaAttribute()).AddUObject(this, &UKDStatsComponent::OnManaChanged);
}

// --- Stat Getters ---

float UKDStatsComponent::GetHealth() const
{
	if (!CombatAttributeSet)
	{
		return 0.f;
	}
	return CombatAttributeSet->GetHealth();
}

float UKDStatsComponent::GetMaxHealth() const
{
	if (!CombatAttributeSet)
	{
		return 0.f;
	}
	return CombatAttributeSet->GetMaxHealth();
}

float UKDStatsComponent::GetHealthNormalized() const
{
	const float Max = GetMaxHealth();
	if (Max <= 0.f)
	{
		return 0.f;
	}
	return GetHealth() / Max;
}

float UKDStatsComponent::GetStamina() const
{
	if (!CombatAttributeSet)
	{
		return 0.f;
	}
	return CombatAttributeSet->GetStamina();
}

float UKDStatsComponent::GetMaxStamina() const
{
	if (!CombatAttributeSet)
	{
		return 0.f;
	}
	return CombatAttributeSet->GetMaxStamina();
}

float UKDStatsComponent::GetStaminaNormalized() const
{
	const float Max = GetMaxStamina();
	if (Max <= 0.f)
	{
		return 0.f;
	}
	return GetStamina() / Max;
}

float UKDStatsComponent::GetMana() const
{
	if (!CombatAttributeSet)
	{
		return 0.f;
	}
	return CombatAttributeSet->GetMana();
}

float UKDStatsComponent::GetMaxMana() const
{
	if (!CombatAttributeSet)
	{
		return 0.f;
	}
	return CombatAttributeSet->GetMaxMana();
}

float UKDStatsComponent::GetManaNormalized() const
{
	const float Max = GetMaxMana();
	if (Max <= 0.f)
	{
		return 0.f;
	}
	return GetMana() / Max;
}

bool UKDStatsComponent::IsAlive() const
{
	return GetHealth() > 0.f;
}

// --- Stat Modifiers ---

void UKDStatsComponent::ReceiveDamage(float DamageAmount, AController* Instigator, AActor* DamageCauser)
{
	if (!bInitialized || !AbilitySystemComponent || DamageAmount <= 0.f)
	{
		return;
	}

	// Apply damage by modifying the IncomingDamage meta attribute
	// The attribute set's PostGameplayEffectExecute will handle applying it to Health
	if (CombatAttributeSet)
	{
		const float NewHealth = FMath::Max(0.f, GetHealth() - DamageAmount);
		AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetHealthAttribute(), NewHealth);
	}
}

bool UKDStatsComponent::ConsumeStamina(float Cost)
{
	if (!bInitialized || !AbilitySystemComponent || Cost <= 0.f)
	{
		return false;
	}

	const float CurrentStamina = GetStamina();
	if (CurrentStamina < Cost)
	{
		return false;
	}

	const float NewStamina = FMath::Max(0.f, CurrentStamina - Cost);
	AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetStaminaAttribute(), NewStamina);

	TimeSinceLastStaminaUse = 0.f;
	return true;
}

void UKDStatsComponent::RegenerateStamina(float Amount)
{
	if (!bInitialized || !AbilitySystemComponent || Amount <= 0.f)
	{
		return;
	}

	const float CurrentStamina = GetStamina();
	const float MaxStamina = GetMaxStamina();
	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	const float NewStamina = FMath::Min(MaxStamina, CurrentStamina + Amount);
	AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetStaminaAttribute(), NewStamina);
}

void UKDStatsComponent::RestoreHealth()
{
	if (!bInitialized || !AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetHealthAttribute(), GetMaxHealth());
}

void UKDStatsComponent::RestoreStamina()
{
	if (!bInitialized || !AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetStaminaAttribute(), GetMaxStamina());
}

void UKDStatsComponent::RestoreMana()
{
	if (!bInitialized || !AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetManaAttribute(), GetMaxMana());
}

void UKDStatsComponent::RestoreAllStats()
{
	RestoreHealth();
	RestoreStamina();
	RestoreMana();
}

bool UKDStatsComponent::CanAffordStaminaCost(float Cost) const
{
	return GetStamina() >= Cost;
}

bool UKDStatsComponent::CanAffordManaCost(float Cost) const
{
	return GetMana() >= Cost;
}

// --- Internal ---

void UKDStatsComponent::OnHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnStatChanged.Broadcast(FGameplayTag(), ChangeData.NewValue);
	OnAttributeSetModified.Broadcast();

	if (ChangeData.NewValue <= 0.f)
	{
		OnHealthReachedZero.Broadcast();
	}
}

void UKDStatsComponent::OnStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnStatChanged.Broadcast(FGameplayTag(), ChangeData.NewValue);
	OnAttributeSetModified.Broadcast();
}

bool UKDStatsComponent::ConsumeMana(float Cost)
{
	if (!bInitialized || !AbilitySystemComponent || Cost <= 0.f)
	{
		return false;
	}

	const float CurrentMana = GetMana();
	if (CurrentMana < Cost)
	{
		return false;
	}

	const float NewMana = FMath::Max(0.f, CurrentMana - Cost);
	AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetManaAttribute(), NewMana);

	TimeSinceLastManaUse = 0.f;
	return true;
}

void UKDStatsComponent::RegenerateMana(float Amount)
{
	if (!bInitialized || !AbilitySystemComponent || Amount <= 0.f)
	{
		return;
	}

	const float CurrentMana = GetMana();
	const float MaxMana = GetMaxMana();
	if (CurrentMana >= MaxMana)
	{
		return;
	}

	const float NewMana = FMath::Min(MaxMana, CurrentMana + Amount);
	AbilitySystemComponent->SetNumericAttributeBase(CombatAttributeSet->GetManaAttribute(), NewMana);
}

void UKDStatsComponent::OnManaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnStatChanged.Broadcast(FGameplayTag(), ChangeData.NewValue);
	OnAttributeSetModified.Broadcast();
}
