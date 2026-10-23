#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NostalgiaInteraction.generated.h"

class APawn;
UINTERFACE(BlueprintType)
class NOSTALGIA_API UNostalgiaInteraction : public UInterface { GENERATED_BODY() };

class NOSTALGIA_API INostalgiaInteraction
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable) FText InteractionText() const;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void Interact(APawn* InstigatorPawn);
};
