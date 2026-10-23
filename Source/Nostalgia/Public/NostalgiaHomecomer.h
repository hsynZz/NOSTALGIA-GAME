#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/NostalgiaEncounter.h"
#include "NostalgiaHomecomer.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;
class ANostalgiaCharacter;

UCLASS()
class NOSTALGIA_API ANostalgiaHomecomer : public AActor
{
    GENERATED_BODY()
public:
    ANostalgiaHomecomer();
    virtual void BeginPlay() override;
    bool CanShow(const ANostalgiaCharacter* Player, bool bSafe) const;
    bool Show(const ANostalgiaCharacter* Player, bool bSafe);
    void UpdateEncounter(const ANostalgiaCharacter* Player, bool bSafe, float DeltaSeconds);
    void ResetPresentation();
    Nostalgia::EncounterPhase GetEntityState() const { return Encounter.phase; }
    bool IsVisibleThreat() const { return Nostalgia::EncounterActive(Encounter); }
private:
    UPROPERTY() TObjectPtr<UCapsuleComponent> Body;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    Nostalgia::Encounter Encounter;
    Nostalgia::Perception Sense(const ANostalgiaCharacter* Player, bool bSafe) const;
    bool bBlocked = false;
    bool bPresentationReady = false;
    FVector Anchor;
};
