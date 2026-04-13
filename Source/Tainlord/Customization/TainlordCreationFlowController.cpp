// Copyright Kingdawn. All Rights Reserved.

#include "TainlordCreationFlowController.h"

// ---------------------------------------------------------------------------
// Flow lifecycle
// ---------------------------------------------------------------------------

void UTainlordCreationFlowController::BeginFlow()
{
	// Reset all accumulated state
	FlowState = FTainlordCreationFlowState();
	FlowState.bIsFlowActive = true;

	// Default: skip City and Name until their UI is implemented
	// Current production flow: RaceSelect → Appearance → Mastery → Confirm
	if (StagesToSkip.Num() == 0)
	{
		StagesToSkip.Add(ECreationStage::City);
		StagesToSkip.Add(ECreationStage::Name);
	}

	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: BeginFlow — starting from RaceSelect"));

	TransitionToStage(ECreationStage::RaceSelect);
}

void UTainlordCreationFlowController::AdvanceToNextStage()
{
	if (!FlowState.IsFlowActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreationFlowController: AdvanceToNextStage called but flow is not active"));
		return;
	}

	// If we're already at Confirm, complete the flow
	if (FlowState.CurrentStage == ECreationStage::Confirm)
	{
		UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Flow completed — broadcasting profile"));
		FlowState.bIsFlowActive = false;
		OnFlowCompleted.Broadcast(FlowState.WorkingProfile);
		return;
	}

	ECreationStage NextStage = ResolveNextStage(FlowState.CurrentStage);

	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Advancing from %s to %s"),
		*UEnum::GetValueAsString(FlowState.CurrentStage),
		*UEnum::GetValueAsString(NextStage));

	TransitionToStage(NextStage);
}

void UTainlordCreationFlowController::ReturnToPreviousStage()
{
	if (!FlowState.IsFlowActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreationFlowController: ReturnToPreviousStage called but flow is not active"));
		return;
	}

	// If at the first stage, abort
	if (FCreationStageNavigation::IsFirstStage(FlowState.CurrentStage))
	{
		UE_LOG(LogTemp, Log, TEXT("CreationFlowController: At first stage — aborting flow"));
		AbortFlow();
		return;
	}

	ECreationStage PrevStage = ResolvePreviousStage(FlowState.CurrentStage);

	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Returning from %s to %s"),
		*UEnum::GetValueAsString(FlowState.CurrentStage),
		*UEnum::GetValueAsString(PrevStage));

	TransitionToStage(PrevStage);
}

void UTainlordCreationFlowController::AbortFlow()
{
	ECreationStage OldStage = FlowState.CurrentStage;
	FlowState.bIsFlowActive = false;

	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Flow aborted (was at stage %s)"),
		*UEnum::GetValueAsString(OldStage));

	OnFlowAborted.Broadcast();
}

// ---------------------------------------------------------------------------
// Stage data commits
// ---------------------------------------------------------------------------

void UTainlordCreationFlowController::CommitRaceSelection(ECharacterRace Race, ECharacterGender Gender)
{
	FlowState.WorkingProfile.Race = Race;
	FlowState.WorkingProfile.Gender = Gender;
	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Race/Gender committed — %s / %s"),
		*UEnum::GetValueAsString(Race), *UEnum::GetValueAsString(Gender));
}

void UTainlordCreationFlowController::CommitAppearanceData(const FTainlordAppearanceData& Appearance)
{
	FlowState.WorkingProfile.AppearanceData = Appearance;
	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Appearance committed"));
}

void UTainlordCreationFlowController::CommitMasterySelection(FName MasteryId)
{
	FlowState.WorkingProfile.SelectedMasteryId = MasteryId;
	FlowState.WorkingProfile.SelectedBuildId = MasteryId; // Backward compat
	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Mastery committed — %s"), *MasteryId.ToString());
}

void UTainlordCreationFlowController::CommitCitySelection(FName CityId)
{
	FlowState.WorkingProfile.SelectedCityId = CityId;
	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: City committed — %s"), *CityId.ToString());
}

void UTainlordCreationFlowController::CommitCharacterName(const FString& Name)
{
	FlowState.WorkingProfile.CharacterName = Name;
	UE_LOG(LogTemp, Log, TEXT("CreationFlowController: Name committed — %s"), *Name);
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool UTainlordCreationFlowController::ShouldSkipStage(ECreationStage Stage) const
{
	return StagesToSkip.Contains(Stage);
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

void UTainlordCreationFlowController::TransitionToStage(ECreationStage NewStage)
{
	ECreationStage OldStage = FlowState.CurrentStage;
	FlowState.CurrentStage = NewStage;

	OnStageChanged.Broadcast(OldStage, NewStage);
}

ECreationStage UTainlordCreationFlowController::ResolveNextStage(ECreationStage FromStage) const
{
	ECreationStage Next = FCreationStageNavigation::GetNextStage(FromStage);

	// Skip stages that are marked for skipping
	// Walk forward until we find a non-skipped stage or reach Confirm
	int32 SafetyCounter = 0;
	while (!FCreationStageNavigation::IsTerminalStage(Next) &&
		   StagesToSkip.Contains(Next) &&
		   SafetyCounter < 10)
	{
		Next = FCreationStageNavigation::GetNextStage(Next);
		++SafetyCounter;
	}

	return Next;
}

ECreationStage UTainlordCreationFlowController::ResolvePreviousStage(ECreationStage FromStage) const
{
	ECreationStage Prev = FCreationStageNavigation::GetPreviousStage(FromStage);

	// Walk backward until we find a non-skipped stage or reach RaceSelect
	int32 SafetyCounter = 0;
	while (!FCreationStageNavigation::IsFirstStage(Prev) &&
		   StagesToSkip.Contains(Prev) &&
		   SafetyCounter < 10)
	{
		Prev = FCreationStageNavigation::GetPreviousStage(Prev);
		++SafetyCounter;
	}

	return Prev;
}
