#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Domain/TainlordGameplayTypes.h"
#include "TainlordItemRuleLibrary.generated.h"

UCLASS()
class TAINLORD_API UTainlordItemRuleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tainlord|Items")
	static FTainlordItemDefinition MakeStarterWeaponDefinition(ETainlordArchetype Archetype);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Items")
	static bool CanEquipItem(const FTainlordCharacterState& CharacterState, const FTainlordItemDefinition& ItemDefinition);

	UFUNCTION(BlueprintPure, Category = "Tainlord|Items")
	static FTainlordEquipmentLoadout EquipItem(const FTainlordEquipmentLoadout& CurrentLoadout, const FTainlordInventoryEntry& InventoryEntry);
};
