#pragma once
#include "common/debug.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// ~ 00131110 - 00131B60

class mgCCamera
{
private:
  // 40
  f32 m_roll;

  // 44
  unk32 m_unk_field_44;

  // 58
  f32 m_step_epsilon;

protected:
  // 0
  vec3 m_position;

  // 10
  vec3 m_reference;

  // 20
  vec3 m_next_position;

  // 30
  vec3 m_next_reference;

  // 48
  f32 m_position_speed;

  // 4C
  f32 m_rotation_speed;

  // 50
  f32 m_angleH;

  // 54
  f32 m_angleV;

  // 5C
  bool m_stopped;

public:
  // 00131630
  // speed is how long it takes the camera to reach its target, in frames (30fps).
  mgCCamera(f32 speed = 8.0f);

  // 001CE150
  // should be handled by compiler
  //mgCCamera(mgCCamera& other);
  // 00131110
  virtual void Step(sint steps = 1);

  // 00131B30
  virtual void Suspend();

  // 00131B40
  virtual void Resume();

  // 001313A0
  virtual void Stay();

  // 001314D0
  virtual matrix4 GetCameraMatrix() const;

  // 00131B50
  virtual sint Iam() const;

  // 001313E0
  inline void SetPos(f32 x, f32 y, f32 z)
  {
    SetPos(vec3(x, y, z));
  }

  // 00131410
  inline void SetPos(const vec3& pos)
  {
    m_position = pos;
  }

  // 00131420
  inline void SetNextPos(f32 x, f32 y, f32 z)
  {
    SetNextPos(vec3(x, y, z));
  }

  // 00131430
  inline void SetNextPos(const vec3& next_pos)
  {
    m_next_position = next_pos;
  }

  // 00131440
  inline void SetRef(f32 h, f32 p, f32 r)
  {
    SetRef(vec3(h, p, r));
  }

  // 00131460
  inline void SetRef(const vec3& ref)
  {
    m_reference = ref;
  }

  // 00131470
  inline void SetNextRef(f32 h, f32 p, f32 r)
  {
    SetNextRef(vec3(h, p, r));
  }

  // 00131480
  inline void SetNextRef(const vec3& next_ref)
  {
    m_next_reference = next_ref;
  }

  // 00131490
  inline vec3 GetDir() const
  {
    return m_reference - m_position;
  }

  // 001315C0
  inline void SetRoll(f32 roll)
  {
    m_roll = roll;
  }

  // 001315D0
  inline vec3 GetPos() const
  {
    return m_position;
  }

  // 001315E0
  inline vec3 GetRef() const
  {
    return m_reference;
  }

  // 001315F0
  inline vec3 GetNextPos() const
  {
    return m_next_position;
  }

  // 00131600
  inline vec3 GetNextRef() const
  {
    return m_next_reference;
  }

  // 00131610
  inline f32 GetAngleH() const
  {
    return m_angleH;
  }

  // 00131620
  inline f32 GetAngleV() const
  {
    return m_angleV;
  }

  // 00376C00
  static bool StopCamera;
};

class mgCCameraFollow : public mgCCamera
{
protected:
  // 70
  vec3 m_follow;

  // 80
  vec3 m_follow_offset;

  // 90
  f32 m_distance;

  // 94
  f32 m_height;

  // 98
  f32 m_angle_soon;

  // 9C
  f32 m_angle;

  // A0
  bool m_following;

  // B0
  vec3 m_follow_next;

public:
  // 00131A90
  mgCCameraFollow(
    f32 follow_distance,
    f32 height,
    f32 angle,
    f32 speed
  );

  // 00131740
  virtual void Step(sint steps = 1);

  // 00131950
  virtual void Stay();

  // 00131B20
  virtual sint Iam() const;

  // 00131680
  vec3 GetFollowNextPos() const;

  // 00131700
  inline vec3 GetFollowNext() const
  {
    return GetFollow() + m_follow_offset;
  }
  // 00131990
  inline void SetFollow(const vec3& follow_pos)
  {
    m_follow = follow_pos;
  }
  // 001319A0
  inline void FollowOn()
  {
    m_following = true;
  }
  // 001319B0
  inline void FollowOff()
  {
    m_following = false;
  }
  // 001319C0
  inline void SetAngle(f32 angle)
  {
    m_angle_soon = angle;
  }
  // 001319D0
  inline void SetAngleSoon(f32 angle)
  {
    m_angle_soon = angle;
    m_angle = angle;
  }
  // 001319E0
  inline f32 GetAngle() const
  {
    return m_angle;
  }
  // 001319F0
  inline void AddAngle(f32 delta)
  {
    m_angle_soon += delta;
  }
  // 00131A00
  inline void SetDistance(f32 distance)
  {
    m_distance = distance;
  }
  // 00131A10
  inline f32 GetDistance() const
  {
    return m_distance;
  }
  // 00131A20
  inline void AddDistance(f32 delta)
  {
    m_distance += delta;
  }
  // 00131A30
  inline void SetHeight(f32 height)
  {
    m_height = height;
  }
  // 00131A40
  inline f32 GetHeight() const
  {
    return m_height;
  }
  // 00131A50
  inline void AddHeight(f32 delta)
  {
    m_height += delta;
  }
  // 00131A60
  inline void SetFollowOffset(const vec3& offset)
  {
    m_follow_offset = offset;
  }
  // 00131A70
  inline vec3 GetFollow() const
  {
    return m_follow;
  }
  // 00131A80
  inline vec3 GetFollowOffset() const
  {
    return m_follow_offset;
  }
};