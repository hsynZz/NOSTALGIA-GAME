#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaInteraction.h"
#include "NostalgiaQualitySlice.generated.h"

class UNostalgiaAudioDirector;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class NOSTALGIA_API ANostalgiaSliceCharacter : public ANostalgiaCharacter
{
    GENERATED_BODY()
public:
    ANostalgiaSliceCharacter();
};

// Deliberately does not inherit the blockout's auto-spawning game mode.
UCLASS()
class NOSTALGIA_API ANostalgiaSliceGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ANostalgiaSliceGameMode();
};

UCLASS()
class NOSTALGIA_API ANostalgiaSliceDirector : public AActor
{
    GENERATED_BODY()
public:
    ANostalgiaSliceDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UNostalgiaAudioDirector> Audio;
private:
    uint32 SeenLoadRevision = 0;
};

UCLASS()
class NOSTALGIA_API ANostalgiaSliceDoor : public AActor, public INostalgiaInteraction
{
    GENERATED_BODY()
public:
    ANostalgiaSliceDoor();
    virtual void Tick(float DeltaSeconds) override;
    virtual FText InteractionText_Implementation() const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USceneComponent> Hinge;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Leaf;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UBoxComponent> Collision;
private:
    bool bOpen = false;
};
