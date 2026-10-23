#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NostalgiaWorldLoop.generated.h"

class UBoxComponent;
UCLASS()
class NOSTALGIA_API ANostalgiaWorldLoop : public AActor
{
    GENERATED_BODY()
public:
    ANostalgiaWorldLoop();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Trigger;
private:
    UFUNCTION() void Enter(UPrimitiveComponent* Component, AActor* Other, UPrimitiveComponent* OtherComponent,
        int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);
};
