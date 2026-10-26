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
    bool IsRestoringSnapshot() const { return bRestoringSnapshot; }
    void BeginSnapshotRestore();
    void EndSnapshotRestore(bool bSucceeded);
    void SetMemoryVisible(bool bVisible);
    bool bClockVisible = false;
    bool bDebugVisible = false;
    bool bHintVisible = false;
    bool bMemoryVisible = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> Camera;

private:
    bool bSprinting = false;
    bool bRestoringSnapshot = false;
    double NextTransitionTime = 0.0;
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void SprintStart();
    void SprintStop();
    void CrouchStart();
    void JumpStart();
    void CrouchStop();
    void Interact();
    void ToggleClock();
    void ToggleDebug();
    void ToggleHint();
    void ToggleMemory();
    void Save();
    void Load();
};
