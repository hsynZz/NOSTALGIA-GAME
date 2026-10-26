#include "Core/NostalgiaPresentation.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace Nostalgia
{
void AdvanceView(ViewMotion& view, double seconds, double speed, bool grounded,
                 bool crouched, bool sprinting, bool reducedMotion)
{
    if (!std::isfinite(seconds) || !std::isfinite(speed) || seconds <= 0 || speed < 0) return;
    // Limit the visual response after suspension/debugger pauses. No time debt.
    const double dt = std::min(seconds, .1);
    const auto approach = [dt](double value, double target, double rate)
        { return target + (value - target) * std::exp(-rate * dt); };
    view.eyeHeight = approach(view.eyeHeight, crouched ? 30.0 : 64.0, 14);
    const bool walking = grounded && speed > 10;
    if (walking)
        view.phase = std::fmod(view.phase + dt * std::min(speed, 520.0) / 145.0 * 2 * std::numbers::pi,
                              2 * std::numbers::pi);
    const double amplitude = reducedMotion || !walking ? 0 : (crouched ? .22 : .55);
    view.bob = approach(view.bob, amplitude * std::sin(view.phase), 16);
    // No roll, aim smoothing, blur, stamina zoom or heartbeat camera shake.
    view.fieldOfView = approach(view.fieldOfView, !reducedMotion && sprinting && walking ? 85.0 : 82.0, 4);
}
}
