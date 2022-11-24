#pragma once
#include <numbers>

#include "common/types.h"
#include "common/math.h"

// ~ 0012F1C0 - 00131110

#define VALID_INDEX(arr, n) (n >= 0 && n < std::size(arr))

// Clamps an angle to (-pi, +pi] radians.
// (Only valid for f32s within (-2pi, +2pi].
// Use mgAngleLimit if your f32 is not within that range.)
inline f32 mgAngleClamp(f32 x)
{
  if (x > common::deg_to_rad(180.0f))
  {
    return x - common::deg_to_rad(360.0f);
  }
  if (x <= common::deg_to_rad(-180.0f))
  {
    return x + common::deg_to_rad(360.0f);
  }
  return x;
}

// 00130B60
f32 mgAngleInterpolate(f32 f1, f32 f2, f32 f3, bool b);

// 00130D10
s32 mgAngleCmp(f32 f1, f32 f2, f32 f3);

// 00130DD0
f32 mgAngleLimit(f32 f);

// 00130EE0
f32 mgRnd();

// 00130F20
// for when you want a rand that returns [-6, 6] but really biases towards 0
// for whatever reason
f32 mgNRnd();