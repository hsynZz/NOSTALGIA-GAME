#include "NostalgiaAudioDirector.h"
#include "NostalgiaStateSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

UNostalgiaAudioDirector::UNostalgiaAudioDirector() { PrimaryComponentTick.bCanEverTick = false; }
void UNostalgiaAudioDirector::BeginPlay()
{
    Super::BeginPlay();
    GardenCue = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/A_GardenPlaceholder.A_GardenPlaceholder"));
    KnockCue = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/A_KnockPlaceholder.A_KnockPlaceholder"));
    Attenuation = NewObject<USoundAttenuation>(this);
    auto& Settings = Attenuation->Attenuation;
    Settings.bAttenuate = true;
    Settings.bSpatialize = true;
    Settings.AttenuationShape = EAttenuationShape::Sphere;
    Settings.AttenuationShapeExtents = FVector(100, 0, 0);
    Settings.FalloffDistance = 1100;
    Settings.bEnableOcclusion = true;
    Settings.OcclusionTraceChannel = ECC_Visibility;
    Settings.OcclusionLowPassFilterFrequency = 1200;
    Settings.OcclusionVolumeAttenuation = 0.25f;
    if (!GardenCue || !KnockCue) UE_LOG(LogNostalgia, Warning, TEXT("Audio assets missing. Run Scripts/prepare_unreal.py in UE 5.8."));
}

void UNostalgiaAudioDirector::Play(Nostalgia::EventId Event)
{
    if (Event == Nostalgia::EventId::GardenLaughter && GardenCue)
        ActiveGarden = UGameplayStatics::SpawnSoundAtLocation(this, GardenCue, GardenOrigin, FRotator::ZeroRotator,
            0.4f, 1.0f, 0.0f, Attenuation, nullptr, false);
    if (Event == Nostalgia::EventId::WindowKnock && KnockCue)
        ActiveKnock = UGameplayStatics::SpawnSoundAtLocation(this, KnockCue, FVector(-900, 3650, 150), FRotator::ZeroRotator,
            0.5f, 1.0f, 0.0f, Attenuation, nullptr, false);
}

void UNostalgiaAudioDirector::UpdateListener(const FVector& Location, Nostalgia::District District)
{
    if (IsValid(ActiveGarden) && ActiveGarden->IsPlaying() &&
        (District != Nostalgia::District::Garden || FVector::Dist(Location, GardenOrigin) < 300))
        ActiveGarden->FadeOut(0.35f, 0.0f);
}

void UNostalgiaAudioDirector::StopTransientAudio()
{
    if (IsValid(ActiveGarden)) { ActiveGarden->Stop(); ActiveGarden->DestroyComponent(); }
    if (IsValid(ActiveKnock)) { ActiveKnock->Stop(); ActiveKnock->DestroyComponent(); }
    ActiveGarden = nullptr;
    ActiveKnock = nullptr;
}
