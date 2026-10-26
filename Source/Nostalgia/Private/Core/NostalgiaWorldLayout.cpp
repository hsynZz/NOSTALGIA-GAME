#include "Core/NostalgiaWorldLayout.h"
#include <cmath>

namespace Nostalgia::Layout
{
namespace
{
bool Finite(Point point)
{
    return std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z);
}
}

bool Contains(const Bounds& bounds, Point point, double margin)
{
    return Finite(point) && std::isfinite(margin) && margin >= 0 &&
        point.x >= bounds.minX + margin && point.x <= bounds.maxX - margin &&
        point.y >= bounds.minY + margin && point.y <= bounds.maxY - margin;
}

bool OverlapsLoopTrigger(Point point)
{
    return Finite(point) &&
        std::abs(point.x - LoopOrigin.x) <= LoopTriggerHalfExtent.x + PlayerRadius &&
        std::abs(point.y - LoopOrigin.y) <= LoopTriggerHalfExtent.y + PlayerRadius &&
        std::abs(point.z - LoopOrigin.z) <= LoopTriggerHalfExtent.z + StandingHalfHeight;
}

bool CanRestoreAt(Point point, Stage stage)
{
    if (!Finite(point) || point.z < 40 || point.z > 800 || stage > Stage::Complete || OverlapsLoopTrigger(point))
        return false;
    if (Contains(NeighborhoodXY, point, PlayerRadius) || Contains(HouseInteriorXY, point, PlayerRadius))
        return true;
    if (Contains(FinaleXY, point, PlayerRadius)) return stage >= Stage::RoadUnlocked;
    if (Contains(PlayhouseInteriorXY, point, PlayerRadius)) return stage >= Stage::DarkLampFound;
    return false;
}

District DistrictAt(Point point)
{
    if (!Finite(point)) return District::Bedroom;
    if (Contains(HouseInteriorXY, point)) return District::Interior;
    if (Contains(HomeSafeXY, point)) return District::Bedroom;
    if (Contains(FinaleXY, point) || Contains(PlayhouseInteriorXY, point)) return District::Finale;
    if (point.x > GardenXY.minX && point.x < GardenXY.maxX && point.y > GardenXY.minY && point.y < GardenXY.maxY)
        return District::Garden;
    return District::Street;
}
}
