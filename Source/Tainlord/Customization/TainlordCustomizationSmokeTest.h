// Copyright Kingdawn. All Rights Reserved.
// Smoke test library for character customization Phase 1.
// Call from Blueprint BeginPlay or console command to validate the pipeline.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TainlordCharacterCustomizationTypes.h"
#include "TainlordCustomizationSmokeTest.generated.h"

class UTainlordCharacterAppearanceComponent;

/**
 * Phase 1 smoke test utilities for character customization.
 * Validates:
 * - Catalog loads and has entries
 * - ApplyAppearanceWithContext runs without crash
 * - Context rejection works (Female head rejected for Male context)
 * - Skin tone apply/reset cycle works
 */
UCLASS()
class TAINLORD_API UTainlordCustomizationSmokeTest : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Run all smoke tests on the given appearance component.
	 * Logs PASS/FAIL for each test case.
	 * @param AppearanceComponent The component to test (must have catalog set).
	 * @return True if all tests passed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Test")
	static bool RunAllSmokeTests(UTainlordCharacterAppearanceComponent* AppearanceComponent);

	/**
	 * Run a single apply/verify cycle with hardcoded test data.
	 * This is the minimal "does it work at all" check.
	 * @param AppearanceComponent The component to test.
	 * @return True if the test passed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Test")
	static bool TestBasicApply(UTainlordCharacterAppearanceComponent* AppearanceComponent);

	/**
	 * Test that an entry with mismatched context is rejected.
	 * Temporarily applies a Female-specific entry with Male context.
	 * @param AppearanceComponent The component to test.
	 * @return True if rejection occurred (test passed).
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Test")
	static bool TestContextRejection(UTainlordCharacterAppearanceComponent* AppearanceComponent);

	/**
	 * Test skin tone apply/reset symmetry.
	 * @param AppearanceComponent The component to test.
	 * @return True if the test passed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Test")
	static bool TestSkinToneCycle(UTainlordCharacterAppearanceComponent* AppearanceComponent);
};
