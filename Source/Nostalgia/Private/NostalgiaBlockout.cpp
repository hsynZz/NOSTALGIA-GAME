#include "NostalgiaBlockout.h"
#include "NostalgiaAudioDirector.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaDoor.h"
#include "NostalgiaEventCatalog.h"
#include "NostalgiaHomecomer.h"
#include "NostalgiaHouseInterior.h"
#include "NostalgiaQuestObject.h"
#include "NostalgiaStateSubsystem.h"
#include "NostalgiaWorldLoop.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/GameInstance.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkyAtmosphere.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ANostalgiaBlockout::ANostalgiaBlockout()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("BlockoutRoot")));
    Audio = CreateDefaultSubobject<UNostalgiaAudioDirector>(TEXT("AudioDirector"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Cube = CubeAsset.Object;
}

UStaticMeshComponent* ANostalgiaBlockout::Box(const FVector& Center, const FVector& Size)
{
    auto* Mesh = NewObject<UStaticMeshComponent>(this, *FString::Printf(TEXT("Block_%d"), ComponentCounter++));
    AddInstanceComponent(Mesh);
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetStaticMesh(Cube);
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh->SetRelativeLocation(Center);
    Mesh->SetRelativeScale3D(Size / 100.0);
    Mesh->RegisterComponent();
    return Mesh;
}

UTextRenderComponent* ANostalgiaBlockout::Text(const FVector& Location, const FString& Value, float Yaw)
{
    auto* Label = NewObject<UTextRenderComponent>(this, *FString::Printf(TEXT("Label_%d"), ComponentCounter++));
    AddInstanceComponent(Label);
    Label->SetupAttachment(RootComponent);
    Label->SetRelativeLocation(Location);
    Label->SetRelativeRotation(FRotator(0, Yaw, 0));
    Label->SetText(FText::FromString(Value));
    Label->SetTextRenderColor(FColor::Black);
    Label->SetWorldSize(30);
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->RegisterComponent();
    return Label;
}

void ANostalgiaBlockout::Room(const FVector& Center, const FVector& Size, bool bDoorway, bool bRearDoorway, bool bWindow)
{
    Box(Center + FVector(0, 0, -10), FVector(Size.X, Size.Y, 20));
    Box(Center + FVector(0, -Size.Y/2, 160), FVector(Size.X, 20, 320));
    if (bWindow)
    {
        // An actual opening instead of a painted window; invisible collision prevents escaping through it.
        const float SideWidth = (Size.X - 300) / 2;
        for (float Sign : {-1.0f, 1.0f})
            Box(Center + FVector(Sign * (150 + SideWidth/2), Size.Y/2, 160), FVector(SideWidth, 20, 320));
        Box(Center + FVector(0, Size.Y/2, 45), FVector(300, 20, 90));
        Box(Center + FVector(0, Size.Y/2, 280), FVector(300, 20, 80));
        auto* WindowCollision = Box(Center + FVector(0, Size.Y/2, 165), FVector(300, 10, 150));
        WindowCollision->SetVisibility(false);
        WindowCollision->SetCastShadow(false);
    }
    else Box(Center + FVector(0, Size.Y/2, 160), FVector(Size.X, 20, 320));
    for (float End : {-1.0f, 1.0f})
    {
        const bool bOpen = End > 0 ? bDoorway : bRearDoorway;
        if (bOpen)
        {
            const float SideWidth = (Size.Y - 240) / 2;
            for (float Sign : {-1.0f, 1.0f})
                Box(Center + FVector(End * Size.X/2, Sign * (120 + SideWidth/2), 160), FVector(20, SideWidth, 320));
            Box(Center + FVector(End * Size.X/2, 0, 280), FVector(20, 240, 80));
        }
        else Box(Center + FVector(End * Size.X/2, 0, 160), FVector(20, Size.Y, 320));
    }
    Box(Center + FVector(0, 0, 330), FVector(Size.X, Size.Y, 20));
    auto* Light = NewObject<UPointLightComponent>(this);
    AddInstanceComponent(Light);
    Light->SetMobility(EComponentMobility::Movable);
    Light->SetupAttachment(RootComponent);
    Light->SetRelativeLocation(Center + FVector(0, 0, 270));
    Light->SetIntensity(2500);
    Light->SetAttenuationRadius(FMath::Min(1200.0, FMath::Max(Size.X, Size.Y)));
    Light->SetCastShadows(false);
    Light->SetLightColor(FLinearColor(1.0, 0.86, 0.63));
    Light->RegisterComponent();
}

void ANostalgiaBlockout::Build()
{
    Room(FVector(-1000, 0, 0), FVector(1600, 800, 320), true, false, true);
    // Continue the existing bedroom into a small residential hall/living room, then the front garden.
    Room(FVector(200, 0, 0), FVector(800, 800, 320), true, true);
    Box(FVector(100, 270, 40), FVector(280, 100, 80)); // sofa, clear of the central walking route
    Box(FVector(250, -240, 35), FVector(120, 90, 70)); // low table
    Text(FVector(0, 380, 210), TEXT("ZU HAUSE - SOMMER 2004"), -90);
    Box(FVector(-1300, -260, 35), FVector(300, 150, 70)); // bed
    Box(FVector(-850, 280, 45), FVector(200, 100, 90)); // desk
    Box(FVector(-600, 300, 110), FVector(100, 70, 80)); // CRT placeholder
    Box(FVector(850, 0, -10), FVector(500, 800, 20)); // front yard / porch
    Box(FVector(-600, 0, -30), FVector(2600, 1800, 20)); // ground visible through the bedroom window
    Box(FVector(3100, 0, -12), FVector(6200, 800, 24)); // street
    Box(FVector(3000, 0, -30), FVector(6400, 4600, 20)); // lawns
    // Walkable continuous ground under the destination; no need to jump at transitions.
    Box(FVector(8000, 0, -12), FVector(3000, 1400, 24));
    Box(FVector(6500, 0, 180), FVector(30, 1800, 360)); // separates curated route from free walking
    auto* Door = GetWorld()->SpawnActor<ANostalgiaDoor>(FVector(-200, -110, 0), FRotator::ZeroRotator);
    Door->DoorId = 0;
    auto* FrontDoor = GetWorld()->SpawnActor<ANostalgiaDoor>(FVector(600, -110, 0), FRotator::ZeroRotator);
    FrontDoor->DoorId = 1;
    Text(FVector(-1500, 380, 210), TEXT("ENDLESS SUMMER - 2004"), -90);

    const float HousesX[] = {1200, 3200, 5000};
    for (int32 Index = 0; Index < 3; ++Index)
        for (float Sign : {-1.0f, 1.0f})
        {
            const FVector Center(HousesX[Index], Sign * 1550, 250);
            Box(Center, FVector(800, 800, 500));
            Box(Center + FVector(0, 0, 270), FVector(900, 900, 40));
            Text(FVector(HousesX[Index], Sign * 1135, 280), FString::FromInt(10 + Index * 2 + (Sign > 0 ? 1 : 0)), Sign > 0 ? -90 : 90);
        }
    ChangedNumber = Text(FVector(4400, 450, 170), TEXT(""));
    Box(FVector(2200, 1100, -5), FVector(1100, 900, 10)); // birthday garden
    Text(FVector(1750, 650, 180), TEXT("GEBURTSTAGSGARTEN"));

    auto QuestObject = [this](FVector Location, FVector Size, Nostalgia::Action Action, const TCHAR* Label)
    {
        auto* Object = GetWorld()->SpawnActor<ANostalgiaQuestObject>(Location, FRotator::ZeroRotator);
        Object->Configure(Action, Label, Size);
    };
    QuestObject(FVector(-850, 260, 102), FVector(50, 40, 10), Nostalgia::Action::Photo, TEXT("Geburtstagsfoto ansehen"));
    QuestObject(FVector(2050, 900, 85), FVector(50, 100, 150), Nostalgia::Action::ComparePhoto, TEXT("Hier fehlt ein Haus - Foto vergleichen"));
    QuestObject(FVector(2400, 1100, 60), FVector(160, 220, 120), Nostalgia::Action::RestoreBirthday, TEXT("Geburtstagstisch: Erinnerung ordnen"));
    MemoryHouse = Box(FVector(2450, 2050, 200), FVector(450, 450, 400));
    MemoryHouse->SetVisibility(false);
    MemoryHouse->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Oversized curated interior, isolated from the outside geometry, with explicit entry/return anchors.
    Room(FVector(-1000, 3200, 0), FVector(2200, 1000, 320), false);
    Text(FVector(-1800, 3500, 180), TEXT("Dieses Haus ist innen zu lang."));
    auto* Entry = GetWorld()->SpawnActor<ANostalgiaHouseInterior>(FVector(1200, -1130, 120), FRotator(0, -90, 0));
    Entry->Destination = FVector(-1700, 3200, 96);
    Entry->DestinationYaw = 0;
    Entry->Prompt = TEXT("Haus 10 betreten");
    // Thin door plane faces north/south; rotate so it stays in front of the facade.
    Entry->Frame->SetWorldScale3D(FVector(.2, 1.4, 2.4));
    auto* Exit = GetWorld()->SpawnActor<ANostalgiaHouseInterior>(FVector(-1950, 3200, 120), FRotator(0, 180, 0));
    Exit->Frame->SetWorldScale3D(FVector(.2, 1.4, 2.4));
    Exit->Destination = FVector(1200, -900, 96);
    Exit->DestinationYaw = 90;
    Exit->Prompt = TEXT("Zur vertrauten Strasse zurueck");
    Text(FVector(1200, -1050, 290), TEXT("E: HAUS 10"), 90);

    GetWorld()->SpawnActor<ANostalgiaWorldLoop>(FVector(5700, 0, 150), FRotator::ZeroRotator);
    Text(FVector(5580, 0, 270), TEXT("STOP"));
    Box(FVector(5780, 0, 340), FVector(30, 950, 30)); // visual threshold
    for (int32 Index = 0; Index < 5; ++Index)
    {
        const FVector Base(Index == 4 ? 8500 : 900 + Index * 1500, 420, 0);
        Box(Base + FVector(0, 0, 220), FVector(20, 20, 440));
        Box(Base + FVector(0, 0, 440), FVector(100, 100, 25));
        if (Index == 4) continue; // the deliberately dark final lamp
        auto* Light = NewObject<UPointLightComponent>(this);
        AddInstanceComponent(Light);
        Light->SetMobility(EComponentMobility::Movable);
        Light->SetupAttachment(RootComponent);
        Light->SetRelativeLocation(Base + FVector(0, 0, 420));
        Light->SetIntensity(3000);
        Light->SetAttenuationRadius(1100);
        Light->SetCastShadows(false);
        Light->SetLightColor(FLinearColor(1.0, .8, .45));
        Light->SetVisibility(false);
        Light->RegisterComponent();
        StreetLights.Add(Light);
    }
    QuestObject(FVector(8500, 330, 100), FVector(40, 60, 180), Nostalgia::Action::ExamineLamp, TEXT("Die Laterne, die nicht angeht"));
    QuestObject(FVector(8900, 450, 100), FVector(220, 220, 200), Nostalgia::Action::EnterPlayhouse, TEXT("Spielhaus betreten - Prototype abschliessen"));
    Text(FVector(7600, -400, 220), TEXT("Der Weg hinter dem fehlenden Haus"));
    Homecomer = GetWorld()->SpawnActor<ANostalgiaHomecomer>(FVector(3500, 850, 0), FRotator(0, -90, 0));

    Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0, 0, 1000), FRotator(-40, -35, 0));
    Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    auto* SunComponent = CastChecked<UDirectionalLightComponent>(Sun->GetLightComponent());
    SunComponent->bAtmosphereSunLight = true;
    SunComponent->SetIntensity(8);
    NightFill = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0, 0, 1000), FRotator(-25, 145, 0));
    auto* NightComponent = CastChecked<UDirectionalLightComponent>(NightFill->GetLightComponent());
    NightComponent->SetMobility(EComponentMobility::Movable);
    NightComponent->bAtmosphereSunLight = false;
    NightComponent->SetCastShadows(false);
    NightComponent->SetLightColor(FLinearColor(.35f, .45f, .7f));
    NightComponent->SetIntensity(0);
    GetWorld()->SpawnActor<ASkyAtmosphere>();
    Sky = GetWorld()->SpawnActor<ASkyLight>();
    Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Sky->GetLightComponent()->SetIntensity(0.8f);
    Sky->GetLightComponent()->RecaptureSky();
}

void ANostalgiaBlockout::BeginPlay()
{
    Super::BeginPlay();
    Build();
    auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    Session->Configure(LoadObject<UNostalgiaEventCatalog>(nullptr, TEXT("/Game/Data/DA_EndlessSummerEvents.DA_EndlessSummerEvents")));
    SeenLoadRevision = Session->LoadRevision;
    UE_LOG(LogNostalgia, Log, TEXT("Blockout created. Runtime acceptance still requires human validation."));
}

Nostalgia::District ANostalgiaBlockout::DistrictAt(const FVector& Location)
{
    return Nostalgia::DistrictAt(Location.X, Location.Y);
}

void ANostalgiaBlockout::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Player = Cast<ANostalgiaCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player || !Homecomer || !Sun || !NightFill) return;
    auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    if (Session->LoadRevision != SeenLoadRevision)
    {
        Audio->StopTransientAudio();
        Homecomer->ResetPresentation();
        SeenLoadRevision = Session->LoadRevision;
    }
    const auto District = DistrictAt(Player->GetActorLocation());
    const float Step = FMath::Clamp(DeltaSeconds, 0.0f, 0.25f);
    Session->SetContext(District, District == Nostalgia::District::Bedroom || District == Nostalgia::District::Finale,
        Homecomer->IsVisibleThreat(), Player->GetVelocity().Size(), Step);
    const auto Event = Session->Step(Step);
    Audio->Play(Event);
    Audio->UpdateListener(Player->GetActorLocation(), District);
    if (Event == Nostalgia::EventId::HomecomerSighting) Homecomer->Show();
    const auto& State = Session->GetState();
    if (DisplayedLoops != State.loops)
    {
        DisplayedLoops = State.loops;
        ChangedNumber->SetText(FText::FromString(State.loops > 0 ? TEXT("HAUS 13 1/2") : TEXT("")));
    }
    if (DisplayedStage != State.stage)
    {
        DisplayedStage = State.stage;
        MemoryHouse->SetVisibility(State.stage >= Nostalgia::Stage::BirthdayRestored);
    }
    const bool bNight = Nostalgia::IsNight(State);
    if (bNight != bWasNight)
    {
        bWasNight = bNight;
        // Keep the captured sky; a second expensive runtime capture is unnecessary for this blockout.
        // A dim directional fill keeps the final unlit lamp navigable without Lumen.
        Sun->SetActorRotation(bNight ? FRotator(15, -35, 0) : FRotator(-40, -35, 0));
        Sun->GetLightComponent()->SetIntensity(bNight ? 0.15f : 8.0f);
        NightFill->GetLightComponent()->SetIntensity(bNight ? 0.3f : 0.0f);
        Sky->GetLightComponent()->SetIntensity(bNight ? 0.12f : 0.8f);
        for (const auto& Light : StreetLights) Light->SetVisibility(bNight);
    }
    if (State.stage == Nostalgia::Stage::Complete) Audio->StopTransientAudio();
    if (Player->GetActorLocation().Z < -500)
    {
        // Recovery keeps quest state and avoids a fall softlock; safe authored anchor.
        if (Player->TeleportTo(FVector(-1200, 0, 100), FRotator::ZeroRotator, false, false))
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            Player->ProtectTransition();
            Session->LastMessage = TEXT("Sicherheitsrueckkehr ins Kinderzimmer; Fortschritt erhalten.");
        }
    }
}
