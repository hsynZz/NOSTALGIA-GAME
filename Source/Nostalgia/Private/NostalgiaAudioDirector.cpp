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

bool UNostalgiaAudioDirector::CanPlay(Nostalgia::EventId Event, const FVector& Location) const
{
    if (Event == Nostalgia::EventId::GardenLaughter)
    {
        const double Distance = FVector::Dist(Location, GardenOrigin);
        return GardenCue && !IsValid(ActiveGarden) && Distance >= 300 && Distance < 1100;
    }
    if (Event == Nostalgia::EventId::WindowKnock)
        return KnockCue && !IsValid(ActiveKnock) && FVector::Dist(Location, KnockOrigin) < 1100;
    return false;
}

bool UNostalgiaAudioDirector::Play(Nostalgia::EventId Event)
{
    if (Event == Nostalgia::EventId::GardenLaughter && GardenCue && !IsValid(ActiveGarden))
    {
        bGardenFading = false;
        ActiveGarden = UGameplayStatics::SpawnSoundAtLocation(this, GardenCue, GardenOrigin, FRotator::ZeroRotator,
            0.4f, 1.0f, 0.0f, Attenuation, nullptr, false);
        return IsValid(ActiveGarden) && ActiveGarden->IsPlaying();
    }
    if (Event == Nostalgia::EventId::WindowKnock && KnockCue && !IsValid(ActiveKnock))
    {
        bKnockFading = false;
        ActiveKnock = UGameplayStatics::SpawnSoundAtLocation(this, KnockCue, KnockOrigin, FRotator::ZeroRotator,
            0.5f, 1.0f, 0.0f, Attenuation, nullptr, false);
        return IsValid(ActiveKnock) && ActiveKnock->IsPlaying();
    }
    return false;
}

void UNostalgiaAudioDirector::UpdateListener(const FVector& Location, Nostalgia::District District)
{
    // Release finished components before a future data-driven repeat. FadeOut is issued once.
    for (auto* Slot : {&ActiveGarden, &ActiveKnock})
        if (IsValid(Slot->Get()) && !(*Slot)->IsPlaying())
        {
            (*Slot)->DestroyComponent();
            *Slot = nullptr;
        }
    if (IsValid(ActiveGarden) && !bGardenFading &&
        (District != Nostalgia::District::Garden || FVector::Dist(Location, GardenOrigin) < 300))
    {
        bGardenFading = true;
        ActiveGarden->FadeOut(0.35f, 0.0f);
    }
    if (IsValid(ActiveKnock) && !bKnockFading && District != Nostalgia::District::Interior)
    {
        bKnockFading = true;
        ActiveKnock->FadeOut(0.2f, 0.0f);
    }
}

void UNostalgiaAudioDirector::StopTransientAudio()
{
    if (IsValid(ActiveGarden)) { ActiveGarden->Stop(); ActiveGarden->DestroyComponent(); }
    if (IsValid(ActiveKnock)) { ActiveKnock->Stop(); ActiveKnock->DestroyComponent(); }
    ActiveGarden = nullptr;
    ActiveKnock = nullptr;
    bGardenFading = false;
    bKnockFading = false;
}

void UNostalgiaAudioDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopTransientAudio();
    Super::EndPlay(EndPlayReason);
}
