#include "Core/NostalgiaCore.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Nostalgia;
namespace
{
void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
State RestoredBirthday()
{
    State state;
    Check(Apply(state, Action::Photo), "photo");
    Check(Apply(state, Action::ComparePhoto), "comparison");
    Check(Apply(state, Action::RestoreBirthday), "birthday");
    return state;
}
Context At(District district) { return {district, false, false, 10, 100}; }
}

int main()
{
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"initial state is valid, summer afternoon, reproducible seed", [] {
            const State state;
            Check(Validate(state), "initial state invalid");
            Check(state.minute == 900 && !IsNight(state) && state.randomState == 2004, "initial conditions");
        }},
        {"escape chain requires all causal steps", [] {
            State state;
            Check(!Apply(state, Action::EnterPlayhouse), "early completion");
            Check(!Apply(state, Action::RestoreBirthday), "early birthday");
            Check(!Apply(state, Action::ComparePhoto), "early comparison");
            Check(!Apply(state, Action::ExamineLamp), "early lamp");
            Check(state.stage == Stage::Arrival, "failed action changed progress");
            Check(Apply(state, Action::Photo), "photo transition");
            Check(!Apply(state, Action::Photo), "duplicate photo");
            Check(Apply(state, Action::ComparePhoto), "comparison transition");
            Check(Apply(state, Action::RestoreBirthday), "memory transition");
            Check(IsNight(state), "memory must cause night");
            Check(!Apply(state, Action::ExamineLamp), "must traverse new route first");
            Check(TraverseLoop(state), "loop transition");
            Check(state.stage == Stage::RoadUnlocked, "loop unlock");
            Check(Apply(state, Action::ExamineLamp), "lamp transition");
            Check(Apply(state, Action::EnterPlayhouse), "exit transition");
            Check(state.stage == Stage::Complete && Validate(state), "completion state");
        }},
        {"early loops remain recoverable", [] {
            State state;
            for (int i=0; i<10; ++i) { Check(TraverseLoop(state), "loop failed"); Advance(state, 3); }
            Check(state.stage == Stage::Arrival && state.loops == 10, "early loop granted progress");
            Apply(state, Action::Photo); Apply(state, Action::ComparePhoto); Apply(state, Action::RestoreBirthday);
            Check(TraverseLoop(state) && state.stage == Stage::RoadUnlocked, "loop softlock");
        }},
        {"loop debounce rejects repeated overlap without mutation", [] {
            State state;
            Check(TraverseLoop(state), "initial loop");
            Check(!TraverseLoop(state) && state.loops == 1, "overlap recursion");
            Advance(state, 2.99);
            Check(!TraverseLoop(state), "cooldown too short");
            Advance(state, .02);
            Check(TraverseLoop(state), "cooldown never released");
        }},
        {"lamp clue requires darkness", [] {
            auto state = RestoredBirthday(); TraverseLoop(state); state.minute = 1000;
            Check(!Apply(state, Action::ExamineLamp), "day lamp accepted");
            state.minute = 1200;
            Check(Apply(state, Action::ExamineLamp), "night lamp rejected");
        }},
        {"completed quest cannot regress or retrigger", [] {
            auto state = RestoredBirthday(); TraverseLoop(state);
            Apply(state, Action::ExamineLamp); Apply(state, Action::EnterPlayhouse);
            for (const auto action : {Action::Photo, Action::ComparePhoto, Action::RestoreBirthday, Action::ExamineLamp, Action::EnterPlayhouse})
                Check(!Apply(state, action) && state.stage == Stage::Complete, "completion regressed");
            Check(SelectEvent(state, At(District::Street)) == EventId::Count, "post-completion scare");
        }},
        {"clock advances deterministically and stops before midnight", [] {
            State state; Advance(state, 1.5);
            Check(state.minute == 901.5 && state.elapsed == 1.5, "clock rate");
            Advance(state, 4000);
            Check(state.minute == 1439 && state.elapsed == 4001.5, "endless date wrap");
            Check(Validate(state), "clock produced invalid state");
        }},
        {"bad time deltas leave state untouched", [] {
            State state;
            for (double delta : {-1.0, 0.0, 86401.0, std::numeric_limits<double>::infinity(), std::nan("")}) Advance(state, delta);
            Check(state.minute == 900 && state.elapsed == 0, "invalid delta mutated state");
        }},
        {"night boundary is explicit", [] {
            State state;
            for (const auto minute : {0., 359.99, 1200., 1439.}) { state.minute = minute; Check(IsNight(state), "expected night"); }
            for (const auto minute : {360., 900., 1199.99}) { state.minute = minute; Check(!IsNight(state), "expected day"); }
        }},
        {"authored home, loop landing and portal anchors keep their intended districts", [] {
            Check(DistrictAt(-1200, 0) == District::Bedroom, "bedroom spawn");
            Check(DistrictAt(200, 0) == District::Bedroom, "living room lost safe home classification");
            Check(DistrictAt(590, 0) == District::Bedroom, "inside front door");
            Check(DistrictAt(950, 0) == District::Street, "loop landing must be outside house");
            Check(DistrictAt(-1700, 3200) == District::Interior, "interior portal classified as home");
            Check(DistrictAt(7200, 0) == District::Finale, "unlocked route destination");
            Check(DistrictAt(8500, 330) == District::Finale, "dark lamp");
        }},
        {"garden source stays local and invalid positions cannot trigger a scare district", [] {
            Check(DistrictAt(2050, 900) == District::Garden, "photo comparison district");
            Check(DistrictAt(2400, 1100) == District::Garden, "birthday table district");
            Check(DistrictAt(2200, 450) == District::Street, "garden laughter leaked to street");
            Check(DistrictAt(std::nan(""), 1000) == District::Bedroom, "invalid position must fail safe");
            Check(DistrictAt(1000, std::numeric_limits<double>::infinity()) == District::Bedroom, "infinite position must fail safe");
        }},
        {"all default event rules validate with unique IDs", [] {
            std::array<bool, EventCount> ids{};
            for (const auto& rule : DefaultRules()) {
                Check(ValidateRule(rule), "bad default rule");
                auto i = static_cast<size_t>(rule.id);
                Check(!ids[i], "duplicate ID"); ids[i] = true;
            }
        }},
        {"garden laughter is local, delayed and single use", [] {
            State state;
            Check(SelectEvent(state, At(District::Street)) == EventId::Count, "global laughter");
            auto context = At(District::Garden); context.dwell = .5;
            Check(SelectEvent(state, context) == EventId::Count, "no dwell gate");
            context.dwell = 1;
            Check(SelectEvent(state, context) == EventId::GardenLaughter, "garden not triggered");
            Advance(state, 1000);
            Check(SelectEvent(state, context) == EventId::Count, "garden spam");
            Check(state.history[0].count == 1, "wrong history count");
        }},
        {"safe zones suppress all event mutation", [] {
            auto state = RestoredBirthday(); auto context = At(District::Street); context.safeZone = true;
            Check(SelectEvent(state, context) == EventId::Count, "unsafe safe zone");
            Check(state.history[1].count == 0, "suppressed event consumed");
            context.safeZone = false;
            Check(SelectEvent(state, context) == EventId::HomecomerSighting, "deferred event lost");
        }},
        {"active threat blocks stacked events", [] {
            State state; auto context = At(District::Garden); context.threatened = true;
            Check(SelectEvent(state, context) == EventId::Count && state.history[0].count == 0, "stacked threat");
        }},
        {"homecomer is progress gated and one shot", [] {
            State state;
            Check(SelectEvent(state, At(District::Street)) == EventId::Count, "premature entity");
            state = RestoredBirthday();
            Check(SelectEvent(state, At(District::Street)) == EventId::HomecomerSighting, "missing sighting");
            Advance(state, 200);
            Check(SelectEvent(state, At(District::Street)) == EventId::Count, "sighting repeats");
        }},
        {"global scare spacing applies across event IDs", [] {
            auto state = RestoredBirthday();
            Check(SelectEvent(state, At(District::Street)) == EventId::HomecomerSighting, "first scare");
            Check(SelectEvent(state, At(District::Interior)) == EventId::Count, "back-to-back scare");
            Advance(state, 44.9);
            Check(SelectEvent(state, At(District::Interior)) == EventId::Count, "short peace interval");
            Advance(state, .2);
            Check(SelectEvent(state, At(District::Interior)) == EventId::WindowKnock, "peace interval never ends");
        }},
        {"audio event respects phase and movement speed", [] {
            State state; Apply(state, Action::Photo);
            Check(SelectEvent(state, At(District::Interior)) == EventId::Count, "day knock");
            state.minute = 1250;
            auto context = At(District::Interior); context.speed = 300;
            Check(SelectEvent(state, context) == EventId::Count, "knock while sprinting");
            context.speed = 0;
            Check(SelectEvent(state, context) == EventId::WindowKnock, "stationary knock missing");
        }},
        {"invalid context cannot consume events", [] {
            State state; auto context = At(District::Garden); context.dwell = std::nan("");
            Check(SelectEvent(state, context) == EventId::Count, "NaN dwell");
            context.dwell = 10; context.speed = -1;
            Check(SelectEvent(state, context) == EventId::Count, "negative speed");
            context.speed = std::nan("");
            Check(SelectEvent(state, context) == EventId::Count, "NaN speed");
        }},
        {"critical event outranks cosmetic priority", [] {
            State state; auto rules = DefaultRules();
            rules[1] = rules[0]; rules[1].id = EventId::HomecomerSighting;
            rules[1].critical = false; rules[1].priority = 10000;
            Check(SelectEvent(state, At(District::Garden), rules) == EventId::GardenLaughter, "critical event starved");
        }},
        {"custom repeat cooldown is enforced", [] {
            State state; auto rules = DefaultRules(); rules[0].maximumPlays = 2;
            Check(SelectEvent(state, At(District::Garden), rules) == EventId::GardenLaughter, "first event");
            Advance(state, 119);
            Check(SelectEvent(state, At(District::Garden), rules) == EventId::Count, "cooldown ignored");
            Advance(state, 1);
            Check(SelectEvent(state, At(District::Garden), rules) == EventId::GardenLaughter, "second event");
            Advance(state, 120);
            Check(SelectEvent(state, At(District::Garden), rules) == EventId::Count, "max plays ignored");
        }},
        {"seeded ties reproduce through a state snapshot", [] {
            auto rules = DefaultRules();
            for (size_t i=0; i<EventCount; ++i) {
                rules[i] = rules[0]; rules[i].id = static_cast<EventId>(i);
                rules[i].maximumPlays = 100; rules[i].cooldown = 0;
            }
            State original;
            for (int i=0; i<5; ++i) SelectEvent(original, At(District::Garden), rules);
            State restored = original;
            for (int i=0; i<40; ++i) {
                Check(SelectEvent(original, At(District::Garden), rules) == SelectEvent(restored, At(District::Garden), rules), "replay diverged");
                Check(original.randomState == restored.randomState, "RNG restore mismatch");
            }
            Check(Validate(original) && Validate(restored), "replay invalid state");
        }},
        {"restored consumed events do not replay", [] {
            State original; SelectEvent(original, At(District::Garden));
            const auto snapshot = original; State restored = snapshot;
            Advance(restored, 300);
            Check(SelectEvent(restored, At(District::Garden)) == EventId::Count, "load replayed garden");
        }},
        {"restored loop debounce remains active", [] {
            auto state = RestoredBirthday(); TraverseLoop(state); State restored = state;
            Check(!TraverseLoop(restored), "load bypasses loop guard");
            Advance(restored, 3); Check(TraverseLoop(restored), "restored loop stuck");
        }},
        {"invalid save stages and progression are rejected", [] {
            State state; state.stage = static_cast<Stage>(255); Check(!Validate(state), "bad enum");
            state = State{}; state.stage = Stage::RoadUnlocked; Check(!Validate(state), "impossible route");
            state.loops = 1; Check(Validate(state), "valid route rejected");
        }},
        {"invalid save clock and seed are rejected", [] {
            for (double time : {-1., 1440., std::nan(""), std::numeric_limits<double>::infinity()}) {
                State state; state.minute = time; Check(!Validate(state), "bad clock accepted");
            }
            State state; state.elapsed = -1; Check(!Validate(state), "bad elapsed");
            state = State{}; state.randomState = 0; Check(!Validate(state), "zero RNG seed");
        }},
        {"corrupt event history and future timestamps rejected", [] {
            State state; state.history[0].lastTime = 100; Check(!Validate(state), "future history");
            state = State{}; state.history[0].count = 1; Check(!Validate(state), "played with no timestamp");
            state = State{}; state.history[0].lastTime = 0; Check(!Validate(state), "unplayed with timestamp");
            state = State{}; state.lastShock = 1; Check(!Validate(state), "future shock");
            state = State{}; state.lastTransition = std::nan(""); Check(!Validate(state), "NaN transition");
            state = State{}; state.history[1].count = 1000001; Check(!Validate(state), "history overflow");
        }},
        {"invalid door bits and loop overflow rejected", [] {
            State state; state.openDoors = 4; Check(!Validate(state), "bad door mask");
            state = State{}; state.loops = 1000001; Check(!Validate(state), "loop count corrupt");
            state = State{}; state.loops = 1000000; Check(!TraverseLoop(state), "loop overflow");
        }},
        {"invalid data rules fail closed without indexing outside history", [] {
            auto rules = DefaultRules();
            rules[0].id = static_cast<EventId>(255);
            Check(!ValidateRule(rules[0]), "invalid event ID");
            State state; Check(SelectEvent(state, At(District::Garden), rules) == EventId::Count, "invalid rule selected");
            auto rule = DefaultRules()[0]; rule.minimumStage = Stage::Complete; Check(!ValidateRule(rule), "inverted stage interval");
            rule = DefaultRules()[0]; rule.cooldown = std::nan(""); Check(!ValidateRule(rule), "NaN cooldown");
            rule = DefaultRules()[0]; rule.maximumPlays = 0; Check(!Eligible(state, At(District::Garden), rule), "disabled rule selected");
        }},
        {"every quest stage has an objective and a recovery hint", [] {
            for (int i=0; i<=6; ++i) {
                Check(!Objective(static_cast<Stage>(i)).empty(), "missing objective");
                Check(!Hint(static_cast<Stage>(i)).empty(), "missing recovery hint");
            }
        }}
    };
    int failed = 0;
    for (const auto& [name, test] : tests)
    {
        try { test(); std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& error) { ++failed; std::cerr << "FAIL " << name << ": " << error.what() << '\n'; }
    }
    std::cout << "CORE_RESULT passed=" << tests.size() - failed << " failed=" << failed << " total=" << tests.size() << '\n';
    return failed == 0 ? 0 : 1;
}
