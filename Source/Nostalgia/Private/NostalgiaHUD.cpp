#include "NostalgiaHUD.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaInteraction.h"
#include "NostalgiaStateSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

void ANostalgiaHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !PlayerOwner) return;
    const auto* Player = Cast<ANostalgiaCharacter>(PlayerOwner->GetPawn());
    const auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    if (!Player || !Session) return;
    const auto& State = Session->GetState();
    DrawRect(FLinearColor(0, 0, 0, .65), 12, 12, FMath::Min(Canvas->SizeX - 24.0f, 1050.0f), Player->bHintVisible ? 180 : 150);
    DrawText(TEXT("NOSTALGIA | Endless Summer - 2004 | Prototype 0.1"), FColor::White, 24, 20);
    DrawText(Session->ObjectiveText(), FColor::Yellow, 24, 45);
    DrawText(TEXT("WASD Maus | Shift Sprint | Strg Ducken | Leertaste Sprung | E Interaktion | Tab Uhr | H Hinweis"), FColor::White, 24, 70);
    DrawText(TEXT("F5 Speichern | F9 Laden | F3 Debug | Alt+F4 Beenden"), FColor::White, 24, 92);
    DrawText(Session->LastMessage, FColor::Cyan, 24, 118);
    if (Player->bHintVisible) DrawText(Session->HintText(), FColor::White, 24, 144);
    if (Player->bClockVisible || State.elapsed < 10)
        DrawText(Session->ClockText(), FColor::White, 24, 205, nullptr, 1.6f);
    DrawText(TEXT("+"), FColor::White, Canvas->SizeX / 2.0f - 4, Canvas->SizeY / 2.0f - 8);
    if (AActor* Focus = Player->FocusedActor())
        DrawText(INostalgiaInteraction::Execute_InteractionText(Focus).ToString(), FColor::Yellow,
            Canvas->SizeX / 2.0f - 150, Canvas->SizeY / 2.0f + 30);
    if (Player->bDebugVisible)
    {
        const auto& Context = Session->GetContext();
        const FString Debug = FString::Printf(TEXT("Trigger/Bezirk=%d Safe=%d Threat=%d | Quest=%d Loops=%u\nEvent=%s | Minute=%.1f Elapsed=%.1f Seed=%u"),
            static_cast<int32>(Context.district), Context.safeZone, Context.threatened, static_cast<int32>(State.stage),
            State.loops, *Session->LastEvent, State.minute, State.elapsed, State.randomState);
        DrawRect(FLinearColor(0, 0, 0, .7), 12, Canvas->SizeY - 90, FMath::Min(Canvas->SizeX - 24.0f, 1050.0f), 78);
        DrawText(Debug, FColor::Green, 24, Canvas->SizeY - 80);
    }
    if (State.stage == Nostalgia::Stage::Complete)
    {
        DrawRect(FLinearColor(0, 0, 0, .8), 70, Canvas->SizeY * .6f, Canvas->SizeX - 140, 90);
        DrawText(TEXT("PROTOTYPE ABGESCHLOSSEN - Das Spielhaus fuehrt spaeter nach Playtime Forever."),
            FColor::White, 90, Canvas->SizeY * .6f + 20);
        DrawText(TEXT("Kein weiteres Kapitel in Version 0.1. F9: Spielstand laden. Alt+F4: beenden."),
            FColor::White, 90, Canvas->SizeY * .6f + 48);
    }
}
