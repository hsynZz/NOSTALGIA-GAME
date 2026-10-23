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
private:
    UPROPERTY() TObjectPtr<USoundBase> GardenCue;
    UPROPERTY() TObjectPtr<USoundBase> KnockCue;
    UPROPERTY() TObjectPtr<USoundAttenuation> Attenuation;
    UPROPERTY() TObjectPtr<UAudioComponent> ActiveGarden;
    UPROPERTY() TObjectPtr<UAudioComponent> ActiveKnock;
    bool bGardenFading = false;
    bool bKnockFading = false;
    const FVector GardenOrigin = FVector(2400, 1250, 110);
    const FVector KnockOrigin = FVector(-900, 3650, 150);
};
