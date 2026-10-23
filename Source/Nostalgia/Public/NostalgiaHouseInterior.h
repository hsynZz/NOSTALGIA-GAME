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
    UPROPERTY(EditAnywhere, meta=(ClampMin="0", ClampMax="6")) uint8 MinimumStage = 0;
    UPROPERTY(EditAnywhere) FString LockedPrompt = TEXT("Der Weg ist noch nicht offen.");
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Frame;
};
