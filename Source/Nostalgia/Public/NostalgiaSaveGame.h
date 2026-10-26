#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Core/NostalgiaSnapshot.h"
#include "NostalgiaSaveGame.generated.h"

USTRUCT()
struct FNostalgiaSavedEvent
{
    GENERATED_BODY()
    UPROPERTY(SaveGame) int32 Count = 0;
    UPROPERTY(SaveGame) double LastTime = -1000000.0;
};

UCLASS()
class NOSTALGIA_API UNostalgiaSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    bool ToSnapshot(Nostalgia::SaveSnapshot& Out) const;
    bool FromSnapshot(const Nostalgia::SaveSnapshot& In);
    UPROPERTY(SaveGame) int32 Version = Nostalgia::SaveVersion;
    UPROPERTY(SaveGame) FString Chapter = TEXT("EndlessSummer2004");
    UPROPERTY(SaveGame) uint8 Stage = 0;
    UPROPERTY(SaveGame) double Minute = 900.0;
    UPROPERTY(SaveGame) double Elapsed = 0.0;
    UPROPERTY(SaveGame) uint32 RandomState = 2004;
    UPROPERTY(SaveGame) uint32 Loops = 0;
    UPROPERTY(SaveGame) uint8 OpenDoors = 0;
    UPROPERTY(SaveGame) double LastTransition = -1000000.0;
    UPROPERTY(SaveGame) double LastShock = -1000000.0;
    UPROPERTY(SaveGame) TArray<FNostalgiaSavedEvent> Events;
    UPROPERTY(SaveGame) FVector PlayerLocation = FVector::ZeroVector;
    UPROPERTY(SaveGame) FRotator PlayerRotation = FRotator::ZeroRotator;
};
