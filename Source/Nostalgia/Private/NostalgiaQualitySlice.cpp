#include "NostalgiaQualitySlice.h"
#include "NostalgiaAudioDirector.h"
#include "NostalgiaEventCatalog.h"
#include "NostalgiaHUD.h"
#include "NostalgiaStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ANostalgiaSliceCharacter::ANostalgiaSliceCharacter()
{
    bPrototypeSaveEnabled = false;
    WalkSpeed = 220;
    SprintSpeed = 380;
}

ANostalgiaSliceGameMode::ANostalgiaSliceGameMode()
{
    DefaultPawnClass = ANostalgiaSliceCharacter::StaticClass();
    HUDClass = ANostalgiaHUD::StaticClass();
}

ANostalgiaSliceDirector::ANostalgiaSliceDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Audio = CreateDefaultSubobject<UNostalgiaAudioDirector>(TEXT("AudioDirector"));
    Audio->bAllowPrototypeSignals = false;
    Audio->GardenOrigin = FVector(-1290, -490, 110);
}

void ANostalgiaSliceDirector::BeginPlay()
{
    Super::BeginPlay();
    auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    // Use the existing transaction-based Dream Director. This scene delivers only the first garden event.
    auto* Catalog = NewObject<UNostalgiaEventCatalog>(this);
    for (auto& Rule : Catalog->Rules)
    {
        if (Rule.Event == ENostalgiaEvent::GardenLaughter)
        {
            Rule.MinimumDwell = 2.4f;
            Rule.MaximumSpeed = 200;
            Rule.MaximumPlays = 1;
        }
    }
    Session->Configure(Catalog);
    SeenLoadRevision = Session->LoadRevision;
}

void ANostalgiaSliceDirector::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Player = Cast<ANostalgiaCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    const auto Location = Player->GetActorLocation();
    const bool bInGarden = Location.Y < -200 && Location.Y > -730 && Location.X > -1800 && Location.X < 300;
    const auto District = bInGarden ? Nostalgia::District::Garden : Nostalgia::District::Bedroom;
    const float Step = FMath::Clamp(DeltaSeconds, 0.0f, .1f);
    if (SeenLoadRevision != Session->LoadRevision)
    {
        Audio->StopTransientAudio();
        SeenLoadRevision = Session->LoadRevision;
    }
    Audio->UpdateListener(Location, District);
    Session->SetContext(District, !bInGarden, false, Player->GetVelocity().Size2D(), Step);
    Session->SetEventAvailable(Nostalgia::EventId::GardenLaughter,
        !Player->bMemoryVisible && Audio->CanPlay(Nostalgia::EventId::GardenLaughter, Location));
    Session->SetEventAvailable(Nostalgia::EventId::HomecomerSighting, false);
    Session->SetEventAvailable(Nostalgia::EventId::WindowKnock, false);
    const auto Event = Session->Step(Step, 1.0f / 60.0f); // Real-time summer afternoon.
    if (Event != Nostalgia::EventId::Count)
    {
        const bool bStarted = Audio->Play(Event);
        if (!Session->AcknowledgeEvent(bStarted) && bStarted) Audio->StopTransientAudio();
    }
    if (Location.Z < -250)
    {
        if (Player->TeleportTo(FVector(-1220, 220, 92), FRotator(0, -145, 0), false, false))
            Player->GetCharacterMovement()->StopMovementImmediately();
    }
}

ANostalgiaSliceDoor::ANostalgiaSliceDoor()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
    Hinge->SetupAttachment(RootComponent);
    Leaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AuthoredDoorLeaf"));
    Leaf->SetupAttachment(Hinge);
    Leaf->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorCollision"));
    Collision->SetupAttachment(Hinge);
    Collision->SetBoxExtent(FVector(7, 47.5f, 105));
    Collision->SetRelativeLocation(FVector(0, -47.5f, 105));
    Collision->SetCollisionProfileName(TEXT("BlockAll"));
}

FText ANostalgiaSliceDoor::InteractionText_Implementation() const
{
    return FText::FromString(bOpen ? TEXT("Tuer schliessen") : TEXT("Tuer oeffnen"));
}

void ANostalgiaSliceDoor::Interact_Implementation(APawn* InstigatorPawn)
{
    if (IsValid(InstigatorPawn)) bOpen = !bOpen;
}

void ANostalgiaSliceDoor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // Sample the swept arc in <=2 degree increments. A paused frame cannot push the leaf through a pawn.
    float Remaining = FMath::Clamp(DeltaSeconds, 0.0f, .1f);
    while (Remaining > SMALL_NUMBER)
    {
        const float Step = FMath::Min(Remaining, .02f);
        Remaining -= Step;
        const float Next = FMath::FInterpConstantTo(Hinge->GetRelativeRotation().Yaw, bOpen ? 90.0f : 0.0f, Step, 80);
        const FQuat Rotation = GetActorQuat() * FRotator(0, Next, 0).Quaternion();
        const FVector Center = GetActorLocation() + Rotation.RotateVector(FVector(0, -47.5f, 105));
        FCollisionObjectQueryParams Pawns(ECC_Pawn);
        FCollisionQueryParams Query(SCENE_QUERY_STAT(NostalgiaSliceDoor), false, this);
        if (GetWorld()->OverlapAnyTestByObjectType(Center, Rotation, Pawns,
            FCollisionShape::MakeBox(FVector(9, 49.5f, 107)), Query)) break;
        Hinge->SetRelativeRotation(FRotator(0, Next, 0));
    }
}
