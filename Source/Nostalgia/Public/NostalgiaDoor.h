#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NostalgiaInteraction.h"
#include "NostalgiaDoor.generated.h"

class UStaticMeshComponent;
UCLASS()
class NOSTALGIA_API ANostalgiaDoor : public AActor, public INostalgiaInteraction
{
    GENERATED_BODY()
public:
    ANostalgiaDoor();
    virtual void Tick(float DeltaSeconds) override;
    virtual FText InteractionText_Implementation() const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
    UPROPERTY(EditAnywhere) uint8 DoorId = 0;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Hinge;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Panel;
};
