#pragma once

#include "Core/NostalgiaCore.h"

// Authored coordinates shared by the Unreal adapters and native validation.
// This describes permitted regions and transition anchors, not a physics engine.
namespace Nostalgia::Layout
{
struct Point { double x, y, z; };
struct Bounds { double minX, maxX, minY, maxY; };

inline constexpr double PlayerRadius = 34;
inline constexpr double StandingHalfHeight = 88;
inline constexpr Point Spawn{-1200, 0, 100};
inline constexpr Point LoopOrigin{5700, 0, 150};
inline constexpr Point LoopTriggerHalfExtent{100, 470, 160};
inline constexpr Point LoopLockedLanding{950, 0, 96};
inline constexpr Point LoopUnlockedLanding{7200, 0, 96};
inline constexpr Point HouseEntryLanding{-1700, 3200, 96};
inline constexpr Point HouseReturnLanding{1200, -900, 96};
inline constexpr Point PlayhouseLanding{7150, 3000, 96};
inline constexpr Point PlayhouseReturnLanding{8550, 450, 96};
inline constexpr Point StreetReturnLanding{5300, 0, 96};

inline constexpr Bounds NeighborhoodXY{-1900, 6100, -2350, 2350};
inline constexpr Bounds HouseInteriorXY{-2100, 100, 2700, 3700};
inline constexpr Bounds PlayhouseInteriorXY{6900, 9100, 2800, 3200};
inline constexpr Bounds FinaleXY{6500, 9500, -700, 700};
inline constexpr Bounds HomeSafeXY{-1800, 600, -420, 420};
inline constexpr Bounds GardenXY{1550, 2850, 500, 1650};
inline constexpr Point NeighborhoodGroundCenter{2100, 0, -30};
inline constexpr Point NeighborhoodGroundSize{8000, 4700, 20};

bool Contains(const Bounds& bounds, Point point, double margin = 0);
bool OverlapsLoopTrigger(Point point);
bool CanRestoreAt(Point point, Stage stage);
District DistrictAt(Point point);
}
