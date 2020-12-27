#include "UtilityAIComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"

UUtilityAIComponent::UUtilityAIComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	bIgnoreZeroScore = true;
	bCanRunWithoutPawn = true;
}

void UUtilityAIComponent::BeginPlay() {
	Super::BeginPlay();

	LastAction = nullptr;
	LastPawn = nullptr;
	LastSwitchTime = 0;
	for (const TSubclassOf<UUtilityAIAction> ActionClass : Actions) {
		SpawnActionInstance(ActionClass);
	}

	OnUtilityInitialised.Broadcast();
}

bool UUtilityAIComponent::CanSpawnActionInstance(const TSubclassOf<UUtilityAIAction> ActionClass) const {
	for (UUtilityAIAction* Action : InstancedActions) {
		if (Action->GetClass() == ActionClass)
			return false;
	}
	return true;
}

UUtilityAIAction* UUtilityAIComponent::SpawnActionInstance(const TSubclassOf<UUtilityAIAction> ActionClass) {
	if (!ActionClass) return nullptr;
	if (!CanSpawnActionInstance(ActionClass)) return nullptr;

	AAIController* Controller = Cast<AAIController>(GetOwner());
	if (!Controller) return nullptr;

	UUtilityAIAction* Action = NewObject<UUtilityAIAction>(Controller, ActionClass);
	InstancedActions.Add(Action);
	Action->Spawn(Controller);
	OnUtilityActionSpawned.Broadcast(Action);

	return Action;
}

bool UUtilityAIComponent::InternalRandBool() const {
	const float r = bUseRandomStream ? RandomStream.FRandRange(0.0f, 1.0f) : FMath::RandRange(0.0f, 1.0f);
	return 0.5f > r;
}

bool UUtilityAIComponent::CheckLowestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const {
	if (!Best) return true;

	if (FMath::Abs(Best->LastScore - Current->LastScore) <= EqualityTolerance) {
		if (bInvertPriority) return true;
		if (bRandomizeOnEquality) return InternalRandBool();
		return false;
	}

	if (Best->LastScore > Current->LastScore) {
		return true;
	}

	return false;
}

bool UUtilityAIComponent::CheckHighestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const {
	if (!Best) return true;
	if (FMath::Abs(Best->LastScore - Current->LastScore) <= EqualityTolerance) {
		if (bInvertPriority) return true;
		if (bRandomizeOnEquality) return InternalRandBool();
		return false;
	}

	if (Best->LastScore < Current->LastScore) return true;
	return false;
}

UUtilityAIAction* UUtilityAIComponent::ReceiveComputeBestAction_Implementation(AAIController* Controller, APawn* Pawn) {
	UUtilityAIAction* BestAction = nullptr;

	for (UUtilityAIAction* Action : InstancedActions) {
		Action->LastCanRun = !Action->IsPendingKill() && Action->CanRun(Controller, Pawn);
		if (!Action->LastCanRun) continue;
		
		Action->LastScore = ScoreFilter(Action, Action->Score(Controller, Pawn));
		if (bIgnoreZeroScore && Action->LastScore == 0) continue;
		
		if (bInvertScoring) {
			if (CheckLowestScore(Action, BestAction)) BestAction = Action;
		} else {
			if (CheckHighestScore(Action, BestAction)) BestAction = Action;
		}
	}

	return BestAction;
}

// Called every frame
void UUtilityAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickDeltaTime += DeltaTime;
	if (!bTickManually) TickUtilityAI();
}

void UUtilityAIComponent::TickUtilityAI() {
	Evaluate();
	TickDeltaTime = 0.0f;
}

void UUtilityAIComponent::Evaluate() {
	AAIController* Controller = Cast<AAIController>(GetOwner());
	if (!Controller) return;
	OnUtilityPreScoring.Broadcast();

	APawn* Pawn = Controller->GetPawn();
	UUtilityAIAction* BestAction = nullptr;

	if (Pawn || bCanRunWithoutPawn) BestAction = ComputeBestAction(Controller, Pawn);
	if (BestAction) {
		OnUtilityActionSelected.Broadcast(BestAction);
		if (LastAction != BestAction) {
			const float CurrentTime = GetWorld()->GetTimeSeconds();

			if (LastSwitchTime == 0 || CurrentTime - LastSwitchTime > MinimumActionSeconds) {
				OnUtilityActionSwitched.Broadcast(BestAction, LastAction);
				if (LastAction) {
					LastAction->Exit(Controller, LastPawn);
				}
				BestAction->Enter(Controller, Pawn);
				LastSwitchTime = CurrentTime;
			} else {
				if (!LastAction) return;
				BestAction = LastAction;
			}
		}
		BestAction->Tick(TickDeltaTime, Controller, Pawn);
		LastAction = BestAction;
		LastPawn = Pawn;
		OnUtilityActionTicked.Broadcast(BestAction);
		return;
	}

	OnUtilityActionUnavailable.Broadcast();
	if (LastAction) {
		OnUtilityActionSwitched.Broadcast(nullptr, LastAction);
		LastAction->Exit(Controller, LastPawn);
		LastAction->RemovePendingKill();
		LastAction = nullptr;
		LastPawn = nullptr;
	}
}

TArray<UUtilityAIAction*> UUtilityAIComponent::GetActionInstances() const {
	return InstancedActions.Array();
}

UUtilityAIAction* UUtilityAIComponent::GetActionInstanceByClass(const TSubclassOf<UUtilityAIAction> ActionClass) const {
	for (UUtilityAIAction* Action : InstancedActions) {
		if (Action->GetClass() == ActionClass) return Action;
	}
	return nullptr;
}

UUtilityAIAction* UUtilityAIComponent::GetCurrentActionInstance() const {
	return LastAction;
}

TSubclassOf<UUtilityAIAction> UUtilityAIComponent::GetCurrentActionClass() const {
	return LastAction ? LastAction->GetClass() : nullptr;
}

void UUtilityAIComponent::SetRandomStream(FRandomStream InRandomStream) {
	RandomStream = InRandomStream;
	bUseRandomStream = true;
}

FRandomStream UUtilityAIComponent::GetRandomStream() const {
	return RandomStream;
}

UUtilityAIAction* UUtilityAIComponent::ComputeBestAction(AAIController* Controller, APawn* Pawn) {
	return ReceiveComputeBestAction(Controller, Pawn);
}
