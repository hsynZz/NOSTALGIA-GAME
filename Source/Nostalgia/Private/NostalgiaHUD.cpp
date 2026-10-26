#include "NostalgiaHUD.h"
#include "NostalgiaCharacter.h"
#include "NostalgiaInteraction.h"
#include "NostalgiaStateSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void ANostalgiaHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !PlayerOwner) return;
    const auto* Player = Cast<ANostalgiaCharacter>(PlayerOwner->GetPawn());
    const auto* Session = GetGameInstance()->GetSubsystem<UNostalgiaStateSubsystem>();
    if (!Player || !Session) return;
    const auto& State = Session->GetState();
    // Normal view stays quiet. Help is explicitly requested, technical state is F3-only.
    if (Player->bHintVisible)
    {
        DrawRect(FLinearColor(0, 0, 0, .65), 20, 20, FMath::Min(Canvas->SizeX - 40.0f, 950.0f), 100);
        DrawText(TEXT("WASD Maus | Shift Sprint | Strg Ducken | Leertaste Sprung | E Benutzen | Tab Uhr"), FColor::White, 32, 32);
        DrawText(Player->bPrototypeSaveEnabled ? *Session->HintText() :
            TEXT("Sommer 2004. Nimm dir Zeit. Hoere am offenen Garten auf die Umgebung."), FColor::White, 32, 62);
        DrawText(TEXT("H: Hilfe schliessen"), FColor::White, 32, 92);
    }
    if (Player->bClockVisible || State.elapsed < 5)
        DrawText(Session->ClockText(), FColor::White, 32, Canvas->SizeY - 60, nullptr, 1.2f);
    if (GetWorld()->GetTimeSeconds() < Player->ActionFeedbackUntil)
        DrawText(Player->ActionFeedback, FColor::White, 32, Canvas->SizeY - 90);
    if (AActor* Focus = Player->FocusedActor())
    {
        DrawRect(FLinearColor(.85, .85, .8, .65), Canvas->SizeX / 2.0f - 1, Canvas->SizeY / 2.0f - 1, 2, 2);
        DrawText(TEXT("E"), FColor(220, 220, 204), Canvas->SizeX / 2.0f - 4, Canvas->SizeY / 2.0f + 24);
        if (Player->bHintVisible || Player->bDebugVisible)
            DrawText(INostalgiaInteraction::Execute_InteractionText(Focus).ToString(), FColor::White,
                Canvas->SizeX / 2.0f - 150, Canvas->SizeY / 2.0f + 50);
    }
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
