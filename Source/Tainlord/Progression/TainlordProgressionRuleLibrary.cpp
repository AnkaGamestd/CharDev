#include "Progression/TainlordProgressionRuleLibrary.h"

int32 UTainlordProgressionRuleLibrary::GetExperienceRequirementForLevel(const int32 Level)
{
	const int32 SafeLevel = FMath::Max(1, Level);
	return 100 + ((SafeLevel - 1) * 45);
}

FTainlordProgressionState UTainlordProgressionRuleLibrary::GrantExperience(const FTainlordProgressionState& ProgressionState, const int32 ExperienceGain)
{
	FTainlordProgressionState Result = ProgressionState;
	Result.Experience += FMath::Max(0, ExperienceGain);

	while (Result.Experience >= Result.ExperienceToNextLevel)
	{
		Result.Experience -= Result.ExperienceToNextLevel;
		Result.Level += 1;
		Result.ExperienceToNextLevel = GetExperienceRequirementForLevel(Result.Level);
	}

	return Result;
}

FTainlordCurrencyWallet UTainlordProgressionRuleLibrary::BuildStarterWallet()
{
	FTainlordCurrencyWallet Wallet;
	Wallet.SoftCurrency = 250;
	Wallet.Coin = 0;
	return Wallet;
}
