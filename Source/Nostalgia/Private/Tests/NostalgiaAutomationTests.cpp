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
    Nostalgia::SaveSnapshot Expected;
    Nostalgia::Advance(Expected.state, 12);
    Nostalgia::Apply(Expected.state, Nostalgia::Action::Photo);
    Nostalgia::Apply(Expected.state, Nostalgia::Action::ComparePhoto);
    Nostalgia::Apply(Expected.state, Nostalgia::Action::RestoreBirthday);
    Expected.state.randomState = 45678;
    Expected.state.openDoors = 3;
    const Nostalgia::Context Garden{Nostalgia::District::Garden, false, false, 10, 0};
    Nostalgia::SelectEvent(Expected.state, Garden);
    Expected.position = {2050, 900, 96};
    Expected.rotation = {0, 90, 0};
    auto* Original = NewObject<UNostalgiaSaveGame>();
    if (!TestTrue(TEXT("Encode validated production snapshot"), Original->FromSnapshot(Expected))) return false;
    TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Serialize actual USaveGame"), UGameplayStatics::SaveGameToMemory(Original, Bytes))) return false;
    auto* Loaded = Cast<UNostalgiaSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Deserialize class"), Loaded)) return false;
    Nostalgia::SaveSnapshot Actual;
    if (!TestTrue(TEXT("Decode actual Unreal serialization"), Loaded->ToSnapshot(Actual))) return false;
    TestTrue(TEXT("Quest, clock, doors, history and RNG roundtrip"), Actual.state == Expected.state);
    TestEqual(TEXT("Position"), Loaded->PlayerLocation, Original->PlayerLocation);
    TestEqual(TEXT("View"), Loaded->PlayerRotation, Original->PlayerRotation);
    TestEqual(TEXT("Event history count"), Loaded->Events.Num(), static_cast<int32>(Nostalgia::EventCount));
    TestEqual(TEXT("Schema"), Loaded->Version, Nostalgia::SaveVersion);
    Loaded->Stage = 255;
    const auto Before = Actual;
    TestFalse(TEXT("Reject corrupt reflected stage"), Loaded->ToSnapshot(Actual));
    TestTrue(TEXT("Rejected decode preserves output"), Actual.state == Before.state && Actual.position.x == Before.position.x);
    Loaded->FromSnapshot(Expected);
    Loaded->Events[0].Count = -1;
    TestFalse(TEXT("Reject negative reflected event count"), Loaded->ToSnapshot(Actual));
    Loaded->FromSnapshot(Expected);
    ++Loaded->Version;
    TestFalse(TEXT("Reject incompatible schema"), Loaded->ToSnapshot(Actual));
    Loaded->FromSnapshot(Expected);
    Loaded->Chapter = TEXT("OtherChapter");
    TestFalse(TEXT("Reject foreign chapter"), Loaded->ToSnapshot(Actual));
    auto Invalid = Expected;
    Invalid.position = {3000, 3200, 96};
    TestFalse(TEXT("Reject unsupported position before encoding"), Original->FromSnapshot(Invalid));
    TestEqual(TEXT("Rejected encode preserves original position"), Original->PlayerLocation, FVector(2050, 900, 96));
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
