#include "NostalgiaHouseInterior.h"
#include "NostalgiaCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"

ANostalgiaHouseInterior::ANostalgiaHouseInterior()
{
    Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalDoor"));
    SetRootComponent(Frame);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Frame->SetStaticMesh(Cube.Object);
    Frame->SetRelativeScale3D(FVector(1.4, 0.2, 2.4));
    Frame->SetCollisionProfileName(TEXT("BlockAll"));
}
FText ANostalgiaHouseInterior::InteractionText_Implementation() const { return FText::FromString(TEXT("E: ") + Prompt); }
void ANostalgiaHouseInterior::Interact_Implementation(APawn* InstigatorPawn)
{
    auto* Player = Cast<ANostalgiaCharacter>(InstigatorPawn);
    if (!Player || !Player->TransitionReady()) return;
    const FRotator View = Player->GetControlRotation();
    const float RelativeYaw = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, View.Yaw);
    const FRotator NewView(View.Pitch, DestinationYaw + RelativeYaw, 0);
    if (!Player->TeleportTo(Destination, FRotator(0, NewView.Yaw, 0), false, false)) return;
    Player->GetCharacterMovement()->StopMovementImmediately();
    if (Player->GetController()) Player->GetController()->SetControlRotation(NewView);
    Player->ProtectTransition();
}
