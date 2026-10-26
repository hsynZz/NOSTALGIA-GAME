#pragma once

namespace Nostalgia
{
// Centimeters/degrees. Presentation never changes collision, aim or quest state.
struct ViewMotion
{
    double eyeHeight = 64;
    double bob = 0;
    double phase = 0;
    double fieldOfView = 82;
};
void AdvanceView(ViewMotion& view, double seconds, double speed, bool grounded,
                 bool crouched, bool sprinting, bool reducedMotion);
}
