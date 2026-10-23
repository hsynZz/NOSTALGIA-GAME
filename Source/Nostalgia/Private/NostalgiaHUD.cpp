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
    DrawText(TEXT("J Foto ansehen/schliessen | F5 Speichern | F9 Laden | F3 Debug | Alt+F4 Beenden"), FColor::White, 24, 92);
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
        const FString Debug = FString::Printf(TEXT("Bezirk=%d Safe=%d Threat=%d | Quest=%d Loops=%u\nEvent=%s | Minute=%.1f Elapsed=%.1f Seed=%u\nHeimkehrer=%s | Bereit: Garten=%d Sichtung=%d Klopfen=%d"),
            static_cast<int32>(Context.district), Context.safeZone, Context.threatened, static_cast<int32>(State.stage),
            State.loops, *Session->LastEvent, State.minute, State.elapsed, State.randomState,
            *Session->EncounterDebug, Context.available[0], Context.available[1], Context.available[2]);
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
    if (Player->bMemoryVisible)
    {
        const float X = FMath::Max(12.0f, (Canvas->SizeX - 620.0f) / 2);
        const float Y = FMath::Max(12.0f, (Canvas->SizeY - 390.0f) / 2);
        DrawRect(FLinearColor(.08f, .07f, .05f, .98f), X, Y, 620, 390);
        DrawText(TEXT("GEBURTSTAGSFOTO | 17.08.2004"), FColor::White, X + 24, Y + 18);
        // Original schematic evidence card: the house and four chairs match the authored garden.
        DrawRect(FLinearColor(.65f, .8f, .9f), X + 24, Y + 50, 572, 180);
        DrawRect(FLinearColor(.4f, .55f, .3f), X + 24, Y + 150, 572, 80);
        DrawRect(FLinearColor(.8f, .7f, .5f), X + 260, Y + 75, 100, 80);
        DrawRect(FLinearColor(.35f, .2f, .15f), X + 250, Y + 65, 120, 15);
        DrawRect(FLinearColor::White, X + 270, Y + 175, 80, 25);
        for (const FVector2D& Offset : {FVector2D(250, 180), FVector2D(355, 180), FVector2D(300, 155), FVector2D(300, 205)})
            DrawRect(FLinearColor(.5f, .25f, .1f), X + Offset.X, Y + Offset.Y, 15, 15);
        DrawText(TEXT("Vier Stuehle um den Geburtstagstisch. Dahinter ein kleines Haus."), FColor::White, X + 24, Y + 250);
        const TCHAR* Observation = State.stage < Nostalgia::Stage::DiscrepancySeen ?
            TEXT("Wie sieht dieser Platz heute aus?") : (State.stage < Nostalgia::Stage::BirthdayRestored ?
            TEXT("Im Garten: ein Stuhl abseits. Hinter dem Tisch fehlt das Haus.") :
            TEXT("Die Anordnung stimmt wieder. Das Haus ist zurueckgekehrt."));
        DrawText(Observation, FColor::Yellow, X + 24, Y + 280);
        DrawText(TEXT("J: Foto schliessen. Die Uhr laeuft waehrend des Betrachtens weiter."), FColor::White, X + 24, Y + 345);
    }
}
