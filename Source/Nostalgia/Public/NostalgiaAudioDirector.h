#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/NostalgiaCore.h"
#include "NostalgiaAudioDirector.generated.h"

class UAudioComponent;
class USoundBase;
class USoundAttenuation;
UCLASS(ClassGroup=(Nostalgia))
class NOSTALGIA_API UNostalgiaAudioDirector : public UActorComponent
{
    GENERATED_BODY()
public:
    UNostalgiaAudioDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    bool CanPlay(Nostalgia::EventId Event, const FVector& Location) const;
    bool Play(Nostalgia::EventId Event);
    void UpdateListener(const FVector& Location, Nostalgia::District District);
    void StopTransientAudio();
    UPROPERTY(EditAnywhere, Category="Nostalgia|Audio") TObjectPtr<USoundBase> GardenCue;
    UPROPERTY(EditAnywhere, Category="Nostalgia|Audio") TObjectPtr<USoundBase> KnockCue;
    UPROPERTY(EditAnywhere, Category="Nostalgia|Audio") bool bAllowPrototypeSignals = true;
    UPROPERTY(EditAnywhere, Category="Nostalgia|Audio") FVector GardenOrigin = FVector(2400, 1250, 110);
    UPROPERTY(EditAnywhere, Category="Nostalgia|Audio") FVector KnockOrigin = FVector(-900, 3650, 150);
private:
    UPROPERTY() TObjectPtr<USoundAttenuation> Attenuation;
    UPROPERTY() TObjectPtr<UAudioComponent> ActiveGarden;
    UPROPERTY() TObjectPtr<UAudioComponent> ActiveKnock;
    bool bGardenFading = false;
    bool bKnockFading = false;
};
