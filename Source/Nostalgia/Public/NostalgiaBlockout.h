#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/NostalgiaCore.h"
#include "NostalgiaBlockout.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;
class UPointLightComponent;
class ADirectionalLight;
class ASkyLight;
class ANostalgiaHomecomer;
class UNostalgiaAudioDirector;
UCLASS()
class NOSTALGIA_API ANostalgiaBlockout : public AActor
{
    GENERATED_BODY()
public:
    ANostalgiaBlockout();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    static Nostalgia::District DistrictAt(const FVector& Location);

private:
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UNostalgiaAudioDirector> Audio;
    UPROPERTY() TObjectPtr<ANostalgiaHomecomer> Homecomer;
    UPROPERTY() TObjectPtr<ADirectionalLight> Sun;
    UPROPERTY() TObjectPtr<ASkyLight> Sky;
    UPROPERTY() TArray<TObjectPtr<UPointLightComponent>> StreetLights;
    UPROPERTY() TObjectPtr<UTextRenderComponent> ChangedNumber;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> MemoryHouse;
    uint32 SeenLoadRevision = 0;
    bool bWasNight = false;
    int32 ComponentCounter = 0;
    UStaticMeshComponent* Box(const FVector& Center, const FVector& Size);
    UTextRenderComponent* Text(const FVector& Location, const FString& Value, float Yaw = 180);
    void Room(const FVector& Center, const FVector& Size, bool bDoorway);
    void Build();
};
