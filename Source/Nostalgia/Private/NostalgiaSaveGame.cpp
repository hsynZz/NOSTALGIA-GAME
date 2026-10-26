#include "NostalgiaSaveGame.h"

bool UNostalgiaSaveGame::ToSnapshot(Nostalgia::SaveSnapshot& Out) const
{
    if (Version != Nostalgia::SaveVersion || Chapter != TEXT("EndlessSummer2004") ||
        Events.Num() != static_cast<int32>(Nostalgia::EventCount)) return false;
    Nostalgia::SaveSnapshot Candidate;
    Candidate.state.stage = static_cast<Nostalgia::Stage>(Stage);
    Candidate.state.minute = Minute;
    Candidate.state.elapsed = Elapsed;
    Candidate.state.randomState = RandomState;
    Candidate.state.loops = Loops;
    Candidate.state.openDoors = OpenDoors;
    Candidate.state.lastTransition = LastTransition;
    Candidate.state.lastShock = LastShock;
    for (int32 Index = 0; Index < Events.Num(); ++Index)
    {
        if (Events[Index].Count < 0) return false;
        Candidate.state.history[Index] = {static_cast<uint32>(Events[Index].Count), Events[Index].LastTime};
    }
    Candidate.position = {PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z};
    Candidate.rotation = {PlayerRotation.Pitch, PlayerRotation.Yaw, PlayerRotation.Roll};
    if (!Nostalgia::ValidateSnapshot(Candidate)) return false;
    Out = Candidate;
    return true;
}

bool UNostalgiaSaveGame::FromSnapshot(const Nostalgia::SaveSnapshot& In)
{
    if (!Nostalgia::ValidateSnapshot(In)) return false;
    Version = Nostalgia::SaveVersion;
    Chapter = TEXT("EndlessSummer2004");
    Stage = static_cast<uint8>(In.state.stage);
    Minute = In.state.minute;
    Elapsed = In.state.elapsed;
    RandomState = In.state.randomState;
    Loops = In.state.loops;
    OpenDoors = In.state.openDoors;
    LastTransition = In.state.lastTransition;
    LastShock = In.state.lastShock;
    Events.Reset();
    for (const auto& Event : In.state.history)
    {
        FNostalgiaSavedEvent Saved;
        Saved.Count = static_cast<int32>(Event.count);
        Saved.LastTime = Event.lastTime;
        Events.Add(Saved);
    }
    PlayerLocation = FVector(In.position.x, In.position.y, In.position.z);
    PlayerRotation = FRotator(In.rotation.x, In.rotation.y, In.rotation.z);
    return true;
}
