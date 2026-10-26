#include "Core/NostalgiaCore.h"
#include "Core/NostalgiaEncounter.h"
#include "Core/NostalgiaSnapshot.h"
#include "Core/NostalgiaPresentation.h"
#include <algorithm>
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
        {"quality real-time clock preserves Director dwell seconds", [] {
            State s;
            Advance(s, 120, 1.0/60.0);
            Check(s.minute == 902 && s.elapsed == 120, "clock and elapsed mixed");
            const State before = s;
            for (double rate : {-1.0, 61.0, std::nan("")}) Advance(s, 1, rate);
            Check(s == before, "invalid clock rate mutated state");
        }},
        {"view motion is bounded and absent at rest", [] {
            ViewMotion v;
            for (int i=0;i<600;++i)
            {
                AdvanceView(v, 1.0/60, 520, true, false, true, false);
                Check(std::abs(v.bob) <= .55 && v.fieldOfView >= 82 && v.fieldOfView <= 85, "excess motion");
            }
            for (int i=0;i<600;++i) AdvanceView(v, 1.0/60, 0, true, false, false, false);
            Check(std::abs(v.bob) < .001 && std::abs(v.fieldOfView-82)<.001, "idle drift");
        }},
        {"view comfort suppresses bob and sprint zoom", [] {
            ViewMotion v;
            for (int i=0;i<120;++i) AdvanceView(v,1.0/60,520,true,false,true,true);
            Check(v.bob == 0 && v.fieldOfView == 82, "reduced motion ignored");
        }},
        {"view crouch settling is independent of framerate", [] {
            ViewMotion a,b;
            for(int i=0;i<30;++i) AdvanceView(a,1.0/30,0,true,true,false,false);
            for(int i=0;i<120;++i) AdvanceView(b,1.0/120,0,true,true,false,false);
            Check(std::abs(a.eyeHeight-b.eyeHeight)<1e-8 && std::abs(a.eyeHeight-30)<.001, "frame dependent crouch");
            const auto before=a;
            AdvanceView(a,std::nan(""),200,true,false,false,false);
            Check(a.eyeHeight == before.eyeHeight, "invalid motion input");
        }},
        {"airborne camera does not invent footsteps", [] {
            ViewMotion v;
            for(int i=0;i<120;++i) AdvanceView(v,1.0/60,320,false,false,false,false);
            Check(v.phase == 0 && v.bob == 0, "airborne bob");
        }},
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
        {"planning and failed delivery preserve history, peace time and RNG", [] {
            auto state = RestoredBirthday(); const State before = state;
            const auto plan = PlanEvent(state, At(District::Street));
            Check(plan.GetEvent() == EventId::HomecomerSighting, "missing plan");
            Check(state == before, "planning consumed state");
            Check(!CommitEvent(state, plan, false) && state == before, "failed presentation consumed event");
            Check(CommitEvent(state, PlanEvent(state, At(District::Street)), true), "failed presentation could not retry");
            Check(state.history[1].count == 1 && state.lastShock == state.elapsed, "acknowledgment missing history");
        }},
        {"an event receipt cannot commit twice", [] {
            State state; const auto plan = PlanEvent(state, At(District::Garden));
            Check(CommitEvent(state, plan, true), "first commit");
            const auto delivered = state;
            Check(!CommitEvent(state, plan, true) && state == delivered, "receipt replay");
        }},
        {"stale receipts cannot overwrite time, quest, doors or other events", [] {
            for (int mutation = 0; mutation < 4; ++mutation) {
                State state; const auto plan = PlanEvent(state, At(District::Garden));
                if (mutation == 0) Advance(state, .1);
                if (mutation == 1) Apply(state, Action::Photo);
                if (mutation == 2) state.openDoors = 1;
                if (mutation == 3) SelectEvent(state, At(District::Garden));
                const auto changed = state;
                Check(!CommitEvent(state, plan, true) && state == changed, "stale commit overwrote live state");
            }
        }},
        {"unavailable high priority presentation does not starve a ready event", [] {
            State state; auto context = At(District::Garden); auto rules = DefaultRules();
            rules[1] = rules[0]; rules[1].id = EventId::HomecomerSighting; rules[1].priority = 1;
            context.available[0] = false;
            const auto plan = PlanEvent(state, context, rules);
            Check(plan.GetEvent() == EventId::HomecomerSighting, "unavailable candidate blocked fallback");
            Check(CommitEvent(state, plan, true) && state.history[0].count == 0, "unavailable history changed");
        }},
        {"missing assets and offscreen sightings remain pending without mutation", [] {
            auto state = RestoredBirthday(); auto context = At(District::Street);
            context.available.fill(false); const auto before = state;
            for (int i=0; i<100; ++i) {
                const auto plan = PlanEvent(state, context);
                Check(plan.GetEvent() == EventId::Count && !CommitEvent(state, plan, true), "unavailable event delivered");
            }
            Check(state == before, "unavailable event mutated history");
            context.available[1] = true;
            Check(SelectEvent(state, context) == EventId::HomecomerSighting, "ready sighting lost");
        }},
        {"failed seeded tie selection is stable on retry and after snapshot restore", [] {
            State state; auto rules = DefaultRules();
            for (size_t i=0; i<EventCount; ++i) { rules[i] = rules[0]; rules[i].id = static_cast<EventId>(i); }
            const auto before = state; const auto first = PlanEvent(state, At(District::Garden), rules);
            Check(!CommitEvent(state, first, false) && state == before, "failed tie changed random state");
            const auto second = PlanEvent(state, At(District::Garden), rules);
            Check(first.GetEvent() == second.GetEvent(), "retry rerolled event");
            auto restored = before;
            Check(CommitEvent(state, second, true), "retry commit");
            SelectEvent(restored, At(District::Garden), rules);
            Check(state == restored, "snapshot replay diverged");
        }},
        {"invalid states and empty plans cannot dispatch", [] {
            State state; state.randomState = 0;
            const auto plan = PlanEvent(state, At(District::Garden));
            Check(plan.GetEvent() == EventId::Count && !CommitEvent(state, plan, true), "invalid state dispatched");
            state = State{};
            Check(!CommitEvent(state, EventPlan{}, true), "empty receipt committed");
        }},
        {"homecomer requires a visible authored distance outside safe zones", [] {
            Encounter e; Perception p{1000, .9, true, false, false, true};
            Check(CanStartEncounter(e, p), "valid sighting blocked");
            for (int gate=0; gate<7; ++gate) {
                auto bad = p;
                if (gate == 0) bad.safe = true;
                if (gate == 1) bad.lineOfSight = false;
                if (gate == 2) bad.viewDot = .59;
                if (gate == 3) bad.distance = 599;
                if (gate == 4) bad.distance = 2201;
                if (gate == 5) bad.blocked = true;
                if (gate == 6) bad.inView = false;
                Check(!StartEncounter(e, bad) && e.phase == EncounterPhase::Dormant, "invalid sighting started");
            }
            Check(StartEncounter(e, p) && !StartEncounter(e, p), "reentrant encounter");
        }},
        {"observed homecomer stays still then retires within eleven seconds", [] {
            Encounter e; Perception p{1000, .9, true, false, false, true}; StartEncounter(e, p);
            for (int frame=0; frame<44; ++frame) {
                const double travel = StepEncounter(e, p, .25);
                Check(travel <= 0, "observed entity approached");
            }
            Check(e.phase == EncounterPhase::Cooldown && !EncounterActive(e), "encounter never retired");
            Check(!StartEncounter(e, p), "cooldown restarted without reset");
        }},
        {"looking away permits at most 150 cm after the quiet introduction", [] {
            Encounter e; Perception p{1000, .9, true, false, false, true}; StartEncounter(e, p); p.viewDot = .1;
            for (int frame=0; frame<7; ++frame) Check(StepEncounter(e, p, .25) == 0, "no introduction");
            double approached = 0;
            for (int frame=0; frame<29; ++frame) approached += std::max(0.0, StepEncounter(e, p, .25));
            Check(approached == 150 && e.approachDistance == 150, "unbounded approach");
        }},
        {"gaze and occlusion stop approach immediately without resetting duration", [] {
            Encounter e; Perception p{1000, .9, true, false, false, true}; StartEncounter(e, p);
            for (int i=0; i<8; ++i) StepEncounter(e, p, .25);
            p.viewDot = 0;
            Check(StepEncounter(e, p, .25) > 0, "look away did not approach");
            p.lineOfSight = false;
            Check(StepEncounter(e, p, .25) == 0, "approached around occlusion");
            p.lineOfSight = true; p.viewDot = .9;
            Check(StepEncounter(e, p, .25) == 0 && e.elapsed == 2.75, "gaze failed or timer reset");
        }},
        {"close player and blocked motion trigger bounded retirement", [] {
            for (bool blocked : {false, true}) {
                Encounter e; Perception p{1000, .9, true, false, false, true}; StartEncounter(e, p);
                p.blocked = blocked; p.distance = blocked ? 1000 : 440;
                for (int i=0; i<8; ++i) Check(StepEncounter(e, p, .25) <= 0, "continued approaching");
                Check(e.phase == EncounterPhase::Cooldown, "stuck encounter did not retire");
            }
        }},
        {"approach step cannot cross the minimum distance", [] {
            Encounter e; Perception p{1000, .9, true, false, false, true}; StartEncounter(e, p);
            for (int i=0; i<8; ++i) StepEncounter(e, p, .25);
            p.distance = 451; p.viewDot = 0;
            Check(StepEncounter(e, p, .25) == 1, "approach overshot minimum gap");
            p.distance = 450;
            Check(StepEncounter(e, p, .25) == 0, "approached at minimum gap");
        }},
        {"safe zones end encounter immediately and reset restores dormant presentation", [] {
            Encounter e; Perception p{1000, .9, true, false, false, true}; StartEncounter(e, p);
            p.safe = true;
            Check(StepEncounter(e, p, .1) == 0 && !EncounterActive(e), "threat entered safe zone");
            e = Encounter{}; p.safe = false;
            Check(CanStartEncounter(e, p), "load/reset did not restore presentation");
        }},
        {"invalid perception fails safe and invalid frame deltas do not move", [] {
            Encounter e; Perception p{1000, .9, true, false, false, true}; StartEncounter(e, p);
            for (double delta : {-1., 0., .251, std::nan(""), std::numeric_limits<double>::infinity()})
                Check(StepEncounter(e, p, delta) == 0 && e.elapsed == 0, "invalid frame progressed");
            p.distance = std::nan("");
            Check(StepEncounter(e, p, .1) == 0 && !EncounterActive(e), "invalid sensor remained dangerous");
            Check(!CanStartEncounter(Encounter{}, p), "invalid sensor started encounter");
        }},
        {"authored playhouse corridor remains in the safe final district", [] {
            for (double x : {6940., 7150., 8000., 9000.})
                Check(DistrictAt(x, 3000) == District::Finale, "corridor permitted scare");
            Check(DistrictAt(5300, 0) == District::Street, "return portal lost street");
        }},
        {"causal sequence survives failed audio, encounter and final return visit", [] {
            State state; Apply(state, Action::Photo);
            auto garden = At(District::Garden);
            auto audio = PlanEvent(state, garden);
            Check(!CommitEvent(state, audio, false) && state.history[0].count == 0, "missing audio consumed");
            Check(CommitEvent(state, PlanEvent(state, garden), true), "audio retry failed");
            Apply(state, Action::ComparePhoto); Apply(state, Action::RestoreBirthday);
            auto street = At(District::Street); street.available[1] = false;
            Check(PlanEvent(state, street).GetEvent() == EventId::Count, "offscreen sighting consumed");
            Encounter encounter; Perception sight{1000, .9, true, false, false, true};
            street.available[1] = CanStartEncounter(encounter, sight);
            auto event = PlanEvent(state, street);
            Check(CommitEvent(state, event, StartEncounter(encounter, sight)), "encounter not acknowledged");
            auto suppressed = garden; suppressed.threatened = EncounterActive(encounter);
            Check(PlanEvent(state, suppressed).GetEvent() == EventId::Count, "stacked event");
            for (int i=0; i<44; ++i) { Advance(state, .25); StepEncounter(encounter, sight, .25); }
            Check(TraverseLoop(state) && Apply(state, Action::ExamineLamp), "lamp chain failed");
            const auto saved = state;
            Advance(state, 3); Check(TraverseLoop(state), "return visit loop failed");
            Check(state.stage == Stage::DarkLampFound, "return regressed quest");
            state = saved;
            Check(Apply(state, Action::EnterPlayhouse) && Validate(state), "restored final action failed");
            Check(state.history[0].count == 1 && state.history[1].count == 1, "delivery history wrong");
        }},
        {"all authored transition landings support valid restore regions", [] {
            for (const auto point : {Layout::Spawn, Layout::LoopLockedLanding, Layout::HouseEntryLanding,
                                   Layout::HouseReturnLanding, Layout::StreetReturnLanding})
                Check(Layout::CanRestoreAt(point, Stage::Arrival), "ordinary landing outside region");
            Check(Layout::CanRestoreAt(Layout::LoopUnlockedLanding, Stage::RoadUnlocked), "unlocked landing");
            Check(Layout::CanRestoreAt(Layout::PlayhouseReturnLanding, Stage::RoadUnlocked), "lamp return landing");
            Check(Layout::CanRestoreAt(Layout::PlayhouseLanding, Stage::DarkLampFound), "corridor landing");
        }},
        {"restore cannot bypass the birthday loop or dark lamp", [] {
            for (const auto stage : {Stage::Arrival, Stage::PhotoFound, Stage::DiscrepancySeen, Stage::BirthdayRestored})
                Check(!Layout::CanRestoreAt(Layout::LoopUnlockedLanding, stage), "early finale restore");
            Check(!Layout::CanRestoreAt(Layout::PlayhouseLanding, Stage::RoadUnlocked), "early corridor restore");
            Check(Layout::CanRestoreAt(Layout::PlayhouseLanding, Stage::Complete), "complete corridor restore");
        }},
        {"voids between interiors and outdoors are excluded from restore", [] {
            for (const auto p : {Layout::Point{1000,3200,96}, Layout::Point{6400,0,96},
                                 Layout::Point{8000,2000,96}, Layout::Point{-2200,0,96}, Layout::Point{9900,0,96}})
                Check(!Layout::CanRestoreAt(p, Stage::Complete), "old broad rectangle accepted empty region");
        }},
        {"restore regions leave room for the player capsule at edges", [] {
            const double edge = Layout::NeighborhoodXY.minY + Layout::PlayerRadius;
            Check(Layout::CanRestoreAt({1000,edge,96}, Stage::Arrival), "valid margin rejected");
            Check(!Layout::CanRestoreAt({1000,edge-.01,96}, Stage::Arrival), "capsule crosses boundary");
            Check(!Layout::Contains(Layout::NeighborhoodXY, {0,0,96}, -1), "negative margin");
        }},
        {"saved capsules overlapping the loop are rejected before teleport", [] {
            const auto p = Layout::LoopOrigin;
            Check(Layout::OverlapsLoopTrigger(p), "trigger center missed");
            Check(!Layout::CanRestoreAt(p, Stage::BirthdayRestored), "load could trigger a loop");
            Check(!Layout::CanRestoreAt({p.x+133.9,p.y,96}, Stage::RoadUnlocked), "capsule expansion missing");
            Check(!Layout::CanRestoreAt({p.x,p.y+503.9,96}, Stage::RoadUnlocked), "side capsule expansion missing");
            Check(Layout::CanRestoreAt({p.x,p.y+505,96}, Stage::Arrival), "unrelated side yard rejected");
        }},
        {"neighborhood floor covers side yards and the complete bounded rectangle", [] {
            const auto center = Layout::NeighborhoodGroundCenter;
            const auto size = Layout::NeighborhoodGroundSize;
            const Layout::Bounds floor{center.x-size.x/2, center.x+size.x/2, center.y-size.y/2, center.y+size.y/2};
            for (const double x : {Layout::NeighborhoodXY.minX, Layout::NeighborhoodXY.maxX})
                for (const double y : {Layout::NeighborhoodXY.minY, Layout::NeighborhoodXY.maxY})
                    Check(Layout::Contains(floor, {x,y,0}), "bounded neighborhood has no floor");
            Check(Layout::Contains(floor, {-1200,1200,0}), "rear garden floor missing");
            Check(Layout::Contains(floor, {-1000,-1500,0}), "side garden floor missing");
            Check(center.z + size.z/2 == -20, "lawn surface height changed");
        }},
        {"district classification uses bounded authored rooms", [] {
            Check(DistrictAt(-1000,3200) == District::Interior, "house region");
            Check(DistrictAt(-1000,5000) == District::Street, "unbounded interior classification");
            Check(DistrictAt(-5000,0) == District::Street, "unbounded safe home classification");
            Check(DistrictAt(8000,2000) == District::Street, "unbounded finale classification");
        }},
        {"snapshot validates a new game and normal quest progression", [] {
            SaveSnapshot snapshot;
            Check(ValidateSnapshot(snapshot), "new snapshot invalid");
            for (const auto action : {Action::Photo, Action::ComparePhoto, Action::RestoreBirthday}) {
                Check(Apply(snapshot.state, action), "quest action");
                Check(ValidateSnapshot(snapshot), "valid progress snapshot rejected");
            }
            Check(TraverseLoop(snapshot.state), "loop");
            snapshot.position = Layout::LoopUnlockedLanding;
            Check(ValidateSnapshot(snapshot), "valid finale snapshot rejected");
            Apply(snapshot.state, Action::ExamineLamp); snapshot.position = Layout::PlayhouseLanding;
            Check(ValidateSnapshot(snapshot), "valid corridor snapshot rejected");
            Apply(snapshot.state, Action::EnterPlayhouse);
            Check(ValidateSnapshot(snapshot), "complete snapshot rejected");
        }},
        {"snapshot rejects corrupt position, rotation and progression", [] {
            for (const double bad : {std::nan(""), std::numeric_limits<double>::infinity()}) {
                SaveSnapshot s; s.position.x = bad; Check(!ValidateSnapshot(s), "bad x");
                s = SaveSnapshot{}; s.rotation.y = bad; Check(!ValidateSnapshot(s), "bad yaw");
            }
            SaveSnapshot s; s.position.z = 801; Check(!ValidateSnapshot(s), "bad height");
            s = SaveSnapshot{}; s.rotation.x = 361; Check(!ValidateSnapshot(s), "unbounded rotation");
            s = SaveSnapshot{}; s.state = RestoredBirthday(); s.state.minute = 1000;
            Check(!ValidateSnapshot(s), "birthday clock reversal accepted");
            s = SaveSnapshot{}; s.state.loops = 1;
            Check(!ValidateSnapshot(s), "loop without transition timestamp");
            s = SaveSnapshot{}; s.state.lastTransition = 0;
            Check(!ValidateSnapshot(s), "transition timestamp without loop");
        }},
        {"snapshot accepts wrapped and signed Unreal view angles", [] {
            SaveSnapshot s;
            for (const double angle : {-360., -90., 0., 270., 360.}) {
                s.rotation = {angle, angle, 0};
                Check(ValidateSnapshot(s), "valid UE angle rejected");
            }
        }},
        {"door preflight distinguishes saved open and closed poses", [] {
            const Layout::Point hinge{-200,-110,0};
            Check(CapsuleOverlapsDoor({-200,0,96}, hinge, false), "closed panel missed");
            Check(!CapsuleOverlapsDoor({-200,0,96}, hinge, true), "valid open doorway blocked");
            Check(CapsuleOverlapsDoor({-90,-110,96}, hinge, true), "open panel missed");
            Check(!CapsuleOverlapsDoor({-90,-110,96}, hinge, false), "closed pose incorrectly uses open panel");
        }},
        {"door preflight handles capsule edges and overhead clearance", [] {
            const Layout::Point hinge{0,0,0};
            Check(CapsuleOverlapsDoor({43.9,110,96}, hinge, false), "capsule side missed");
            Check(!CapsuleOverlapsDoor({44.1,110,96}, hinge, false), "capsule side false overlap");
            Check(CapsuleOverlapsDoor({0,110,327.9}, hinge, false), "capsule lower cap missed");
            Check(!CapsuleOverlapsDoor({0,110,328.1}, hinge, false), "capsule above door blocked");
            // Rounded lower cap does not fill the enclosing AABB's corners.
            Check(!CapsuleOverlapsDoor({43,110,327}, hinge, false), "box approximation blocks clear cap corner");
        }},
        {"door collision model is equivalent under the authored ninety degree rotation", [] {
            const Layout::Point hinge{600,-110,0};
            for (int x=-80; x<=300; x+=10)
                for (int y=-80; y<=300; y+=10) {
                    const Layout::Point p{hinge.x+x,hinge.y+y,96};
                    const Layout::Point rotated{hinge.x-y,hinge.y+x,96};
                    Check(CapsuleOverlapsDoor(p, hinge, true) == CapsuleOverlapsDoor(rotated, hinge, false),
                        "open pose rotation differs from panel geometry");
                }
        }},
        {"malformed collision dimensions fail closed", [] {
            const Layout::Point p{0,0,96}, hinge{0,0,0};
            Check(CapsuleOverlapsDoor(p, hinge, false, -1,88), "negative radius");
            Check(CapsuleOverlapsDoor(p, hinge, false, 34,10), "invalid capsule height");
            Check(CapsuleOverlapsDoor({std::nan(""),0,96}, hinge, true), "bad position");
        }},
        {"persisted snapshot keeps director history, RNG and door states", [] {
            SaveSnapshot original; original.state = RestoredBirthday(); original.state.openDoors = 3;
            SelectEvent(original.state, At(District::Street));
            TraverseLoop(original.state); Apply(original.state, Action::ExamineLamp);
            original.position = Layout::PlayhouseLanding;
            const SaveSnapshot restored = original;
            Check(ValidateSnapshot(restored), "snapshot invalid");
            auto replay = restored.state; Advance(replay, 200);
            Check(SelectEvent(replay, At(District::Street)) == EventId::Count, "consumed sighting replayed");
            Check(replay.openDoors == 3 && replay.randomState == original.state.randomState, "saved state changed");
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
