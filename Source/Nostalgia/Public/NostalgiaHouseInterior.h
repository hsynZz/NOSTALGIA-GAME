#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NostalgiaInteraction.h"
#include "NostalgiaHouseInterior.generated.h"

class UStaticMeshComponent;
UCLASS()
class NOSTALGIA_API ANostalgiaHouseInterior : public AActor, public INostalgiaInteraction
{
    GENERATED_BODY()
public:
    ANostalgiaHouseInterior();
    virtual FText InteractionText_Implementation() const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
    UPROPERTY(EditAnywhere) FVector Destination = FVector::ZeroVector;
    UPROPERTY(EditAnywhere) float DestinationYaw = 0;
    UPROPERTY(EditAnywhere) FString Prompt = TEXT("Haus betreten");
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Frame;
};
