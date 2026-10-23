#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NostalgiaInteraction.h"
#include "Core/NostalgiaCore.h"
#include "NostalgiaQuestObject.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
UCLASS()
class NOSTALGIA_API ANostalgiaQuestObject : public AActor, public INostalgiaInteraction
{
    GENERATED_BODY()
public:
    ANostalgiaQuestObject();
    void Configure(Nostalgia::Action InAction, const FString& InLabel, FVector Size);
    virtual FText InteractionText_Implementation() const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
private:
    Nostalgia::Action Action = Nostalgia::Action::Photo;
    FString Prompt;
};
