#include "NostalgiaHomecomer.h"
#include "NostalgiaStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ANostalgiaHomecomer::ANostalgiaHomecomer()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Anchor")));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    const FVector Positions[] = {FVector(0,0,200), FVector(0,0,340), FVector(0,-32,180), FVector(0,32,180), FVector(0,-15,65), FVector(0,15,65)};
    const FVector Scales[] = {FVector(.55,.55,2.2), FVector(.42,.42,.6), FVector(.12,.12,2.4), FVector(.12,.12,2.4), FVector(.18,.18,1.7), FVector(.18,.18,1.7)};
    for (int32 Index = 0; Index < 6; ++Index)
    {
        auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Silhouette%d"), Index));
        Part->SetupAttachment(RootComponent);
        Part->SetStaticMesh(Sphere.Object);
        Part->SetRelativeLocation(Positions[Index]);
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
        for (const auto& Part : Parts) Part->SetMaterial(0, Material);
}
void ANostalgiaHomecomer::Show()
{
    if (State != EHomecomerState::Dormant) return;
    SetActorLocation(Anchor);
    State = EHomecomerState::Observing;
    Remaining = 7;
    SetActorHiddenInGame(false);
}
void ANostalgiaHomecomer::ResetPresentation()
{
    State = EHomecomerState::Dormant;
    Remaining = 0;
    SetActorLocation(Anchor);
    SetActorHiddenInGame(true);
}
void ANostalgiaHomecomer::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (State == EHomecomerState::Dormant || State == EHomecomerState::Cooldown) return;
    const auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    if (Session->GetContext().safeZone || Session->GetState().stage == Nostalgia::Stage::Complete)
    {
        SetActorHiddenInGame(true);
        State = EHomecomerState::Cooldown;
        return;
    }
    Remaining -= DeltaSeconds;
    if (Remaining <= 0)
    {
        if (State == EHomecomerState::Observing) { State = EHomecomerState::Retreating; Remaining = 2; }
        else { State = EHomecomerState::Cooldown; SetActorHiddenInGame(true); }
    }
    if (State == EHomecomerState::Retreating) AddActorWorldOffset(FVector(0, 80 * DeltaSeconds, 0));
    // Bounded duration is intentional: no navigation, pursuit or damage in Prototype 0.1.
}
