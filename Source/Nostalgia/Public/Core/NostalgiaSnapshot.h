#pragma once

#include "Core/NostalgiaWorldLayout.h"

namespace Nostalgia
{
// Portable validation model; Unreal's USaveGame remains the on-disk representation.
struct SaveSnapshot
{
    State state;
    Layout::Point position = Layout::Spawn;
    // Pitch, yaw, roll in degrees. UE rotations may use either signed or wrapped degrees.
    Layout::Point rotation{0, 0, 0};
};
bool ValidateSnapshot(const SaveSnapshot& snapshot);
// The UE door panel is 20x220x240 cm, hinged at local (0,0,0), opening to -90 degrees.
bool CapsuleOverlapsDoor(Layout::Point capsule, Layout::Point hinge, bool open,
                         double radius = Layout::PlayerRadius,
                         double halfHeight = Layout::StandingHalfHeight);
}
