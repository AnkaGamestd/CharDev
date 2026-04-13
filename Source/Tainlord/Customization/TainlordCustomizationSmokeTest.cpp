// Copyright Kingdawn. All Rights Reserved.

#include "Customization/TainlordCustomizationSmokeTest.h"
#include "Customization/TainlordCharacterAppearanceComponent.h"
#include "Customization/TainlordCharacterCustomizationCatalog.h"

DEFINE_LOG_CATEGORY_STATIC(LogCustomizationTest, Log, All);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void LogTestResult(const TCHAR* TestName, bool bPassed)
{
	if (bPassed)
	{
		UE_LOG(LogCustomizationTest, Log, TEXT("[PASS] %s"), TestName);
	}
	else
	{
		UE_LOG(LogCustomizationTest, Warning, TEXT("[FAIL] %s"), TestName);
	}
}

// ---------------------------------------------------------------------------
// RunAllSmokeTests
// ---------------------------------------------------------------------------

bool UTainlordCustomizationSmokeTest::RunAllSmokeTests(UTainlordCharacterAppearanceComponent* AppearanceComponent)
{
	UE_LOG(LogCustomizationTest, Log, TEXT(""));
	UE_LOG(LogCustomizationTest, Log, TEXT("============================================================"));
	UE_LOG(LogCustomizationTest, Log, TEXT("Tainlord Customization Smoke Test - START"));
	UE_LOG(LogCustomizationTest, Log, TEXT("============================================================"));

	if (!AppearanceComponent)
	{
		UE_LOG(LogCustomizationTest, Error, TEXT("[FAIL] AppearanceComponent is null - cannot run tests"));
		return false;
	}

	// Verify catalog is set and loadable
	UTainlordCharacterCustomizationCatalog* Catalog = AppearanceComponent->GetLoadedCatalog();
	if (!Catalog)
	{
		UE_LOG(LogCustomizationTest, Error, TEXT("[FAIL] No catalog loaded on appearance component - set the Catalog property first"));
		return false;
	}

	UE_LOG(LogCustomizationTest, Log, TEXT("Catalog loaded: %s"), *Catalog->GetName());
	UE_LOG(LogCustomizationTest, Log, TEXT("  Heads: %d entries"), Catalog->Heads.Num());
	UE_LOG(LogCustomizationTest, Log, TEXT("  Hair: %d entries"), Catalog->Hair.Num());
	UE_LOG(LogCustomizationTest, Log, TEXT("  Beards: %d entries"), Catalog->Beards.Num());
	UE_LOG(LogCustomizationTest, Log, TEXT("  SkinTones: %d entries"), Catalog->SkinTones.Num());

	bool bAllPassed = true;

	// Test 1: Basic apply
	{
		const bool bResult = TestBasicApply(AppearanceComponent);
		LogTestResult(TEXT("BasicApply"), bResult);
		bAllPassed &= bResult;
	}

	// Test 2: Context rejection
	{
		const bool bResult = TestContextRejection(AppearanceComponent);
		LogTestResult(TEXT("ContextRejection"), bResult);
		bAllPassed &= bResult;
	}

	// Test 3: Skin tone cycle
	{
		const bool bResult = TestSkinToneCycle(AppearanceComponent);
		LogTestResult(TEXT("SkinToneCycle"), bResult);
		bAllPassed &= bResult;
	}

	UE_LOG(LogCustomizationTest, Log, TEXT(""));
	UE_LOG(LogCustomizationTest, Log, TEXT("============================================================"));
	UE_LOG(LogCustomizationTest, Log, TEXT("Tainlord Customization Smoke Test - %s"),
		bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	UE_LOG(LogCustomizationTest, Log, TEXT("============================================================"));

	return bAllPassed;
}

// ---------------------------------------------------------------------------
// TestBasicApply
// ---------------------------------------------------------------------------

bool UTainlordCustomizationSmokeTest::TestBasicApply(UTainlordCharacterAppearanceComponent* AppearanceComponent)
{
	if (!AppearanceComponent)
	{
		return false;
	}

	UTainlordCharacterCustomizationCatalog* Catalog = AppearanceComponent->GetLoadedCatalog();
	if (!Catalog)
	{
		UE_LOG(LogCustomizationTest, Warning, TEXT("  BasicApply: No catalog loaded"));
		return false;
	}

	// Find the first valid Male/Human head
	FName TestHeadId = NAME_None;
	TArray<FTainlordHeadEntry> FilteredHeads = Catalog->GetFilteredHeads(ECharacterGender::Male, ECharacterRace::Human);
	if (FilteredHeads.Num() > 0)
	{
		TestHeadId = FilteredHeads[0].Id;
		UE_LOG(LogCustomizationTest, Log, TEXT("  BasicApply: Using head '%s'"), *TestHeadId.ToString());
	}
	else
	{
		UE_LOG(LogCustomizationTest, Warning, TEXT("  BasicApply: No Male/Human heads in catalog (empty catalog is OK for skeleton test)"));
	}

	// Find first valid skin tone
	FName TestSkinToneId = NAME_None;
	TArray<FTainlordSkinToneEntry> FilteredSkinTones = Catalog->GetFilteredSkinTones(ECharacterGender::Male, ECharacterRace::Human);
	if (FilteredSkinTones.Num() > 0)
	{
		TestSkinToneId = FilteredSkinTones[0].Id;
		UE_LOG(LogCustomizationTest, Log, TEXT("  BasicApply: Using skin tone '%s'"), *TestSkinToneId.ToString());
	}

	// Build test appearance data
	FTainlordAppearanceData TestData;
	TestData.HeadId = TestHeadId;
	TestData.SkinToneId = TestSkinToneId;

	// Apply with context
	AppearanceComponent->ApplyAppearanceWithContext(TestData, ECharacterGender::Male, ECharacterRace::Human);

	// Verify current appearance matches what we set
	const FTainlordAppearanceData& Current = AppearanceComponent->GetCurrentAppearance();
	const bool bHeadMatch = (Current.HeadId == TestData.HeadId);
	const bool bSkinMatch = (Current.SkinToneId == TestData.SkinToneId);

	UE_LOG(LogCustomizationTest, Log, TEXT("  BasicApply: HeadId match=%s, SkinToneId match=%s"),
		bHeadMatch ? TEXT("true") : TEXT("false"),
		bSkinMatch ? TEXT("true") : TEXT("false"));

	return bHeadMatch && bSkinMatch;
}

// ---------------------------------------------------------------------------
// TestContextRejection
// ---------------------------------------------------------------------------

bool UTainlordCustomizationSmokeTest::TestContextRejection(UTainlordCharacterAppearanceComponent* AppearanceComponent)
{
	if (!AppearanceComponent)
	{
		return false;
	}

	UTainlordCharacterCustomizationCatalog* Catalog = AppearanceComponent->GetLoadedCatalog();
	if (!Catalog)
	{
		UE_LOG(LogCustomizationTest, Warning, TEXT("  ContextRejection: No catalog loaded"));
		return false;
	}

	// Look for a Female-specific head entry
	FName FemaleHeadId = NAME_None;
	for (const FTainlordHeadEntry& Head : Catalog->Heads)
	{
		if (Head.GenderFilter == ECharacterGender::Female && Head.IsValid())
		{
			FemaleHeadId = Head.Id;
			break;
		}
	}

	if (FemaleHeadId.IsNone())
	{
		// No Female-specific head in catalog. Try using a non-existent ID instead.
		FemaleHeadId = FName(TEXT("nonexistent_head_for_rejection_test"));
		UE_LOG(LogCustomizationTest, Log, TEXT("  ContextRejection: No Female heads in catalog, using fake ID for rejection test"));
	}
	else
	{
		UE_LOG(LogCustomizationTest, Log, TEXT("  ContextRejection: Found Female head '%s', applying with Male context"), *FemaleHeadId.ToString());
	}

	// Set context to Male and try to apply a Female head
	AppearanceComponent->SetProfileContext(ECharacterGender::Male, ECharacterRace::Human);

	// Clear first to get clean state
	AppearanceComponent->ClearAppearance();

	// Try to apply the female/nonexistent head - should fail
	const bool bApplied = AppearanceComponent->ApplyHead(FemaleHeadId);

	// Success = rejection occurred (bApplied should be false)
	if (!bApplied)
	{
		UE_LOG(LogCustomizationTest, Log, TEXT("  ContextRejection: Correctly rejected head '%s' for Male context"), *FemaleHeadId.ToString());
		return true;
	}
	else
	{
		UE_LOG(LogCustomizationTest, Warning, TEXT("  ContextRejection: Head '%s' was NOT rejected - context validation may not be working"), *FemaleHeadId.ToString());
		return false;
	}
}

// ---------------------------------------------------------------------------
// TestSkinToneCycle
// ---------------------------------------------------------------------------

bool UTainlordCustomizationSmokeTest::TestSkinToneCycle(UTainlordCharacterAppearanceComponent* AppearanceComponent)
{
	if (!AppearanceComponent)
	{
		return false;
	}

	UTainlordCharacterCustomizationCatalog* Catalog = AppearanceComponent->GetLoadedCatalog();
	if (!Catalog)
	{
		UE_LOG(LogCustomizationTest, Warning, TEXT("  SkinToneCycle: No catalog loaded"));
		return false;
	}

	AppearanceComponent->SetProfileContext(ECharacterGender::Male, ECharacterRace::Human);

	// Get available skin tones
	TArray<FTainlordSkinToneEntry> SkinTones = Catalog->GetFilteredSkinTones(ECharacterGender::Male, ECharacterRace::Human);
	if (SkinTones.Num() == 0)
	{
		UE_LOG(LogCustomizationTest, Log, TEXT("  SkinToneCycle: No skin tones in catalog, skipping (PASS by default)"));
		return true;
	}

	// Apply first skin tone
	const FName FirstTone = SkinTones[0].Id;
	const bool bApplied = AppearanceComponent->ApplySkinTone(FirstTone);
	UE_LOG(LogCustomizationTest, Log, TEXT("  SkinToneCycle: Applied skin tone '%s' = %s"),
		*FirstTone.ToString(), bApplied ? TEXT("OK") : TEXT("FAILED"));

	if (!bApplied)
	{
		return false;
	}

	// Apply a second one if available
	if (SkinTones.Num() > 1)
	{
		const FName SecondTone = SkinTones[1].Id;
		const bool bSecondApplied = AppearanceComponent->ApplySkinTone(SecondTone);
		UE_LOG(LogCustomizationTest, Log, TEXT("  SkinToneCycle: Swapped to skin tone '%s' = %s"),
			*SecondTone.ToString(), bSecondApplied ? TEXT("OK") : TEXT("FAILED"));
	}

	// Reset skin tone (apply None)
	const bool bReset = AppearanceComponent->ApplySkinTone(NAME_None);
	UE_LOG(LogCustomizationTest, Log, TEXT("  SkinToneCycle: Reset skin tone = %s"),
		bReset ? TEXT("OK") : TEXT("FAILED"));

	// Verify current appearance has no skin tone set
	const FTainlordAppearanceData& Current = AppearanceComponent->GetCurrentAppearance();
	const bool bSkinToneCleared = Current.SkinToneId.IsNone();
	UE_LOG(LogCustomizationTest, Log, TEXT("  SkinToneCycle: SkinToneId cleared = %s"),
		bSkinToneCleared ? TEXT("true") : TEXT("false"));

	return bReset && bSkinToneCleared;
}
