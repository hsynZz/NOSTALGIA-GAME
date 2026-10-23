#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NostalgiaHUD.generated.h"

UCLASS()
class NOSTALGIA_API ANostalgiaHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
