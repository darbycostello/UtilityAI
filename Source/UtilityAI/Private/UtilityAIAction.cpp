#include "UtilityAIAction.h"

UUtilityAIAction::UUtilityAIAction() {
	bPendingKill = false;
}

UWorld* UUtilityAIAction::GetWorld() const {
	UObject* Outer = GetOuter();
	if (!Outer)
		return nullptr;
	AAIController* Controller = Cast<AAIController>(GetOuter());
	if (!Controller)
		return nullptr;

	return Controller->GetWorld();
}

void UUtilityAIAction::Tick(float DeltaTime, AAIController* Controller, APawn* Pawn) {
	ReceiveTick(DeltaTime, Controller, Pawn);
}

bool UUtilityAIAction::CanRun(AAIController* Controller, APawn* Pawn) const {
	return ReceiveCanRun(Controller, Pawn);
}

float UUtilityAIAction::Score(AAIController* Controller, APawn* Pawn) {
	return ReceiveScore(Controller, Pawn);
}

void UUtilityAIAction::Enter(AAIController* Controller, APawn* Pawn) {
	ReceiveEnter(Controller, Pawn);
}

void UUtilityAIAction::Exit(AAIController* Controller, APawn* Pawn) {
	ReceiveExit(Controller, Pawn);
}

void UUtilityAIAction::Spawn(AAIController* Controller) {
	ReceiveSpawn(Controller);
}

void UUtilityAIAction::Kill() {
	bPendingKill = true;
}

bool UUtilityAIAction::IsActionPendingKill() const {
	return bPendingKill;
}

void UUtilityAIAction::RemovePendingKill() {
	bPendingKill = false;
}

