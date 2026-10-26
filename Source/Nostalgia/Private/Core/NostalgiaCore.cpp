#include "Core/NostalgiaCore.h"
#include "Core/NostalgiaWorldLayout.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Nostalgia
{
namespace
{
bool ValidStage(Stage value) { return value >= Stage::Arrival && value <= Stage::Complete; }
std::uint32_t Random(std::uint32_t& value)
{
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    return value;
}
bool ValidTime(double value, double now)
{
    return std::isfinite(value) && value >= -1000000.0 && value <= now;
}
}

const std::array<EventRule, EventCount>& DefaultRules()
{
    static const std::array<EventRule, EventCount> rules{{
        {EventId::GardenLaughter, District::Garden, Stage::Arrival, Stage::BirthdayRestored,
         Phase::Any, 1.0, 700.0, 120.0, 1, 100, false, true},
        {EventId::HomecomerSighting, District::Street, Stage::BirthdayRestored, Stage::DarkLampFound,
         Phase::Any, 2.0, 700.0, 90.0, 1, 90, true, true},
        {EventId::WindowKnock, District::Interior, Stage::PhotoFound, Stage::RoadUnlocked,
         Phase::Night, 4.0, 200.0, 100.0, 1, 20, true, false}
    }};
    return rules;
}

bool IsNight(const State& state) { return state.minute >= 20.0 * 60.0 || state.minute < 6.0 * 60.0; }

District DistrictAt(double x, double y)
{
    return Layout::DistrictAt({x, y, 0});
}

bool Validate(const State& state)
{
    if (!ValidStage(state.stage) || !std::isfinite(state.minute) || state.minute < 0.0 || state.minute >= 1440.0 ||
        !std::isfinite(state.elapsed) || state.elapsed < 0.0 || state.randomState == 0 ||
        state.loops > 1000000 || state.openDoors > 3 ||
        !ValidTime(state.lastShock, state.elapsed) || !ValidTime(state.lastTransition, state.elapsed)) return false;
    if (state.stage >= Stage::RoadUnlocked && state.loops == 0) return false;
    for (const auto& item : state.history)
        if (item.count > 1000000 || !ValidTime(item.lastTime, state.elapsed) ||
            (item.count == 0 && item.lastTime != -1000000.0) || (item.count > 0 && item.lastTime < 0.0)) return false;
    return true;
}

bool ValidateRule(const EventRule& rule)
{
    return rule.id < EventId::Count && rule.district <= District::Finale &&
        ValidStage(rule.minimumStage) && ValidStage(rule.maximumStage) && rule.minimumStage <= rule.maximumStage &&
        rule.phase <= Phase::Night && std::isfinite(rule.minimumDwell) && rule.minimumDwell >= 0.0 &&
        std::isfinite(rule.maximumSpeed) && rule.maximumSpeed >= 0.0 &&
        std::isfinite(rule.cooldown) && rule.cooldown >= 0.0 && rule.maximumPlays <= 1000000;
}

bool Apply(State& state, Action action)
{
    Stage next = state.stage;
    switch (action)
    {
    case Action::Photo: if (state.stage == Stage::Arrival) next = Stage::PhotoFound; break;
    case Action::ComparePhoto: if (state.stage == Stage::PhotoFound) next = Stage::DiscrepancySeen; break;
    case Action::RestoreBirthday: if (state.stage == Stage::DiscrepancySeen) next = Stage::BirthdayRestored; break;
    case Action::ExamineLamp: if (state.stage == Stage::RoadUnlocked && IsNight(state)) next = Stage::DarkLampFound; break;
    case Action::EnterPlayhouse: if (state.stage == Stage::DarkLampFound) next = Stage::Complete; break;
    }
    if (next == state.stage) return false;
    state.stage = next;
    // A curated memory change causes sunset; no requirement to wait real hours.
    if (next == Stage::BirthdayRestored) state.minute = std::max(state.minute, 20.0 * 60.0 + 4.0);
    return true;
}

bool TraverseLoop(State& state)
{
    if (state.elapsed - state.lastTransition < 3.0 || state.loops == 1000000) return false;
    state.lastTransition = state.elapsed;
    ++state.loops;
    if (state.stage == Stage::BirthdayRestored) state.stage = Stage::RoadUnlocked;
    return true;
}

void Advance(State& state, double realSeconds)
{
    if (!std::isfinite(realSeconds) || realSeconds <= 0.0 || realSeconds > 86400.0) return;
    state.elapsed += realSeconds;
    // One game minute per real second. This endless summer day stops at 23:59.
    state.minute = std::min(1439.0, state.minute + realSeconds);
}

bool Eligible(const State& state, const Context& context, const EventRule& rule)
{
    if (!ValidateRule(rule) || state.stage == Stage::Complete || context.safeZone || context.threatened ||
        context.district != rule.district || state.stage < rule.minimumStage || state.stage > rule.maximumStage ||
        !std::isfinite(context.dwell) || context.dwell < rule.minimumDwell ||
        !std::isfinite(context.speed) || context.speed < 0.0 || context.speed > rule.maximumSpeed) return false;
    if ((rule.phase == Phase::Day && IsNight(state)) || (rule.phase == Phase::Night && !IsNight(state))) return false;
    if (!context.available[static_cast<std::size_t>(rule.id)]) return false;
    const auto& history = state.history[static_cast<std::size_t>(rule.id)];
    return history.count < rule.maximumPlays && state.elapsed - history.lastTime >= rule.cooldown &&
        (!rule.scare || state.elapsed - state.lastShock >= ShockGap);
}

EventPlan PlanEvent(const State& source, const Context& context, const std::array<EventRule, EventCount>& rules)
{
    EventPlan plan;
    if (!Validate(source)) return plan;
    plan.before = source;
    plan.after = source;
    auto& state = plan.after;
    const EventRule* selected = nullptr;
    std::uint32_t ties = 0;
    for (const auto& rule : rules)
    {
        if (!Eligible(state, context, rule)) continue;
        const bool higher = !selected || (rule.critical && !selected->critical) ||
            (rule.critical == selected->critical && rule.priority > selected->priority);
        if (higher) { selected = &rule; ties = 1; }
        else if (rule.critical == selected->critical && rule.priority == selected->priority)
        {
            ++ties;
            if (Random(state.randomState) % ties == 0) selected = &rule;
        }
    }
    if (!selected) return plan;
    auto& history = state.history[static_cast<std::size_t>(selected->id)];
    ++history.count;
    history.lastTime = state.elapsed;
    if (selected->scare) state.lastShock = state.elapsed;
    plan.event = selected->id;
    return plan;
}

bool CommitEvent(State& state, const EventPlan& plan, bool delivered)
{
    if (!delivered || plan.event == EventId::Count || !(state == plan.before)) return false;
    state = plan.after;
    return true;
}

EventId SelectEvent(State& state, const Context& context, const std::array<EventRule, EventCount>& rules)
{
    const auto plan = PlanEvent(state, context, rules);
    return CommitEvent(state, plan, true) ? plan.GetEvent() : EventId::Count;
}

std::string_view Objective(Stage stage)
{
    switch (stage)
    {
    case Stage::Arrival: return "Finde das Geburtstagsfoto im Kinderzimmer.";
    case Stage::PhotoFound: return "Vergleiche das Foto mit der leeren Parzelle im Garten.";
    case Stage::DiscrepancySeen: return "Stelle am Geburtstagstisch die Erinnerung wieder her.";
    case Stage::BirthdayRestored: return "Folge der Strasse erneut bis zum Stoppschild.";
    case Stage::RoadUnlocked: return "Untersuche die einzige dunkle Laterne.";
    case Stage::DarkLampFound: return "Betritt das Spielhaus und folge seinem viel zu langen Flur.";
    case Stage::Complete: return "Prototype 0.1 abgeschlossen. Playtime Forever folgt spaeter.";
    }
    return "Ungueltiger Quest-Zustand";
}

std::string_view Hint(Stage stage)
{
    switch (stage)
    {
    case Stage::Arrival: return "Foto auf dem Tisch: ansehen und E druecken. Mit E die Zimmertuer oeffnen.";
    case Stage::PhotoFound: return "Garten rechts der Strasse: Markierung 'Hier fehlt ein Haus' ansehen und E.";
    case Stage::DiscrepancySeen: return "Am Tisch im selben Garten E druecken. Danach wird es Abend.";
    case Stage::BirthdayRestored: return "Gehe zum STOP am Ende der Strasse. Die Schleife fuehrt jetzt weiter.";
    case Stage::RoadUnlocked: return "Die unbeleuchtete Laterne steht am neuen Strassenende. E zum Untersuchen.";
    case Stage::DarkLampFound: return "E am Spielhaus, dann bis zum Flurende gehen und die letzte Tuer mit E pruefen.";
    case Stage::Complete: return "F9 laedt den letzten manuellen Spielstand. Ein Neustart beginnt einen neuen Versuch.";
    }
    return "";
}

std::string_view EventName(EventId id)
{
    switch (id)
    {
    case EventId::GardenLaughter: return "ES04.Garden.Laughter";
    case EventId::HomecomerSighting: return "ES04.Entity.Homecomer";
    case EventId::WindowKnock: return "ES04.Interior.Knock";
    case EventId::Count: return "None";
    }
    return "Invalid";
}
}
