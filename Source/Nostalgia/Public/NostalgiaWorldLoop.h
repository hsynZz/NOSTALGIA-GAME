#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NostalgiaWorldLoop.generated.h"

class UBoxComponent;
class ANostalgiaCharacter;
UCLASS()
class NOSTALGIA_API ANostalgiaWorldLoop : public AActor
{
    GENERATED_BODY()
public:
    ANostalgiaWorldLoop();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Trigger;
private:
    UPROPERTY() TWeakObjectPtr<ANostalgiaCharacter> PendingPlayer;
    void TryTransition();
    UFUNCTION() void Enter(UPrimitiveComponent* Component, AActor* Other, UPrimitiveComponent* OtherComponent,
        int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);
};
