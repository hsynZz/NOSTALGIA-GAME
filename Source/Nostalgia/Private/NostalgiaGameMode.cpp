#include "NostalgiaGameMode.h"
#include "NostalgiaBlockout.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaHUD.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"

ANostalgiaGameMode::ANostalgiaGameMode()
{
    DefaultPawnClass = ANostalgiaCharacter::StaticClass();
    HUDClass = ANostalgiaHUD::StaticClass();
}
void ANostalgiaGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    if (!TActorIterator<ANostalgiaBlockout>(GetWorld())) GetWorld()->SpawnActor<ANostalgiaBlockout>();
    if (!TActorIterator<APlayerStart>(GetWorld()))
        GetWorld()->SpawnActor<APlayerStart>(FVector(-1200, 0, 100), FRotator::ZeroRotator);
}
