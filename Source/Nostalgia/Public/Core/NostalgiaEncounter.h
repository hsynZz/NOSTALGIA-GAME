#pragma once

namespace Nostalgia
{
enum class EncounterPhase { Dormant, Observing, Approaching, Retreating, Cooldown };
struct Perception
{
    double distance = 0;
    double viewDot = -1;
    bool lineOfSight = false;
    bool safe = true;
    bool blocked = false;
    bool inView = false;
};
struct Encounter
{
    EncounterPhase phase = EncounterPhase::Dormant;
    double elapsed = 0;
    double retreatElapsed = 0;
    double approachDistance = 0;
};
bool CanStartEncounter(const Encounter& encounter, const Perception& perception);
bool StartEncounter(Encounter& encounter, const Perception& perception);
// Signed requested travel in cm: positive toward player, negative away. Adapter sweeps collision.
double StepEncounter(Encounter& encounter, const Perception& perception, double seconds);
bool EncounterActive(const Encounter& encounter);
const char* EncounterName(EncounterPhase phase);
}
