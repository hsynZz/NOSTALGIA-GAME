#include "NostalgiaWorldLoop.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaStateSubsystem.h"
#include "Core/NostalgiaWorldLayout.h"
#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

ANostalgiaWorldLoop::ANostalgiaWorldLoop()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickInterval = .1f;
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StreetBoundary"));
    SetRootComponent(Trigger);
    const auto Extent = Nostalgia::Layout::LoopTriggerHalfExtent;
    Trigger->SetBoxExtent(FVector(Extent.x, Extent.y, Extent.z));
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &ANostalgiaWorldLoop::Enter);
}

void ANostalgiaWorldLoop::Enter(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (auto* Player = Cast<ANostalgiaCharacter>(Other))
    {
        // A save/load placement must not enqueue a deferred traversal from its overlap callbacks.
        if (Player->IsRestoringSnapshot()) return;
        PendingPlayer = Player;
        SetActorTickEnabled(true);
        TryTransition();
    }
}

void ANostalgiaWorldLoop::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TryTransition();
}

void ANostalgiaWorldLoop::TryTransition()
{
    auto* Player = PendingPlayer.Get();
    if (!Player || !Trigger->IsOverlappingActor(Player))
    {
        PendingPlayer.Reset();
        SetActorTickEnabled(false);
        return;
    }
    // An overlap during the transition cooldown is retried while still inside the boundary.
    if (!Player->TransitionReady()) return;
    auto* State = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    // Preflight the exact core operation, commit it only after successful placement.
    auto Candidate = State->GetState();
    if (!Nostalgia::TraverseLoop(Candidate)) return;
    const bool bUnlocked = Candidate.stage >= Nostalgia::Stage::RoadUnlocked;
    // X=600 is now the residential front door. The loop lands safely outside the house.
    const auto Landing = bUnlocked ? Nostalgia::Layout::LoopUnlockedLanding : Nostalgia::Layout::LoopLockedLanding;
    const FVector Target(Landing.x, FMath::Clamp(Player->GetActorLocation().Y, -250.0, 250.0), Landing.z);
    const FRotator View = Player->GetControlRotation();
    if (!Player->TeleportTo(Target, FRotator(0, View.Yaw, 0), false, false)) return;
    Player->GetCharacterMovement()->StopMovementImmediately();
    if (Player->GetController()) Player->GetController()->SetControlRotation(View);
    Player->ProtectTransition();
    State->TraverseLoop();
    PendingPlayer.Reset();
    SetActorTickEnabled(false);
}
