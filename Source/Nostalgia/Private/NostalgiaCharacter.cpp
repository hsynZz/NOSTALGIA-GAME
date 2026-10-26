#include "NostalgiaCharacter.h"
#include "NostalgiaInteraction.h"
#include "NostalgiaStateSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

ANostalgiaCharacter::ANostalgiaCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    Camera->SetupAttachment(GetCapsuleComponent());
    Camera->SetRelativeLocation(FVector(0, 0, 64));
    Camera->bUsePawnControlRotation = true;
    Camera->FieldOfView = 85;
    bUseControllerRotationYaw = true;
    auto* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = 320;
    Movement->MaxWalkSpeedCrouched = 150;
    Movement->JumpZVelocity = 420;
    Movement->AirControl = 0.2f;
    Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
    Movement->SetCrouchedHalfHeight(48);
}

void ANostalgiaCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    GetCharacterMovement()->MaxWalkSpeed = bSprinting && !bIsCrouched ? 520 : 320;
    // Capsule shrinks on crouch, camera moves with it; no artificial shake or blur.
    const float EyeHeight = bIsCrouched ? 30.0f : 64.0f;
    Camera->SetRelativeLocation(FVector(0, 0, FMath::FInterpTo(Camera->GetRelativeLocation().Z, EyeHeight, DeltaSeconds, 12)));
}

void ANostalgiaCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis(TEXT("MoveForward"), this, &ANostalgiaCharacter::MoveForward);
    Input->BindAxis(TEXT("MoveRight"), this, &ANostalgiaCharacter::MoveRight);
    Input->BindAxis(TEXT("Turn"), this, &ANostalgiaCharacter::Turn);
    Input->BindAxis(TEXT("LookUp"), this, &ANostalgiaCharacter::LookUp);
    Input->BindAction(TEXT("Jump"), IE_Pressed, this, &ANostalgiaCharacter::JumpStart);
    Input->BindAction(TEXT("Jump"), IE_Released, this, &ANostalgiaCharacter::StopJumping);
    Input->BindAction(TEXT("Sprint"), IE_Pressed, this, &ANostalgiaCharacter::SprintStart);
    Input->BindAction(TEXT("Sprint"), IE_Released, this, &ANostalgiaCharacter::SprintStop);
    Input->BindAction(TEXT("Crouch"), IE_Pressed, this, &ANostalgiaCharacter::CrouchStart);
    Input->BindAction(TEXT("Crouch"), IE_Released, this, &ANostalgiaCharacter::CrouchStop);
    Input->BindAction(TEXT("Interact"), IE_Pressed, this, &ANostalgiaCharacter::Interact);
    Input->BindAction(TEXT("Clock"), IE_Pressed, this, &ANostalgiaCharacter::ToggleClock);
    Input->BindAction(TEXT("Debug"), IE_Pressed, this, &ANostalgiaCharacter::ToggleDebug);
    Input->BindAction(TEXT("Hint"), IE_Pressed, this, &ANostalgiaCharacter::ToggleHint);
    Input->BindAction(TEXT("Memory"), IE_Pressed, this, &ANostalgiaCharacter::ToggleMemory);
    Input->BindAction(TEXT("Save"), IE_Pressed, this, &ANostalgiaCharacter::Save);
    Input->BindAction(TEXT("Load"), IE_Pressed, this, &ANostalgiaCharacter::Load);
}

void ANostalgiaCharacter::MoveForward(float Value) { if (!bMemoryVisible) AddMovementInput(GetActorForwardVector(), Value); }
void ANostalgiaCharacter::MoveRight(float Value) { if (!bMemoryVisible) AddMovementInput(GetActorRightVector(), Value); }
void ANostalgiaCharacter::Turn(float Value) { if (!bMemoryVisible) AddControllerYawInput(Value); }
void ANostalgiaCharacter::LookUp(float Value) { if (!bMemoryVisible) AddControllerPitchInput(Value); }
void ANostalgiaCharacter::SprintStart() { if (!bMemoryVisible) bSprinting = true; }
void ANostalgiaCharacter::SprintStop() { bSprinting = false; }
void ANostalgiaCharacter::CrouchStart() { if (!bMemoryVisible) Crouch(); }
void ANostalgiaCharacter::JumpStart() { if (!bMemoryVisible) Jump(); }
void ANostalgiaCharacter::CrouchStop() { UnCrouch(); }
void ANostalgiaCharacter::ToggleClock() { bClockVisible = !bClockVisible; }
void ANostalgiaCharacter::ToggleDebug() { bDebugVisible = !bDebugVisible; }
void ANostalgiaCharacter::ToggleHint() { bHintVisible = !bHintVisible; }
void ANostalgiaCharacter::ToggleMemory()
{
    const auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    if (Session && Session->GetState().stage >= Nostalgia::Stage::PhotoFound)
        SetMemoryVisible(!bMemoryVisible);
}

void ANostalgiaCharacter::SetMemoryVisible(bool bVisible)
{
    bMemoryVisible = bVisible;
    if (bVisible)
    {
        bSprinting = false;
        StopJumping();
        // Stop grounded drift; opening a hint in midair must not cancel gravity or launch momentum.
        if (GetCharacterMovement()->IsMovingOnGround()) GetCharacterMovement()->StopMovementImmediately();
    }
}

AActor* ANostalgiaCharacter::FocusedActor() const
{
    if (bMemoryVisible) return nullptr;
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(NostalgiaInteraction), false, this);
    const FVector Start = Camera->GetComponentLocation();
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Camera->GetForwardVector() * 260, ECC_Visibility, Query))
    {
        AActor* Actor = Hit.GetActor();
        if (Actor && Actor->GetClass()->ImplementsInterface(UNostalgiaInteraction::StaticClass())) return Actor;
    }
    return nullptr;
}

void ANostalgiaCharacter::Interact()
{
    if (AActor* Actor = FocusedActor()) INostalgiaInteraction::Execute_Interact(Actor, this);
}
void ANostalgiaCharacter::Save() { if (auto* State = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>()) State->Save(this); }
void ANostalgiaCharacter::Load()
{
    if (auto* State = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>())
        if (State->Load(this)) SetMemoryVisible(false);
}
bool ANostalgiaCharacter::TransitionReady() const { return !bRestoringSnapshot && GetWorld()->GetTimeSeconds() >= NextTransitionTime; }
void ANostalgiaCharacter::ProtectTransition() { NextTransitionTime = GetWorld()->GetTimeSeconds() + 3.0; }
void ANostalgiaCharacter::BeginSnapshotRestore() { bRestoringSnapshot = true; }
void ANostalgiaCharacter::EndSnapshotRestore(bool bSucceeded)
{
    bRestoringSnapshot = false;
    if (bSucceeded) ProtectTransition();
}
