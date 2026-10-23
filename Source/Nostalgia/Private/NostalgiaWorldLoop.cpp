#include "NostalgiaWorldLoop.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

ANostalgiaWorldLoop::ANostalgiaWorldLoop()
{
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StreetBoundary"));
    SetRootComponent(Trigger);
    Trigger->SetBoxExtent(FVector(100, 470, 160));
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Trigger->SetGenerateOverlapEvents(true);
    Trigger->OnComponentBeginOverlap.AddDynamic(this, &ANostalgiaWorldLoop::Enter);
}

void ANostalgiaWorldLoop::Enter(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    auto* Player = Cast<ANostalgiaCharacter>(Other);
    if (!Player || !Player->TransitionReady()) return;
    auto* State = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    // Preflight the exact core operation, commit it only after successful placement.
    auto Candidate = State->GetState();
    if (!Nostalgia::TraverseLoop(Candidate)) return;
    const bool bUnlocked = Candidate.stage >= Nostalgia::Stage::RoadUnlocked;
    // X=600 is now the residential front door. The loop lands safely outside the house.
    const FVector Target(bUnlocked ? 7200 : 950, FMath::Clamp(Player->GetActorLocation().Y, -250.0, 250.0), 96);
    const FRotator View = Player->GetControlRotation();
    if (!Player->TeleportTo(Target, FRotator(0, View.Yaw, 0), false, false)) return;
    Player->GetCharacterMovement()->StopMovementImmediately();
    if (Player->GetController()) Player->GetController()->SetControlRotation(View);
    Player->ProtectTransition();
    State->TraverseLoop();
}
