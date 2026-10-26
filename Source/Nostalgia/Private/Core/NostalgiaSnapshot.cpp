#include "Core/NostalgiaSnapshot.h"
#include <algorithm>
#include <cmath>

namespace Nostalgia
{
namespace
{
bool Finite(Layout::Point p) { return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z); }
}
bool ValidateSnapshot(const SaveSnapshot& snapshot)
{
    const auto& state = snapshot.state;
    if (!Validate(state) || !Layout::CanRestoreAt(snapshot.position, state.stage) || !Finite(snapshot.rotation) ||
        std::abs(snapshot.rotation.x) > 360 || std::abs(snapshot.rotation.y) > 360 ||
        std::abs(snapshot.rotation.z) > 360) return false;
    // These transitions cannot be undone by the production clock or quest logic.
    if (state.stage >= Stage::BirthdayRestored && state.minute < 1204) return false;
    if ((state.loops == 0 && state.lastTransition != -1000000.0) ||
        (state.loops > 0 && state.lastTransition < 0)) return false;
    return true;
}

bool CapsuleOverlapsDoor(Layout::Point capsule, Layout::Point hinge, bool open, double radius, double halfHeight)
{
    // Invalid collision input fails closed: never authorize a restore using malformed dimensions.
    if (!Finite(capsule) || !Finite(hinge) || !std::isfinite(radius) || !std::isfinite(halfHeight) ||
        radius <= 0 || halfHeight < radius) return true;
    const double dx = capsule.x - hinge.x;
    const double dy = capsule.y - hinge.y;
    // Transform capsule center into the unrotated panel's local frame. Z axis is unaffected by yaw.
    const double localX = open ? -dy : dx;
    const double localY = (open ? dx : dy) - 110;
    const double localZ = capsule.z - hinge.z - 120;
    const double gapX = std::max(0.0, std::abs(localX) - 10);
    const double gapY = std::max(0.0, std::abs(localY) - 110);
    const double gapZ = std::max(0.0, std::abs(localZ) - (halfHeight - radius) - 120);
    return gapX * gapX + gapY * gapY + gapZ * gapZ <= radius * radius;
}
}
