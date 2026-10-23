#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/NostalgiaCore.h"
#include "NostalgiaStateSubsystem.generated.h"

class ANostalgiaCharacter;
class UNostalgiaEventCatalog;
DECLARE_LOG_CATEGORY_EXTERN(LogNostalgia, Log, All);

UCLASS(Config=Game)
class NOSTALGIA_API UNostalgiaStateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    const Nostalgia::State& GetState() const { return State; }
    const Nostalgia::Context& GetContext() const { return Context; }
    void SetContext(Nostalgia::District District, bool bSafe, bool bThreatened, float Speed, float DeltaSeconds);
    Nostalgia::EventId Step(float DeltaSeconds);
    void SetEventAvailable(Nostalgia::EventId Event, bool bAvailable);
    bool AcknowledgeEvent(bool bDelivered);
    bool Perform(Nostalgia::Action Action);
    bool TraverseLoop();
    void ToggleDoor(uint8 DoorId);
    bool IsDoorOpen(uint8 DoorId) const;
    bool Save(ANostalgiaCharacter* Player);
    bool Load(ANostalgiaCharacter* Player);
    void Configure(const UNostalgiaEventCatalog* Catalog);
    FString ClockText() const;
    FString ObjectiveText() const;
    FString HintText() const;
    FString LastEvent = TEXT("None");
    FString LastMessage;
    FString EncounterDebug = TEXT("Dormant");
    // Incremented only on successful load; transient presentation resets on this revision.
    uint32 LoadRevision = 0;

private:
    UPROPERTY(Config) int32 InitialSeed = 2004;
    Nostalgia::State State;
    Nostalgia::Context Context;
    Nostalgia::EventPlan PendingEvent;
    std::array<Nostalgia::EventRule, Nostalgia::EventCount> Rules = Nostalgia::DefaultRules();
};
