#include "NostalgiaEventCatalog.h"

Nostalgia::EventRule FNostalgiaEventRule::ToCore() const
{
    return {static_cast<Nostalgia::EventId>(Event), static_cast<Nostalgia::District>(District),
        static_cast<Nostalgia::Stage>(MinimumStage), static_cast<Nostalgia::Stage>(MaximumStage),
        static_cast<Nostalgia::Phase>(Phase), MinimumDwell, MaximumSpeed, Cooldown,
        static_cast<uint32>(MaximumPlays), Priority, bScare, bCritical};
}

UNostalgiaEventCatalog::UNostalgiaEventCatalog()
{
    for (const auto& Default : Nostalgia::DefaultRules())
    {
        FNostalgiaEventRule Rule;
        Rule.Event = static_cast<ENostalgiaEvent>(Default.id);
        Rule.District = static_cast<uint8>(Default.district);
        Rule.MinimumStage = static_cast<uint8>(Default.minimumStage);
        Rule.MaximumStage = static_cast<uint8>(Default.maximumStage);
        Rule.Phase = static_cast<uint8>(Default.phase);
        Rule.MinimumDwell = Default.minimumDwell;
        Rule.MaximumSpeed = Default.maximumSpeed;
        Rule.Cooldown = Default.cooldown;
        Rule.MaximumPlays = Default.maximumPlays;
        Rule.Priority = Default.priority;
        Rule.bScare = Default.scare;
        Rule.bCritical = Default.critical;
        Rules.Add(Rule);
    }
}

bool UNostalgiaEventCatalog::GetValidatedRules(std::array<Nostalgia::EventRule, Nostalgia::EventCount>& OutRules) const
{
    if (Rules.Num() != static_cast<int32>(Nostalgia::EventCount)) return false;
    std::array<bool, Nostalgia::EventCount> Seen{};
    auto Candidate = Nostalgia::DefaultRules();
    for (const auto& Rule : Rules)
    {
        const auto Core = Rule.ToCore();
        if (!Nostalgia::ValidateRule(Core)) return false;
        const auto Index = static_cast<size_t>(Core.id);
        if (Seen[Index]) return false;
        Seen[Index] = true;
        Candidate[Index] = Core;
    }
    OutRules = Candidate;
    return true;
}
