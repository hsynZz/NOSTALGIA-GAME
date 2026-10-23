#include "Core/NostalgiaCore.h"
#include "NostalgiaEventCatalog.h"
#include "NostalgiaSaveGame.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNostalgiaQuestChainTest, "Nostalgia.Core.CausalEscapeChain",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNostalgiaQuestChainTest::RunTest(const FString& Parameters)
{
    Nostalgia::State State;
    TestFalse(TEXT("No early exit"), Nostalgia::Apply(State, Nostalgia::Action::EnterPlayhouse));
    TestTrue(TEXT("Photo"), Nostalgia::Apply(State, Nostalgia::Action::Photo));
    TestTrue(TEXT("Compare"), Nostalgia::Apply(State, Nostalgia::Action::ComparePhoto));
    TestTrue(TEXT("Birthday"), Nostalgia::Apply(State, Nostalgia::Action::RestoreBirthday));
    TestTrue(TEXT("Night"), Nostalgia::IsNight(State));
    TestTrue(TEXT("Loop"), Nostalgia::TraverseLoop(State));
    TestTrue(TEXT("Lamp"), Nostalgia::Apply(State, Nostalgia::Action::ExamineLamp));
    TestTrue(TEXT("Exit"), Nostalgia::Apply(State, Nostalgia::Action::EnterPlayhouse));
    TestTrue(TEXT("Valid terminal state"), Nostalgia::Validate(State));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNostalgiaCatalogTest, "Nostalgia.Data.EventCatalogValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNostalgiaCatalogTest::RunTest(const FString& Parameters)
{
    auto* Catalog = NewObject<UNostalgiaEventCatalog>();
    auto Rules = Nostalgia::DefaultRules();
    TestTrue(TEXT("C++ defaults"), Catalog->GetValidatedRules(Rules));
    Catalog->Rules[1].Event = Catalog->Rules[0].Event;
    TestFalse(TEXT("Reject duplicate IDs"), Catalog->GetValidatedRules(Rules));
    Catalog->Rules.Empty();
    TestFalse(TEXT("Reject incomplete catalog"), Catalog->GetValidatedRules(Rules));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNostalgiaSaveRoundtripTest, "Nostalgia.Save.UnrealSerialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNostalgiaSaveRoundtripTest::RunTest(const FString& Parameters)
{
    auto* Original = NewObject<UNostalgiaSaveGame>();
    Original->Stage = 3;
    Original->Minute = 1234;
    Original->RandomState = 45678;
    Original->OpenDoors = 1;
    Original->PlayerLocation = FVector(1200, 42, 96);
    FNostalgiaSavedEvent History;
    History.Count = 1; History.LastTime = 12;
    Original->Events.Add(History);
    TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Serialize actual USaveGame"), UGameplayStatics::SaveGameToMemory(Original, Bytes))) return false;
    auto* Loaded = Cast<UNostalgiaSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Deserialize class"), Loaded)) return false;
    TestEqual(TEXT("Schema"), Loaded->Version, Nostalgia::SaveVersion);
    TestEqual(TEXT("Chapter"), Loaded->Chapter, Original->Chapter);
    TestEqual(TEXT("Quest"), Loaded->Stage, Original->Stage);
    TestEqual(TEXT("Clock"), Loaded->Minute, Original->Minute);
    TestEqual(TEXT("RNG"), Loaded->RandomState, Original->RandomState);
    TestEqual(TEXT("Door"), Loaded->OpenDoors, Original->OpenDoors);
    TestEqual(TEXT("Position"), Loaded->PlayerLocation, Original->PlayerLocation);
    if (TestEqual(TEXT("History size"), Loaded->Events.Num(), 1))
    {
        TestEqual(TEXT("Count"), Loaded->Events[0].Count, 1);
        TestEqual(TEXT("Last event time"), Loaded->Events[0].LastTime, 12.0);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNostalgiaPreparedAssetsTest, "Nostalgia.Assets.PreparedPackages",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNostalgiaPreparedAssetsTest::RunTest(const FString& Parameters)
{
    auto* Map = LoadObject<UWorld>(nullptr, TEXT("/Game/Maps/EndlessSummer.EndlessSummer"));
    TestNotNull(TEXT("Genuine saved map"), Map);
    auto* Catalog = LoadObject<UNostalgiaEventCatalog>(nullptr, TEXT("/Game/Data/DA_EndlessSummerEvents.DA_EndlessSummerEvents"));
    if (TestNotNull(TEXT("Generated event asset"), Catalog))
    {
        auto Rules = Nostalgia::DefaultRules();
        TestTrue(TEXT("Asset rules valid"), Catalog->GetValidatedRules(Rules));
    }
    TestNotNull(TEXT("Silhouette material"), LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Silhouette.M_Silhouette")));
    for (const TCHAR* Path : {TEXT("/Game/Audio/A_GardenPlaceholder.A_GardenPlaceholder"), TEXT("/Game/Audio/A_KnockPlaceholder.A_KnockPlaceholder")})
    {
        const auto* Sound = LoadObject<USoundWave>(nullptr, Path);
        if (TestNotNull(Path, Sound)) TestTrue(TEXT("Imported sound has duration"), Sound->Duration > 0.0f);
    }
    return true;
}
#endif
