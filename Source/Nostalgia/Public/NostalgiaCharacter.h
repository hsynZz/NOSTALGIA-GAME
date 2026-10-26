#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/NostalgiaPresentation.h"
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
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nostalgia|Comfort") bool bReducedCameraMotion = false;
    UPROPERTY(EditAnywhere, Category="Nostalgia|Movement") float WalkSpeed = 320;
    UPROPERTY(EditAnywhere, Category="Nostalgia|Movement") float SprintSpeed = 520;
    // The authored quality scene has a different layout; never load a blockout save into it.
    bool bPrototypeSaveEnabled = true;
    FString ActionFeedback;
    double ActionFeedbackUntil = 0;

private:
    bool bSprinting = false;
    bool bRestoringSnapshot = false;
    double NextTransitionTime = 0.0;
    Nostalgia::ViewMotion View;
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
    void ToggleComfort();
    void Save();
    void Load();
};
