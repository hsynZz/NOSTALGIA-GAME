#include "NostalgiaStateSubsystem.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaDoor.h"
#include "NostalgiaEventCatalog.h"
#include "NostalgiaSaveGame.h"
#include "Core/NostalgiaSnapshot.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ScopeExit.h"

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
    PendingEvent = Nostalgia::EventPlan{};
    if (State.stage == Nostalgia::Stage::Complete) return Nostalgia::EventId::Count;
    Nostalgia::Advance(State, DeltaSeconds);
    PendingEvent = Nostalgia::PlanEvent(State, Context, Rules);
    return PendingEvent.GetEvent();
}

void UNostalgiaStateSubsystem::SetEventAvailable(Nostalgia::EventId Event, bool bAvailable)
{
    if (Event < Nostalgia::EventId::Count) Context.available[static_cast<size_t>(Event)] = bAvailable;
}

bool UNostalgiaStateSubsystem::AcknowledgeEvent(bool bDelivered)
{
    const auto Event = PendingEvent.GetEvent();
    const bool bCommitted = Nostalgia::CommitEvent(State, PendingEvent, bDelivered);
    PendingEvent = Nostalgia::EventPlan{};
    if (bCommitted)
    {
        LastEvent = UTF8_TO_TCHAR(Nostalgia::EventName(Event).data());
        UE_LOG(LogNostalgia, Log, TEXT("Event=%s Stage=%d Time=%.2f Seed=%u"), *LastEvent,
            static_cast<int32>(State.stage), State.elapsed, State.randomState);
    }
    return bCommitted;
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
    if (!Player || Player->IsRestoringSnapshot() || !Player->GetCharacterMovement()->IsMovingOnGround() || Player->bIsCrouched || !Nostalgia::Validate(State))
    {
        LastMessage = TEXT("Speichern: bitte auf festem Boden stehen und aufrichten.");
        return false;
    }
    auto* Data = Cast<UNostalgiaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNostalgiaSaveGame::StaticClass()));
    if (!Data) return false;
    const FVector Location = Player->GetActorLocation();
    const FRotator Rotation = Player->GetControlRotation();
    const Nostalgia::SaveSnapshot Snapshot{State, {Location.X, Location.Y, Location.Z}, {Rotation.Pitch, Rotation.Yaw, Rotation.Roll}};
    if (!Nostalgia::ValidateSnapshot(Snapshot))
    {
        LastMessage = TEXT("Speichern abgelehnt: Position oder Fortschritt passt nicht zur vorbereiteten Welt.");
        return false;
    }
    // Save the intended stable door pose, not an animation frame that could make this save unloadable.
    const double CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const double CapsuleHalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    for (TActorIterator<ANostalgiaDoor> It(Player->GetWorld()); It; ++It)
        if (It->OverlapsSavedPose(Location, IsDoorOpen(It->DoorId), CapsuleRadius, CapsuleHalfHeight))
        {
            LastMessage = TEXT("Speichern: bitte aus dem Schwenkbereich der Tuer treten.");
            return false;
        }
    if (!Data->FromSnapshot(Snapshot))
    {
        LastMessage = TEXT("Speichern abgelehnt: Snapshot konnte nicht erstellt werden.");
        return false;
    }
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Data, SaveSlot, 0);
    LastMessage = bSaved ? TEXT("Spielstand gespeichert.") : TEXT("Speichern fehlgeschlagen; siehe Log/Datentraeger.");
    UE_LOG(LogNostalgia, Log, TEXT("Save success=%d"), bSaved);
    return bSaved;
}

bool UNostalgiaStateSubsystem::Load(ANostalgiaCharacter* Player)
{
    const auto* Data = Cast<UNostalgiaSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
    LastMessage = TEXT("Laden abgelehnt: Spielstand fehlt, ist ungueltig oder inkompatibel.");
    Nostalgia::SaveSnapshot Candidate;
    if (!Player || !Data || Player->IsRestoringSnapshot() || Player->bIsCrouched || !Data->ToSnapshot(Candidate)) return false;
    const FVector Location(Candidate.position.x, Candidate.position.y, Candidate.position.z);
    const FRotator Rotation(Candidate.rotation.x, Candidate.rotation.y, Candidate.rotation.z);
    const double CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const double CapsuleHalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    struct FDoorPlacement
    {
        TWeakObjectPtr<ANostalgiaDoor> Door;
        ECollisionEnabled::Type Collision;
        bool bOpen;
    };
    TArray<FDoorPlacement> Doors;
    for (TActorIterator<ANostalgiaDoor> It(Player->GetWorld()); It; ++It)
    {
        const bool bOpen = It->DoorId < 2 && (Candidate.state.openDoors & (1u << It->DoorId)) != 0;
        if (It->OverlapsSavedPose(Location, bOpen, CapsuleRadius, CapsuleHalfHeight))
        {
            LastMessage = TEXT("Laden abgelehnt: gespeicherte Position liegt im gespeicherten Tuerblatt.");
            return false;
        }
        Doors.Add({*It, It->Panel->GetCollisionEnabled(), bOpen});
    }

    const FVector OriginalLocation = Player->GetActorLocation();
    const FRotator OriginalActorRotation = Player->GetActorRotation();
    bool bRestored = false;
    Player->BeginSnapshotRestore();
    ON_SCOPE_EXIT { Player->EndSnapshotRestore(bRestored); };
    // Current door angles must not invalidate a valid saved doorway position. Other geometry remains blocking.
    for (const auto& Item : Doors) if (Item.Door.IsValid()) Item.Door->Panel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ON_SCOPE_EXIT
    {
        if (!bRestored)
            Player->SetActorLocationAndRotation(OriginalLocation, OriginalActorRotation, false, nullptr, ETeleportType::TeleportPhysics);
        for (const auto& Item : Doors) if (Item.Door.IsValid()) Item.Door->Panel->SetCollisionEnabled(Item.Collision);
    };
    LastMessage = TEXT("Laden abgelehnt: Zielposition ist blockiert. Zustand unveraendert.");
    if (!Player->TeleportTo(Location, FRotator(0, Rotation.Yaw, 0), false, false)) return false;
    // TeleportTo can adjust placement. Revalidate the actual position before enabling saved door collision.
    const FVector ActualLocation = Player->GetActorLocation();
    Candidate.position = {ActualLocation.X, ActualLocation.Y, ActualLocation.Z};
    if (!Nostalgia::ValidateSnapshot(Candidate)) return false;
    for (const auto& Item : Doors)
        if (Item.Door.IsValid() && Item.Door->OverlapsSavedPose(ActualLocation, Item.bOpen, CapsuleRadius, CapsuleHalfHeight)) return false;

    for (const auto& Item : Doors) if (Item.Door.IsValid()) Item.Door->ApplySavedPose(Item.bOpen);
    Player->GetCharacterMovement()->StopMovementImmediately();
    if (Player->GetController()) Player->GetController()->SetControlRotation(Rotation);
    State = Candidate.state;
    Context = Nostalgia::Context{};
    PendingEvent = Nostalgia::EventPlan{};
    LastEvent = TEXT("Load.Restored");
    LastMessage = TEXT("Spielstand geladen; Uhr, Quest, Schleifen, Tueren und Event-Historie wiederhergestellt.");
    ++LoadRevision;
    bRestored = true;
    UE_LOG(LogNostalgia, Log, TEXT("Load success Quest=%d"), static_cast<int32>(State.stage));
    return true;
}
