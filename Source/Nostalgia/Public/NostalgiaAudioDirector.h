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
    void Play(Nostalgia::EventId Event);
    void UpdateListener(const FVector& Location, Nostalgia::District District);
    void StopTransientAudio();
private:
    UPROPERTY() TObjectPtr<USoundBase> GardenCue;
    UPROPERTY() TObjectPtr<USoundBase> KnockCue;
    UPROPERTY() TObjectPtr<USoundAttenuation> Attenuation;
    UPROPERTY() TObjectPtr<UAudioComponent> ActiveGarden;
    UPROPERTY() TObjectPtr<UAudioComponent> ActiveKnock;
    const FVector GardenOrigin = FVector(2400, 1250, 110);
};
