#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NostalgiaCharacter.generated.h"

class UCameraComponent;
UCLASS()
class NOSTALGIA_API ANostalgiaCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ANostalgiaCharacter();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    AActor* FocusedActor() const;
    bool TransitionReady() const;
    void ProtectTransition();
    bool bClockVisible = false;
    bool bDebugVisible = true;
    bool bHintVisible = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> Camera;

private:
    bool bSprinting = false;
    double NextTransitionTime = 0.0;
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void SprintStart();
    void SprintStop();
    void CrouchStart();
    void CrouchStop();
    void Interact();
    void ToggleClock();
    void ToggleDebug();
    void ToggleHint();
    void Save();
    void Load();
};
