#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NostalgiaHomecomer.generated.h"

class UStaticMeshComponent;
UENUM()
enum class EHomecomerState : uint8 { Dormant, Observing, Retreating, Cooldown };

UCLASS()
class NOSTALGIA_API ANostalgiaHomecomer : public AActor
{
    GENERATED_BODY()
public:
    ANostalgiaHomecomer();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void Show();
    void ResetPresentation();
    EHomecomerState GetEntityState() const { return State; }
    bool IsVisibleThreat() const { return State == EHomecomerState::Observing || State == EHomecomerState::Retreating; }
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    EHomecomerState State = EHomecomerState::Dormant;
    float Remaining = 0;
    FVector Anchor;
};
