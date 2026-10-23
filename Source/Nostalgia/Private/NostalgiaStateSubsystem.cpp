#include "NostalgiaStateSubsystem.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaEventCatalog.h"
#include "NostalgiaSaveGame.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogNostalgia);
namespace { const FString SaveSlot = TEXT("Nostalgia_Prototype01"); }

void UNostalgiaStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    State = Nostalgia::State{};
    State.randomState = InitialSeed > 0 ? static_cast<uint32>(InitialSeed) : 2004;
}

void UNostalgiaStateSubsystem::SetContext(Nostalgia::District District, bool bSafe, bool bThreatened, float Speed, float DeltaSeconds)
{
    Context.dwell = Context.district == District ? Context.dwell + FMath::Max(0.0f, DeltaSeconds) : 0.0;
    Context.district = District;
    Context.safeZone = bSafe;
    Context.threatened = bThreatened;
    Context.speed = Speed;
}

Nostalgia::EventId UNostalgiaStateSubsystem::Step(float DeltaSeconds)
{
    if (State.stage == Nostalgia::Stage::Complete) return Nostalgia::EventId::Count;
    Nostalgia::Advance(State, DeltaSeconds);
    const auto Event = Nostalgia::SelectEvent(State, Context, Rules);
    if (Event != Nostalgia::EventId::Count)
    {
        LastEvent = UTF8_TO_TCHAR(Nostalgia::EventName(Event).data());
        UE_LOG(LogNostalgia, Log, TEXT("Event=%s Stage=%d Time=%.2f Seed=%u"), *LastEvent,
            static_cast<int32>(State.stage), State.elapsed, State.randomState);
    }
    return Event;
}

bool UNostalgiaStateSubsystem::Perform(Nostalgia::Action Action)
{
    const bool bChanged = Nostalgia::Apply(State, Action);
    LastMessage = bChanged ? ObjectiveText() : HintText();
    if (bChanged) UE_LOG(LogNostalgia, Log, TEXT("Quest=%d %s"), static_cast<int32>(State.stage), *LastMessage);
    return bChanged;
}

bool UNostalgiaStateSubsystem::TraverseLoop()
{
    const bool bChanged = Nostalgia::TraverseLoop(State);
    if (bChanged)
    {
        LastMessage = State.stage >= Nostalgia::Stage::RoadUnlocked ?
            TEXT("Die Erinnerung hat den Weg veraendert. Eine Laterne bleibt dunkel.") :
            TEXT("Wieder dieselbe Strasse. War diese Hausnummer vorher schon da?");
        UE_LOG(LogNostalgia, Log, TEXT("Loop=%u Quest=%d"), State.loops, static_cast<int32>(State.stage));
    }
    return bChanged;
}

void UNostalgiaStateSubsystem::ToggleDoor(uint8 DoorId)
{
    if (DoorId < 2) State.openDoors ^= static_cast<uint8>(1u << DoorId);
}

bool UNostalgiaStateSubsystem::IsDoorOpen(uint8 DoorId) const
{
    return DoorId < 2 && (State.openDoors & (1u << DoorId)) != 0;
}

void UNostalgiaStateSubsystem::Configure(const UNostalgiaEventCatalog* Catalog)
{
    if (Catalog && !Catalog->GetValidatedRules(Rules))
        UE_LOG(LogNostalgia, Error, TEXT("Invalid Dream Director catalog; validated C++ defaults retained."));
}

FString UNostalgiaStateSubsystem::ClockText() const
{
    const int32 Minute = FMath::FloorToInt(State.minute);
    return FString::Printf(TEXT("17.08.2004  %02d:%02d"), Minute / 60, Minute % 60);
}
FString UNostalgiaStateSubsystem::ObjectiveText() const { return UTF8_TO_TCHAR(Nostalgia::Objective(State.stage).data()); }
FString UNostalgiaStateSubsystem::HintText() const { return UTF8_TO_TCHAR(Nostalgia::Hint(State.stage).data()); }

bool UNostalgiaStateSubsystem::Save(ANostalgiaCharacter* Player)
{
    if (!Player || !Player->GetCharacterMovement()->IsMovingOnGround() || Player->bIsCrouched || !Nostalgia::Validate(State))
    {
        LastMessage = TEXT("Speichern: bitte auf festem Boden stehen und aufrichten.");
        return false;
    }
    auto* Data = Cast<UNostalgiaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNostalgiaSaveGame::StaticClass()));
    if (!Data) return false;
    Data->Version = Nostalgia::SaveVersion;
    Data->Stage = static_cast<uint8>(State.stage);
    Data->Minute = State.minute;
    Data->Elapsed = State.elapsed;
    Data->RandomState = State.randomState;
    Data->Loops = State.loops;
    Data->OpenDoors = State.openDoors;
    Data->LastTransition = State.lastTransition;
    Data->LastShock = State.lastShock;
    for (const auto& Event : State.history)
    {
        FNostalgiaSavedEvent Saved;
        Saved.Count = static_cast<int32>(Event.count);
        Saved.LastTime = Event.lastTime;
        Data->Events.Add(Saved);
    }
    Data->PlayerLocation = Player->GetActorLocation();
    Data->PlayerRotation = Player->GetControlRotation();
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Data, SaveSlot, 0);
    LastMessage = bSaved ? TEXT("Spielstand gespeichert.") : TEXT("Speichern fehlgeschlagen; siehe Log/Datentraeger.");
    UE_LOG(LogNostalgia, Log, TEXT("Save success=%d"), bSaved);
    return bSaved;
}

bool UNostalgiaStateSubsystem::Load(ANostalgiaCharacter* Player)
{
    const auto* Data = Cast<UNostalgiaSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
    LastMessage = TEXT("Laden abgelehnt: Spielstand fehlt, ist ungueltig oder inkompatibel.");
    if (!Player || !Data || Data->Version != Nostalgia::SaveVersion || Data->Chapter != TEXT("EndlessSummer2004") ||
        Data->Events.Num() != static_cast<int32>(Nostalgia::EventCount)) return false;
    Nostalgia::State Candidate;
    Candidate.stage = static_cast<Nostalgia::Stage>(Data->Stage);
    Candidate.minute = Data->Minute;
    Candidate.elapsed = Data->Elapsed;
    Candidate.randomState = Data->RandomState;
    Candidate.loops = Data->Loops;
    Candidate.openDoors = Data->OpenDoors;
    Candidate.lastTransition = Data->LastTransition;
    Candidate.lastShock = Data->LastShock;
    for (int32 Index = 0; Index < Data->Events.Num(); ++Index)
    {
        Candidate.history[Index].count = static_cast<uint32>(Data->Events[Index].Count);
        Candidate.history[Index].lastTime = Data->Events[Index].LastTime;
    }
    const FVector Location = Data->PlayerLocation;
    const FRotator Rotation = Data->PlayerRotation;
    if (!Nostalgia::Validate(Candidate) || Location.ContainsNaN() || Rotation.ContainsNaN() ||
        Location.X < -2400 || Location.X > 10000 || Location.Y < -2500 || Location.Y > 4500 ||
        Location.Z < 40 || Location.Z > 800) return false;
    // Validate placement before replacing live state. TeleportTo uses the capsule collision checks.
    if (Player->bIsCrouched || !Player->TeleportTo(Location, FRotator(0, Rotation.Yaw, 0), false, false))
    {
        LastMessage = TEXT("Laden: aufrichten / gespeicherte Position ist blockiert. Zustand unveraendert.");
        return false;
    }
    Player->GetCharacterMovement()->StopMovementImmediately();
    if (Player->GetController()) Player->GetController()->SetControlRotation(Rotation);
    Player->ProtectTransition();
    State = Candidate;
    Context = Nostalgia::Context{};
    LastEvent = TEXT("Load.Restored");
    LastMessage = TEXT("Spielstand geladen; Uhr, Quest, Schleifen, Tueren und Event-Historie wiederhergestellt.");
    ++LoadRevision;
    UE_LOG(LogNostalgia, Log, TEXT("Load success Quest=%d"), static_cast<int32>(State.stage));
    return true;
}
