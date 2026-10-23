#include "Core/NostalgiaEncounter.h"
#include <algorithm>
#include <cmath>

namespace Nostalgia
{
namespace
{
bool Valid(const Perception& p)
{
    return std::isfinite(p.distance) && p.distance >= 0 && std::isfinite(p.viewDot) &&
        p.viewDot >= -1 && p.viewDot <= 1;
}
}
bool CanStartEncounter(const Encounter& e, const Perception& p)
{
    return e.phase == EncounterPhase::Dormant && Valid(p) && !p.safe && !p.blocked &&
        p.lineOfSight && p.inView && p.viewDot >= .6 && p.distance >= 600 && p.distance <= 2200;
}
bool StartEncounter(Encounter& e, const Perception& p)
{
    if (!CanStartEncounter(e, p)) return false;
    e = Encounter{};
    e.phase = EncounterPhase::Observing;
    return true;
}
bool EncounterActive(const Encounter& e)
{
    return e.phase == EncounterPhase::Observing || e.phase == EncounterPhase::Approaching ||
        e.phase == EncounterPhase::Retreating;
}
double StepEncounter(Encounter& e, const Perception& p, double seconds)
{
    if (!EncounterActive(e)) return 0;
    if (!Valid(p) || p.safe) { e.phase = EncounterPhase::Cooldown; return 0; }
    if (!std::isfinite(seconds) || seconds <= 0 || seconds > .25) return 0;
    e.elapsed += seconds;
    if (e.phase != EncounterPhase::Retreating && (p.blocked || p.distance < 450 || e.elapsed >= 9))
        e.phase = EncounterPhase::Retreating;
    if (e.phase == EncounterPhase::Retreating)
    {
        e.retreatElapsed += seconds;
        if (e.retreatElapsed >= 2) { e.phase = EncounterPhase::Cooldown; return 0; }
        return p.blocked ? 0 : -80 * seconds;
    }
    // First two seconds remain still. An occluded figure never follows around corners.
    const bool approach = e.elapsed >= 2 && p.lineOfSight && (!p.inView || p.viewDot < .6) &&
        p.distance > 450 && e.approachDistance < 150;
    e.phase = approach ? EncounterPhase::Approaching : EncounterPhase::Observing;
    if (!approach) return 0;
    const double distance = std::min({60 * seconds, 150 - e.approachDistance, p.distance - 450});
    e.approachDistance += distance;
    return distance;
}
const char* EncounterName(EncounterPhase phase)
{
    switch (phase)
    {
    case EncounterPhase::Dormant: return "Dormant";
    case EncounterPhase::Observing: return "Observing";
    case EncounterPhase::Approaching: return "Approaching";
    case EncounterPhase::Retreating: return "Retreating";
    case EncounterPhase::Cooldown: return "Cooldown";
    }
    return "Invalid";
}
}
