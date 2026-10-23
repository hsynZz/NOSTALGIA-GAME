#pragma once

// Engine-independent production logic, compiled by both Unreal and Tests/core_tests.cpp.
#include <array>
#include <cstdint>
#include <string_view>

namespace Nostalgia
{
enum class Stage : std::uint8_t
{
    Arrival, PhotoFound, DiscrepancySeen, BirthdayRestored, RoadUnlocked, DarkLampFound, Complete
};
enum class Action : std::uint8_t { Photo, ComparePhoto, RestoreBirthday, ExamineLamp, EnterPlayhouse };
enum class District : std::uint8_t { Bedroom, Street, Garden, Interior, Finale };
enum class EventId : std::uint8_t { GardenLaughter, HomecomerSighting, WindowKnock, Count };
enum class Phase : std::uint8_t { Any, Day, Night };
constexpr std::size_t EventCount = static_cast<std::size_t>(EventId::Count);
constexpr int SaveVersion = 1;
constexpr double ShockGap = 45.0;

struct EventHistory
{
    std::uint32_t count = 0;
    double lastTime = -1000000.0;
    bool operator==(const EventHistory&) const = default;
};

struct State
{
    Stage stage = Stage::Arrival;
    double minute = 15.0 * 60.0;
    double elapsed = 0.0;
    std::uint32_t randomState = 2004;
    std::uint32_t loops = 0;
    std::uint8_t openDoors = 0;
    double lastTransition = -1000000.0;
    double lastShock = -1000000.0;
    std::array<EventHistory, EventCount> history{};
    bool operator==(const State&) const = default;
};

struct Context
{
    District district = District::Bedroom;
    bool safeZone = true;
    bool threatened = false;
    double dwell = 0.0;
    double speed = 0.0;
    // Adapters report asset/presentation readiness before arbitration. No unavailable event starves another.
    std::array<bool, EventCount> available{true, true, true};
};

struct EventRule
{
    EventId id;
    District district;
    Stage minimumStage;
    Stage maximumStage;
    Phase phase;
    double minimumDwell;
    double maximumSpeed;
    double cooldown;
    std::uint32_t maximumPlays;
    int priority;
    bool scare;
    bool critical;
};

const std::array<EventRule, EventCount>& DefaultRules();
class EventPlan
{
public:
    EventId GetEvent() const { return event; }
private:
    EventId event = EventId::Count;
    State before;
    State after;
    friend EventPlan PlanEvent(const State&, const Context&, const std::array<EventRule, EventCount>&);
    friend bool CommitEvent(State&, const EventPlan&, bool);
};
// Selection is speculative until the synchronous presentation adapter acknowledges success.
EventPlan PlanEvent(const State& state, const Context& context,
                    const std::array<EventRule, EventCount>& rules = DefaultRules());
bool CommitEvent(State& state, const EventPlan& plan, bool delivered);
District DistrictAt(double x, double y);
bool IsNight(const State& state);
bool Validate(const State& state);
bool ValidateRule(const EventRule& rule);
bool Apply(State& state, Action action);
bool TraverseLoop(State& state);
void Advance(State& state, double realSeconds);
bool Eligible(const State& state, const Context& context, const EventRule& rule);
// Returns Count when no event is eligible. Selection and history mutation are atomic.
EventId SelectEvent(State& state, const Context& context,
                    const std::array<EventRule, EventCount>& rules = DefaultRules());
std::string_view Objective(Stage stage);
std::string_view Hint(Stage stage);
std::string_view EventName(EventId id);
}
