#include <array>

#include "common/math.h"
#include "common/log.h"

#include "dc2/mg/mg_math.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

set_log_channel("mg_math");

// 0037FD40
static std::array<f32, 1024> SinTable{};
// 00376504
constexpr static f32 sin_table_unit_1 = static_cast<f32>(SinTable.size()) / common::pi2();

// 0012F1D0
mgVu0FBOX8 mgCreateBox8(const vec4& c1, const vec4& c2)
{
  log_trace("{}({}, {})", __func__, fmt::ptr(&c1), fmt::ptr(&c2));

  // TODO: Check if this produces better code with swizzling.
  mgVu0FBOX8 result;

  result.vertices[0] = c2;
  result.vertices[1] = c2;
  result.vertices[2] = c2;
  result.vertices[3] = c2;
  result.vertices[4] = c1;
  result.vertices[5] = c1;
  result.vertices[6] = c1;
  result.vertices[7] = c1;

  result.vertices[1].x = c1.x;
  result.vertices[2].y = c1.y;
  result.vertices[3].z = c1.z;
  result.vertices[4].x = c2.x;
  result.vertices[5].y = c2.y;
  result.vertices[6].z = c2.z;
  return result;
}

// 0012F250
bool mgClipBoxVertex(const vec4& v1, const vec4& v2, const vec4& v3)
{
  // NOTE: Status & 0x0080 is the signed sticky flag; should be set if the vsub op results in any negative components
  log_trace("{}({}, {}, {})", __func__, v1, v2, v3);

  return !(glm::any(glm::lessThan(v2.xyz - v1.xyz, { 0, 0, 0 })) || glm::any(glm::lessThan(v1.xyz - v3.xyz, { 0, 0, 0 })));
}

// 0012F290
bool mgClipBox(const vec4& v1, const vec4& v2, const vec4& v3, const vec4& v4)
{
  log_trace("{}({}, {}, {}, {})", __func__, v1, v2, v3, v4);

  return !(glm::any(glm::lessThan(v1.xyz - v4.xyz, { 0, 0, 0 })) || glm::any(glm::lessThan(v3.xyz - v2.xyz, { 0, 0, 0 })));
}

// 0012F2E0
bool mgClipBoxW(const vec4& v1, const vec4& v2, const vec4& v3, const vec4& v4)
{
  log_trace("{}({}, {}, {}, {})", __func__, v1, v2, v3, v4);

  return !(glm::any(glm::lessThan(v1.xyw - v4.xyw, { 0, 0, 0 })) || glm::any(glm::lessThan(v3.xyw - v2.xyw, { 0, 0, 0 })));
}

// 0012F330
bool mgClipInBox(const vec4& v1, const vec4& v2, const vec4& v3, const vec4& v4)
{
  log_trace("{}({}, {}, {}, {})", __func__, v1, v2, v3, v4);

  return !(glm::any(glm::lessThan(v3.xyz - v1.xyz, { 0, 0, 0 })) || glm::any(glm::lessThan(v2.xyz - v4.xyz, { 0, 0, 0 })));
}

// 0012F380
bool mgClipInBoxW(const vec4& v1, const vec4& v2, const vec4& v3, const vec4& v4)
{
  log_trace("{}({}, {}, {}, {})", __func__, v1, v2, v3, v4);

  return !(glm::any(glm::lessThan(v3.xyw - v1.xyw, { 0, 0, 0 })) || glm::any(glm::lessThan(v2.xyw - v4.xyw, { 0, 0, 0 })));
}

// 0012F410
vec4 mgNormalizeVector(const vec4& v, f32 scale)
{
  log_trace("{}({}, {})", __func__, fmt::ptr(&v), scale);

  return glm::normalize(v) * scale;
}

// 0012F540
void mgBoxMaxMin(mgVu0FBOX& lhs, const mgVu0FBOX& rhs)
{
  log_trace("{}({}, {})", __func__, fmt::ptr(&lhs), fmt::ptr(&rhs));

  lhs.corners[0] = glm::max(lhs.corners[0], glm::max(lhs.corners[1], glm::max(rhs.corners[0], rhs.corners[1])));
  lhs.corners[0] = glm::min(lhs.corners[0], glm::min(lhs.corners[1], glm::min(rhs.corners[0], rhs.corners[1])));
}

// 0012F580
vec3 mgPlaneNormal(const vec3& v1, const vec3& v2, const vec3& v3)
{
  log_trace("{}({}, {}, {})", __func__, v1, v2, v3);

  auto temp1 = v2 - v1;
  auto temp2 = v3 - v1;
  
  return common::vector_outer_product(temp1, temp2);
}

// 0012F5B0
f32 mgDistPlanePoint(const vec3& v1, const vec3& v2, const vec3& v3)
{
  log_trace("{}({}, {})", __func__, v1, v2);

  return glm::dot(v1, v3 - v2);
}

// 0012F5F0
f32 mgDistLinePoint(const vec3& v1, const vec3& v2, const vec3& v3, vec3& v4_dest)
{
  log_trace("{}({}, {}, {}, {})", __func__, v1, v2, v3, fmt::ptr(&v4_dest));

  auto var_40 = v2 - v1;
  auto var_30 = v3 - v1;
  auto var_10 = var_30 - var_40;
  f32 f20 = mgDistVector(var_10);
  f20 *= f20;
  f32 f0 = -glm::dot(var_40, var_10);
  f32 f12 = f0 / f20;

  if (f12 >= 0.0f && f12 <= 1.0f)
  {
    auto var_20 = var_10 * f12;
    var_20 += var_40;
    v4_dest = var_20 + v1;
    return mgDistVector(var_20);
  }

  f20 = mgDistVector(v1, v2);
  f32 f21 = mgDistVector(v1, v3);

  if (f20 < f21)
  {
    v4_dest = v2;
    return f20;
  }
  else
  {
    v4_dest = v3;
    return f21;
  }
}

// 0012F760
f32 mgReflectionPlane(const vec3& v1, const vec3& v2, const vec3& v3, vec3& v4_dest)
{
  log_trace("{}({}, {}, {}, {})", __func__, v1, v2, v3, fmt::ptr(&v4_dest));

  f32 result = mgDistPlanePoint(v1, v2, v3) * 2.0f;
  v4_dest = v2 - v3 - (v1 * -result);
  return result;
}

// 0012F7F0
usize mgIntersectionSphereLine0(const vec4& start, const vec4& end, vec4* intersections, f32 radius)
{
  log_trace("{}({}, {}, {}, {})", __func__, start, end, fmt::ptr(intersections), radius);

  // 0012F7F0 - 0012F828
  auto distance = end - start;
  // 0012F828 - 0012F834
  float f20 = mgDistVector2(distance);
  // 0012F834 - 0012F844
  float f21 = glm::dot(vec3{ distance }, vec3{ start });
  // 0012F844 - 0012F854
  float f0 = mgDistVector2(start) - (radius * radius);

  // 0012F854 - 0012F85C
  // NOTE: another way of thinking about these 2 instructions:
  // ACC = f21 * f21
  // f22 = ACC - (f20 * f0)
  float f22 = (f21 * f21) - (f20 * f0);

  // 0012F864 - 0012F87C
  if (f22 < 0.0f)
  {
    return 0;
  }

  // 0012F888 - 0012F88C
  usize n_intersections = 0;

  // 0012F87C - 0012F884 (f22 is moved into f12 before previous branch)
  f0 = sqrtf(f22);
  // 0012F884 - 0012F888
  float f2 = -f21;
  // 0012F88C - 0012F894
  float f12 = (f2 - f0) / f20;
  // 0012F894 - 0012F8A0
  f20 = (f0 + f2) / f20;

  // 0012F8A0 - 0012F8D0
  vec4 intersection_dist;
  if (f12 >= 0.0f && f12 <= 1.0f)
  {
    // 0012F8D0 - 0012F8D8
    intersection_dist = distance * f12;
    // 0012F8D8 - 0012F8E8
    intersections[0] = start + intersection_dist;
    // 0012F8E8 - 0012F8EC
    ++n_intersections;
  }

  // 0012F8EC - 0012F904
  if (f22 == 0.0f)
  {
    return 1;
  }

  // 0012F90C - 0012F938
  if (f20 < 0.0f || f20 > 1.0f)
  {
    return n_intersections;
  }

  // 0012F938 - 0012F944
  intersection_dist = distance * f20;
  // 0012F948 - 0012F958
  intersections[n_intersections] = start + intersection_dist;
  // 0012F958 - 0012F95C
  ++n_intersections;

  // 0012F95C -
  return n_intersections;
}

// 0012F990
usize mgIntersectionSphereLine(const vec4& sphere, const vec4& start, const vec4& end, vec4* intersections)
{
  log_trace("{}({}, {}, {})", __func__, start, end, fmt::ptr(intersections));

  float radius = sphere.w;

  auto start_normal = start - sphere;
  auto end_normal = end - sphere;

  usize n_intersections = mgIntersectionSphereLine0(start_normal, end_normal, intersections, radius);

  for (int i = 0; i < n_intersections; ++i)
  {
    intersections[i] += sphere;
  }
  return n_intersections;
}

// 0012FA50
bool mgIntersectionPoint_line_poly3(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, const vec3& v5, const vec3& v6, vec3& v7_dest)
{
  log_trace("{}({}, {}, {}, {}, {}, {}, {})", __func__, v1, v2, v3, v4, v5, v6, v7_dest);

  auto var_40 = v2 - v1;
  auto var_30 = v3 - v1;
  auto var_20 = v4 - v1;
  auto var_10 = v5 - v1;
  f32 f1 = glm::dot(v5, var_30);
  f32 f0 = glm::dot(v5, var_40);

  if (f0 == 0.0f)
  {
    return false;
  }

  v7_dest = var_40 * (f1 / f0) * 2.0f;
  return mgCheckPointPoly3_XYZ(v7_dest, v3, v4, v5, v6);
}

// 0012FB70
bool mgCheckPointPoly3_XYZ(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, const vec3& v5)
{
  log_trace("{}({}, {}, {}, {}, {})", __func__, v1, v2, v3, v4, v5);

  using namespace common;
  auto var_90 = v1 - v2;
  auto var_80 = v1 - v3;
  auto var_70 = v1 - v4;
  auto var_60 = v3 - v2;
  auto var_50 = v4 - v3;
  auto var_40 = v2 - v4;
  auto var_30 = vector_outer_product(var_60, var_90);
  auto var_20 = vector_outer_product(var_50, var_80);
  auto var_10 = vector_outer_product(var_40, var_70);

  f32 f20 = glm::dot(var_30, v5);
  f32 f21 = glm::dot(var_20, v5);
  f32 f0 = glm::dot(var_10, v5);

  return 
    (f20 >= 0.0f && f21 >= 0.0f && f0 >= 0.0f) ||
    (f20 <= 0.0f && f21 <= 0.0f && f0 <= 0.0f);
}

// 0012FD10
uint mgCheckPointPoly3_XZ(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4)
{
  log_trace("{}({}, {}, {}, {})", __func__, v1, v2, v3, v4);

  return Check_Point_Poly3(v1.x, v1.z, v2.x, v2.z, v3.x, v3.z, v4.x, v4.z);
}

// 0012FD40
uint Check_Point_Poly3(f32 aa, f32 ab, f32 ba, f32 bb, f32 ca, f32 cb, f32 da, f32 db)
{
  log_trace("{}({}, {}, {}, {}, {}, {}, {}, {})", __func__, aa, ab, ba, ab, ca, cb, da, db);

  if (aa < std::min({ ba, ca, da }))
  {
    return 0;
  }

  if (aa > std::max({ ba, ca, da }))
  {
    return 0;
  }

  if (ab < std::min({ bb, cb, db }))
  {
    return 0;
  }

  if (ab > std::max({ ba, ca, da }))
  {
    return 0;
  }

  f32 f6 = (ca - ba) * (ab - bb) - (cb - bb) * (aa - ba);
  f32 f4 = (da - ca) * (ab - cb) - (db - cb) * (aa - ca);
  f32 f1 = (ba - da) * (ab - db) - (bb - db) * (aa - da);

  if (f6 == 0.0f)
  {
    return 2;
  }

  if (f4 == 0.0f)
  {
    return 3;
  }

  if (f1 == 0.0f)
  {
    return 4;
  }

  if (
    (f6 > 0.0f && f4 > 0.0f && f1 > 0.0f) ||
    (f6 < 0.0f && f4 < 0.0f && f1 < 0.0f)
    )
  {
    return 1;
  }
  return 0;
}

// 00130060
f32 mgDistVector(const vec3& v, const vec3& other)
{
  log_trace("{}({})", __func__, v, other);

  return glm::distance(v, other);
}

// 001300A0
f32 mgDistVectorXZ(const vec3& v, const vec3& other)
{
  log_trace("{}({})", __func__, v, other);

  return glm::distance(vec2{ v.xz }, vec2{ other.xz });
}

// 001300E0
f32 mgDistVector2(const vec3& v, const vec3& other)
{
  log_trace("{}({})", __func__, v, other);

  auto temp = v - other;
  temp *= temp;
  return temp.x + temp.y + temp.z;
}

// 00130110
f32 mgDistVectorXZ2(const vec3& v, const vec3& other)
{
  log_trace("{}({})", __func__, v, other);

  auto temp = v - other;
  temp *= temp;
  return temp.x + temp.z;
}

// 00130140
matrix4 mgUnitMatrix()
{
  log_trace("{}()", __func__);

  return matrix4{ 1.0f };
}

// 00130160
matrix4 mgZeroMatrix()
{
  log_trace("{}()", __func__);

  return matrix4{ 0.0f };
}

// 00130180
matrix4 MulMatrix3(const matrix4& m1, const matrix4& m2, const matrix4& m3)
{
  log_trace("{}({}, {}, {})", __func__, fmt::ptr(&m1), fmt::ptr(&m2), fmt::ptr(&m3));

  matrix4 temp;
  temp[0] = m1 * m2[0];
  temp[1] = m1 * m2[1];
  temp[2] = m1 * m2[2];
  temp[3] = m1 * m2[3];
  matrix4 result;
  result[0] = temp * m3[0];
  result[1] = temp * m3[1];
  result[2] = temp * m3[2];
  result[3] = temp * m3[3];
  return result;
}

// 00130250
matrix4 mgMulMatrix(const matrix4& lhs, const matrix4& rhs)
{
  log_trace("{}({}, {})", __func__, fmt::ptr(&lhs), fmt::ptr(&rhs));

  matrix4 result;
  result[0] = lhs * rhs[0];
  result[1] = lhs * rhs[1];
  result[2] = lhs * rhs[2];
  result[3] = lhs * rhs[3];
  return result;
}

// 001302D0
matrix4 mgInverseMatrix(const matrix4& mat)
{
  log_trace("{}({})", __func__, fmt::ptr(&mat));

  todo;
  return matrix4{ 1.0f };
}

// 001303D0
matrix4 mgRotMatrixX(f32 rotation)
{
  log_trace("{}({})", __func__, rotation);

  matrix4 mat = mgUnitMatrix();

  f32 f = cosf(rotation);
  mat[2].z = f;
  mat[1].y = f;

  f = sinf(rotation);
  mat[1].z = f;
  mat[2].y = f;
  return mat;
}

// 00130430
matrix4 mgRotMatrixY(f32 rotation)
{
  log_trace("{}({})", __func__, rotation);

  matrix4 mat = mgUnitMatrix();

  f32 f = cosf(rotation);
  mat[2].z = f;
  mat[0].x = f;

  f = sinf(rotation);
  mat[0].z = f;
  mat[2].x = f;
  return mat;
}

// 00130490
matrix4 mgRotMatrixZ(f32 rotation)
{
  log_trace("{}({})", __func__, rotation);

  matrix4 mat = mgUnitMatrix();

  f32 f = cosf(rotation);
  mat[1].y = f;
  mat[0].x = f;

  f = sinf(rotation);
  mat[0].y = f;
  mat[1].x = f;
  return mat;
}

// 001304F0
matrix4 mgRotMatrixXYZ(const vec3& rotation)
{
  log_trace("{}({})", __func__, rotation);

  matrix4 rotX = mgRotMatrixX(rotation.x);
  matrix4 rotY = mgRotMatrixY(rotation.y);
  matrix4 rotZ = mgRotMatrixZ(rotation.z);

  MulMatrix3(rotZ, rotY, rotX);
  return rotZ;
}

// 00130550
matrix4 mgCreateMatrixPY(const vec4& v, f32 f)
{
  log_trace("{}({}, {})", __func__, v, f);

  matrix4 result = mgUnitMatrix();
  result = glm::rotate(result, f, { 0, 1, 0 });

  result[3] = v;
  result[3].w = 1.0f;
  return result;
}

// 001305B0
matrix4 mgLookAtMatrixZ(const vec4& v)
{
  log_trace("{}({})", __func__, fmt::ptr(&v));

  todo;
  return matrix4{ 1.0f };
}

// 00130690
matrix4 mgShadowMatrix(const vec4& v1, const vec4& v2, const vec4& v3)
{
  log_trace("{}({}, {}, {})", __func__, fmt::ptr(&v1), fmt::ptr(&v2), fmt::ptr(&v3));

  todo;
  return matrix4{ 1.0f };
}

// 001308A0
void mgApplyMatrixN(vec4* vecs_dest, const matrix4& mat, const vec4* vecs, usize n)
{
  log_trace("{}({}, {}, {}, {})", __func__, fmt::ptr(vecs_dest), fmt::ptr(&mat), fmt::ptr(vecs), n);

  todo;
}

// 001308F0
void mgApplyMatrixN_MaxMin(vec4* vecs_dest, const matrix4& mat, const vec4* vecs, usize n, vec4& max_dest, vec4& min_dest)
{
  log_trace("{}({}, {}, {}, {}, {})", __func__, fmt::ptr(vecs_dest), fmt::ptr(&mat), fmt::ptr(vecs), n, fmt::ptr(&max_dest), fmt::ptr(&min_dest));

  todo;
}

// 00130980
void mgVectorMinMaxN(vec4* max_dest, vec4* min_dest, vec4* vecs, usize n)
{
  log_trace("{}({}, {}, {}, {})", __func__, fmt::ptr(max_dest), fmt::ptr(min_dest), fmt::ptr(vecs), n);

  todo;
}

// 001309E0
void mgApplyMatrix(vec4& max_dest, vec4& min_dest, const matrix4& mat, const vec4& c1, const vec4& c2)
{
  log_trace("{}({}, {}, {}, {}, {})", __func__, fmt::ptr(&max_dest), fmt::ptr(&min_dest), fmt::ptr(&mat), fmt::ptr(&c1), fmt::ptr(&c2));

  todo;
}

// 00130A50
vec4 mgVectorInterpolate(const vec4& lhs, const vec4& rhs, f32 t, bool b)
{
  log_trace("{}({}, {}, {}, {})", __func__, fmt::ptr(&lhs), fmt::ptr(&rhs), t, b);

  todo;
  return vec4{ 0, 0, 0, 1 };
}

// 00130B60
f32 mgAngleInterpolate(f32 f12, f32 f13, f32 f14, bool b)
{
  log_trace("{}({}, {}, {}, {})", __func__, f12, f13, f14, b);

  f32 f1 = mgAngleClamp(f13 - f12);

  if (!b)
  {
    if (fabsf(f1) < f14)
    {
      return f13;
    }
  }

  f32 f2 = 0.0f;

  if (b)
  {
    f2 = f1 / f14;
  }
  else
  {
    if (f1 < 0.0f)
    {
      if (f14 < f1)
      {
        return f13;
      }
      f2 -= f14;
    }

    if (f1 > 0.0f)
    {
      if (f14 > f1)
      {
        return f13;
      }
      f2 += f14;
    }
  }

  return mgAngleClamp(f12 + f2);
}

// 00130D10
s32 mgAngleCmp(f32 f12, f32 f13, f32 f14)
{
  log_trace("{}({}, {}, {})", __func__, f12, f13, f14);

  f32 n = f12 - f13;
  if (n == 0.0f)
  {
    return 0;
  }

  n = mgAngleClamp(n);

  if (n > f14)
  {
    return 1;
  }
  if (n < -f14)
  {
    return -1;
  }
  return 0;
}

// 00130DD0
f32 mgAngleLimit(f32 f)
{
  log_trace("{}({})", __func__, f);

  if (common::negate(common::pi()) <= f && f < common::pi())
  {
    return f;
  }

  f -= truncf(f / common::pi2()) * common::pi2();
  return mgAngleClamp(f);
}

// 00130EE0
f32 mgRnd()
{
  log_trace("{}()", __func__);

  return rand() / static_cast<f32>(std::numeric_limits<s32>::max());
}

// 00130F20
// for when you want a rand that returns [-6, 6] but really biases towards 0
// for whatever reason
f32 mgNRnd()
{
  f32 f = 0;
  for (int i = 0; i < 12; ++i)
  {
    f += mgRnd();
  }
  return f - 6.0f;
}

// 00130FB0
void mgCreateSinTable()
{
  log_trace("{}()", __func__);

  f32 sin_table_num = static_cast<f32>(SinTable.size());

  for (int i = 0; i < SinTable.size(); ++i)
  {
    SinTable[i] = sinf(static_cast<f32>(i) * common::pi2() / sin_table_num);
  }
}

// 00131050
f32 mgSinf(f32 f)
{
  log_trace("{}({})", __func__, f);

  f32 sign = copysign(1, f);

  ssize index = static_cast<ssize>(fabsf(f) * sin_table_unit_1);
  index = abs(index) % SinTable.size();
  return SinTable[index] * sign;
}

// 001310F0
f32 mgCosf(f32 f)
{
  log_trace("{}({})", __func__, f);

  return mgSinf(f + common::deg_to_rad(90.0f));
}