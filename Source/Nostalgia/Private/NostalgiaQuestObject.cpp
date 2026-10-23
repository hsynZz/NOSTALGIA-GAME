#include "NostalgiaQuestObject.h"
#include "NostalgiaStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ANostalgiaQuestObject::ANostalgiaQuestObject()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractionMesh"));
    SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Cube.Object);
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Mesh);
    Label->SetAbsolute(false, false, true);
    Label->SetWorldSize(20);
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetTextRenderColor(FColor::Black);
}

void ANostalgiaQuestObject::Configure(Nostalgia::Action InAction, const FString& InLabel, FVector Size)
{
    Action = InAction;
    Prompt = InLabel;
    Mesh->SetWorldScale3D(Size / 100.0);
    Label->SetText(FText::FromString(InLabel));
    Label->SetWorldLocation(GetActorLocation() + FVector(-Size.X / 2 - 2, 0, Size.Z / 2 + 35));
    Label->SetWorldRotation(FRotator(0, 180, 0));
}

FText ANostalgiaQuestObject::InteractionText_Implementation() const { return FText::FromString(TEXT("E: ") + Prompt); }
void ANostalgiaQuestObject::Interact_Implementation(APawn* InstigatorPawn)
{
    if (InstigatorPawn) GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>()->Perform(Action);
}
