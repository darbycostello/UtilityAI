#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "UtilityAIAction.generated.h"

/**
 * Utility AI Action class
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class UTILITYAI_API UUtilityAIAction final : public UObject
{
	GENERATED_BODY()

public:

	UUtilityAIAction();

	UPROPERTY(BlueprintReadOnly)
	float LastScore;

	UPROPERTY(BlueprintReadOnly)
	bool LastCanRun;

	UFUNCTION(BlueprintCallable)
	void Kill();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsActionPendingKill() const;

	UFUNCTION(BlueprintCallable)
	void RemovePendingKill();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Spawn"))
    void ReceiveSpawn(AAIController* Controller);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Tick"))
    void ReceiveTick(float DeltaTime, AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "Can Run"))
    bool ReceiveCanRun(AAIController* Controller, APawn* Pawn) const;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Score"))
    float ReceiveScore(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Enter"))
    void ReceiveEnter(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="Exit"))
    void ReceiveExit(AAIController* Controller, APawn* Pawn);

	virtual void Spawn(AAIController* Controller);
	virtual void Tick(float DeltaTime, AAIController* Controller, APawn* Pawn);
	virtual bool CanRun(AAIController* Controller, APawn* Pawn) const;
	virtual float Score(AAIController* Controller, APawn* Pawn);
	virtual void Enter(AAIController* Controller, APawn* Pawn);
	virtual void Exit(AAIController* Controller, APawn* Pawn);	
	

private:
	bool bPendingKill;

	virtual UWorld* GetWorld() const override;
	bool ReceiveCanRun_Implementation(AAIController* Controller, APawn* Pawn) const { return true; }
};
