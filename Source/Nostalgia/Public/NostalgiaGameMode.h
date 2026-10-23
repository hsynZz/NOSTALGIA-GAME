#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NostalgiaGameMode.generated.h"

UCLASS()
class NOSTALGIA_API ANostalgiaGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ANostalgiaGameMode();
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
};
