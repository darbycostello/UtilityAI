#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Core/Public/Math/RandomStream.h"
#include "UtilityAIAction.h"
#include "UtilityAIComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUtilityActionSpawned, UUtilityAIAction*, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUtilityActionSwitched, UUtilityAIAction*, NewAction, UUtilityAIAction*, OldAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUtilityActionUnavailable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUtilityInitialised);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUtilityPreScoring);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUtilityActionSelected, UUtilityAIAction*, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUtilityActionTicked, UUtilityAIAction*, Action);


UCLASS(BlueprintType, Blueprintable, ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class UTILITYAI_API UUtilityAIComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UUtilityAIComponent();

	// The pool of actions available to this Utility AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI", meta=(ExposeOnSpawn="true"))
	TSet<TSubclassOf<UUtilityAIAction>> Actions;

	// Whether this component's tick should be called manually instead of on every level tick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
    bool bTickManually;
	
	// Ignore actions returning a score of zero, this has the same effect of CanRun() returning false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	bool bIgnoreZeroScore;

	// Invert the algorithm by choosing the lowest value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	bool bInvertScoring;

	// Actions are evaluated in the order they have been inserted. Inverting priority means that in the event of a tie, the last inserted action will be selected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	bool bInvertPriority;

	// Instead of using priority, randomize the choice in the event of a tie
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	bool bRandomizeOnEquality;

	// Set a tolerance threshold for float equality when scoring
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	float EqualityTolerance;

	// The minimum amount of time in seconds before allowing an action switch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	float MinimumActionSeconds;

	// Whether this component can be initialised without a pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility AI")
	bool bCanRunWithoutPawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility AI")
	float TickDeltaTime;
	
	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On Utility Action Spawned"))
	FUtilityActionSpawned OnUtilityActionSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On Utility Action Switched"))
	FUtilityActionSwitched OnUtilityActionSwitched;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On Utility Action Unavailable"))
	FUtilityActionUnavailable OnUtilityActionUnavailable;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On Utility AI Initialised"))
	FUtilityInitialised OnUtilityInitialised;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On Utility Action Selected"))
	FUtilityActionSelected OnUtilityActionSelected;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On Utility Action Ticked"))
	FUtilityActionSelected OnUtilityActionTicked;

	UPROPERTY(BlueprintAssignable, Category = "Utility AI", meta = (DisplayName = "On Utility Pre-Scoring"))
	FUtilityPreScoring OnUtilityPreScoring;

	UPROPERTY()
	TSet<UUtilityAIAction*> InstancedActions;

	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	void TickUtilityAI();
	
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "Compute Best Action"))
    UUtilityAIAction* ReceiveComputeBestAction(AAIController* Controller, APawn* Pawn);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
	TArray<UUtilityAIAction*> GetActionInstances() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
	UUtilityAIAction* GetActionInstanceByClass(TSubclassOf<UUtilityAIAction> ActionClass) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
	UUtilityAIAction* GetCurrentActionInstance() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
	TSubclassOf<UUtilityAIAction> GetCurrentActionClass() const;

	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	UUtilityAIAction* SpawnActionInstance(TSubclassOf<UUtilityAIAction> ActionClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
	bool CanSpawnActionInstance(TSubclassOf<UUtilityAIAction> ActionClass) const;

	UFUNCTION(BlueprintCallable, Category = "Utility AI")
	void SetRandomStream(FRandomStream InRandomStream);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
	FRandomStream GetRandomStream() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Utility AI", meta = (DisplayName = "Score Filter"))
	float ScoreFilter(UUtilityAIAction* Action, float Score) const;

	float ScoreFilter_Implementation(UUtilityAIAction* Action, float Score) const { return Score; }
	UUtilityAIAction* ReceiveComputeBestAction_Implementation(AAIController* Controller, APawn* Pawn);
	UUtilityAIAction* ComputeBestAction(AAIController* Controller, APawn* Pawn);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	UPROPERTY()
	UUtilityAIAction* LastAction;

	UPROPERTY()
	APawn* LastPawn;

	UPROPERTY()
	float LastSwitchTime;

	UPROPERTY()
	bool bUseRandomStream;

	UPROPERTY()
	FRandomStream RandomStream;

	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void Evaluate();
	bool InternalRandBool() const;
	bool CheckLowestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const;
	bool CheckHighestScore(UUtilityAIAction* Current, UUtilityAIAction* Best) const;
};
