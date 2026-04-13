#include "Characters/TainlordDemoBootstrapLibrary.h"

namespace
{
	bool IsFundingDemoArchetype(const ETainlordArchetype Archetype)
	{
		for (const FTainlordClassDefinition& Definition : UTainlordDemoClassLibrary::GetFundingDemoClassDefinitions())
		{
			if (Definition.Archetype == Archetype)
			{
				return true;
			}
		}

		return false;
	}

	bool IsFundingDemoCity(const FName CityId)
	{
		for (const FTainlordCityDefinition& City : UTainlordOnboardingLibrary::GetFundingDemoCities())
		{
			if (City.CityId == CityId && City.bIncludedInFundingDemo)
			{
				return true;
			}
		}

		return false;
	}
}

FTainlordStarterProfile UTainlordDemoBootstrapLibrary::GetRecommendedStarterProfile()
{
	FTainlordStarterProfile Profile;
	Profile.Archetype = ETainlordArchetype::Warrior;
	Profile.RaceId = "human";
	Profile.CityId = "aztec_capital";
	Profile.CharacterName = TEXT("TainlordHero");
	return Profile;
}

FTainlordStarterProfile UTainlordDemoBootstrapLibrary::SanitizeStarterProfile(const FTainlordStarterProfile& RequestedProfile)
{
	FTainlordStarterProfile Result = RequestedProfile;

	if (!IsFundingDemoArchetype(Result.Archetype))
	{
		Result.Archetype = GetRecommendedStarterProfile().Archetype;
	}

	if (!IsFundingDemoCity(Result.CityId))
	{
		Result.CityId = GetRecommendedStarterProfile().CityId;
	}

	if (Result.RaceId.IsNone())
	{
		Result.RaceId = GetRecommendedStarterProfile().RaceId;
	}

	if (Result.CharacterName.TrimStartAndEnd().IsEmpty())
	{
		Result.CharacterName = GetRecommendedStarterProfile().CharacterName;
	}

	return Result;
}

FTainlordBootstrapResult UTainlordDemoBootstrapLibrary::BuildBootstrapResult(const FTainlordStarterProfile& RequestedProfile)
{
	FTainlordBootstrapResult Result;
	Result.StarterProfile = SanitizeStarterProfile(RequestedProfile);
	Result.ClassDefinition = UTainlordDemoClassLibrary::GetClassDefinition(Result.StarterProfile.Archetype);
	Result.OnboardingPackage = UTainlordOnboardingLibrary::BuildFundingDemoOnboardingPackage(
		Result.StarterProfile.Archetype,
		Result.StarterProfile.RaceId,
		Result.StarterProfile.CityId);
	Result.TutorialSteps = UTainlordOnboardingLibrary::GetFundingDemoTutorialSteps();
	return Result;
}
