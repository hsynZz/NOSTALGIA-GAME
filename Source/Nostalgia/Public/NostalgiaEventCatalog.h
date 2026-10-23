#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/NostalgiaCore.h"
#include "NostalgiaEventCatalog.generated.h"

UENUM(BlueprintType)
enum class ENostalgiaEvent : uint8 { GardenLaughter, HomecomerSighting, WindowKnock };

USTRUCT(BlueprintType)
struct FNostalgiaEventRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) ENostalgiaEvent Event = ENostalgiaEvent::GardenLaughter;
    // Values documented in Docs/ARCHITECTURE.md and checked before use.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="4")) uint8 District = 2;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="6")) uint8 MinimumStage = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="6")) uint8 MaximumStage = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="2")) uint8 Phase = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) float MinimumDwell = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) float MaximumSpeed = 700.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) float Cooldown = 120.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) int32 MaximumPlays = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Priority = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bScare = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCritical = true;

    Nostalgia::EventRule ToCore() const;
};

UCLASS(BlueprintType)
class NOSTALGIA_API UNostalgiaEventCatalog : public UDataAsset
{
    GENERATED_BODY()
public:
    UNostalgiaEventCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FNostalgiaEventRule> Rules;
    // Reject the complete catalog on invalid/duplicate/missing IDs; caller keeps validated defaults.
    bool GetValidatedRules(std::array<Nostalgia::EventRule, Nostalgia::EventCount>& OutRules) const;
};
