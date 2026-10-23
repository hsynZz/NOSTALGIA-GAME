#include "NostalgiaHomecomer.h"
#include "NostalgiaStateSubsystem.h"
#include "NostalgiaCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ANostalgiaHomecomer::ANostalgiaHomecomer()
{
    // One orchestrator updates perception/encounter before Director arbitration; no tick-order race.
    PrimaryActorTick.bCanEverTick = false;
    Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EncounterBody"));
    SetRootComponent(Body);
    Body->InitCapsuleSize(42, 190);
    Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Body->SetCollisionObjectType(ECC_WorldDynamic);
    Body->SetCollisionResponseToAllChannels(ECR_Ignore);
    Body->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Body->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    Body->SetGenerateOverlapEvents(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    const FVector Positions[] = {FVector(0,0,200), FVector(0,0,340), FVector(0,-32,180), FVector(0,32,180), FVector(0,-15,65), FVector(0,15,65)};
    const FVector Scales[] = {FVector(.55,.55,2.2), FVector(.42,.42,.6), FVector(.12,.12,2.4), FVector(.12,.12,2.4), FVector(.18,.18,1.7), FVector(.18,.18,1.7)};
    for (int32 Index = 0; Index < 6; ++Index)
    {
        auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Silhouette%d"), Index));
        Part->SetupAttachment(RootComponent);
        Part->SetStaticMesh(Sphere.Object);
        Part->SetRelativeLocation(Positions[Index] - FVector(0, 0, 190));
        Part->SetRelativeScale3D(Scales[Index]);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Parts.Add(Part);
    }
    SetActorHiddenInGame(true);
}
void ANostalgiaHomecomer::BeginPlay()
{
    Super::BeginPlay();
    Anchor = GetActorLocation();
    if (auto* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Silhouette.M_Silhouette")))
    {
        bPresentationReady = true;
        for (const auto& Part : Parts)
        {
            Part->SetMaterial(0, Material);
            bPresentationReady = bPresentationReady && Part->GetStaticMesh() != nullptr;
        }
    }
    if (!bPresentationReady) UE_LOG(LogNostalgia, Warning, TEXT("Homecomer presentation assets missing; sighting deferred."));
}
Nostalgia::Perception ANostalgiaHomecomer::Sense(const ANostalgiaCharacter* Player, bool bSafe) const
{
    Nostalgia::Perception Perception;
    if (!Player || !Player->Camera) return Perception;
    const FVector Eye = Player->Camera->GetComponentLocation();
    const FVector Target = GetActorLocation() + FVector(0, 0, 70);
    const FVector Direction = (Target - Eye).GetSafeNormal();
    Perception.distance = FVector::Dist2D(Player->GetActorLocation(), GetActorLocation());
    Perception.viewDot = FMath::Clamp(FVector::DotProduct(Player->Camera->GetForwardVector(), Direction), -1.0, 1.0);
    Perception.safe = bSafe;
    Perception.blocked = bBlocked;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(HomecomerSight), false, this);
    Query.AddIgnoredActor(Player);
    Perception.lineOfSight = !GetWorld()->LineTraceTestByChannel(Eye, Target, ECC_Visibility, Query);
    if (const auto* Controller = Cast<APlayerController>(Player->GetController()))
    {
        int32 Width = 0, Height = 0;
        FVector2D Screen;
        Controller->GetViewportSize(Width, Height);
        Perception.inView = Controller->ProjectWorldLocationToScreen(Target, Screen, true) &&
            Screen.X >= 16 && Screen.Y >= 16 && Screen.X < Width - 16 && Screen.Y < Height - 16;
    }
    return Perception;
}
bool ANostalgiaHomecomer::CanShow(const ANostalgiaCharacter* Player, bool bSafe) const
{
    return bPresentationReady && !bSafe && Encounter.phase == Nostalgia::EncounterPhase::Dormant &&
        Nostalgia::CanStartEncounter(Encounter, Sense(Player, bSafe));
}
bool ANostalgiaHomecomer::Show(const ANostalgiaCharacter* Player, bool bSafe)
{
    if (!bPresentationReady || !Nostalgia::StartEncounter(Encounter, Sense(Player, bSafe))) return false;
    SetActorHiddenInGame(false);
    return true;
}
void ANostalgiaHomecomer::ResetPresentation()
{
    Encounter = Nostalgia::Encounter{};
    bBlocked = false;
    SetActorLocation(Anchor);
    SetActorHiddenInGame(true);
}
void ANostalgiaHomecomer::UpdateEncounter(const ANostalgiaCharacter* Player, bool bSafe, float DeltaSeconds)
{
    if (!IsVisibleThreat()) return;
    const double Travel = Nostalgia::StepEncounter(Encounter, Sense(Player, bSafe), DeltaSeconds);
    if (Player && Travel != 0)
    {
        const FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
        FHitResult Hit;
        AddActorWorldOffset(Direction * Travel, true, &Hit);
        bBlocked = Hit.bBlockingHit; // A blocked encounter retires within two seconds, even against a wall.
    }
    SetActorHiddenInGame(!IsVisibleThreat());
}
