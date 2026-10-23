#include "NostalgiaDoor.h"
#include "NostalgiaStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ANostalgiaDoor::ANostalgiaDoor()
{
    PrimaryActorTick.bCanEverTick = true;
    Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
    SetRootComponent(Hinge);
    Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanel"));
    Panel->SetupAttachment(Hinge);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Panel->SetStaticMesh(Cube.Object);
    Panel->SetRelativeLocation(FVector(0, 110, 120));
    Panel->SetRelativeScale3D(FVector(0.2, 2.2, 2.4));
    Panel->SetCollisionProfileName(TEXT("BlockAll"));
}

void ANostalgiaDoor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const auto* State = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    const float Target = State->IsDoorOpen(DoorId) ? -90.0f : 0.0f;
    if (FMath::IsNearlyEqual(Hinge->GetRelativeRotation().Yaw, Target)) return;
    const float NextYaw = FMath::FInterpConstantTo(Hinge->GetRelativeRotation().Yaw, Target, DeltaSeconds, 100);
    const FQuat NextRotation = FRotator(0, NextYaw, 0).Quaternion();
    const FVector Center = GetActorLocation() + NextRotation.RotateVector(FVector(0, 110, 120));
    FCollisionObjectQueryParams Pawns(ECC_Pawn);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(NostalgiaDoor), false, this);
    if (!GetWorld()->OverlapAnyTestByObjectType(Center, NextRotation, Pawns,
            FCollisionShape::MakeBox(FVector(12, 110, 120)), Query))
        Hinge->SetRelativeRotation(FRotator(0, NextYaw, 0));
}
FText ANostalgiaDoor::InteractionText_Implementation() const { return FText::FromString(TEXT("E: Tuer oeffnen / schliessen")); }
void ANostalgiaDoor::Interact_Implementation(APawn* InstigatorPawn)
{
    if (InstigatorPawn) GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>()->ToggleDoor(DoorId);
}
