#pragma once
#include "common/types.h"

namespace common
{
  template<typename type>
  class rectangle
  {
  public:
    using storage_type = vec<type, 4>;
    using point_type = vec<type, 2>;

    rectangle()
      : m_data{ 0, 0, 0, 0 }
    {
    }

    // construct from a vector type
    explicit rectangle(storage_type v)
      : m_data{ v }
    {
    }

    // consturct by values
    explicit rectangle(type x0, type y0, type x1, type y1)
      : m_data{ x0, y0, x1, y1 }
    {
    }

    // construct a rectangle from two points xy0 and xy1
    // assumes top left origin (meaning x0 < x1 and y0 < y1)
    static constexpr rectangle<type> from_points(type x0, type y0, type x1, type y1)
    {
      return rectangle<type>{ x0, y0, x1, y1 };
    }

    // construct from a single point and an extent
    static constexpr rectangle<type> from_extent(type x, type y, type width, type height)
    {
      return rectangle<type>{ x, y, x + width, y + height };
    }

    // construct from an extent assuming top left is 0,0
    static constexpr rectangle<type> from_extent(type width, type height)
    {
      return rectangle<type>{ 0, 0, width, height };
    }

    // get the width of the rectangle
    constexpr type width() const
    {
      return m_data.z - m_data.x;
    }

    // get the height of the rectangle
    constexpr type height() const
    {
      return m_data.w - m_data.y;
    }

    // get the extent (width, height)
    constexpr point_type extent()
    {
      return { width(), height() };
    }

    // get the top left origin point (x, y)
    constexpr point_type origin()
    {
      return { m_data.x, m_data.y };
    }

  private:
    // vector storage
    storage_type m_data{ 0, 0, 0, 0 };
  };
}

using irect = common::rectangle<sint>;
using urect = common::rectangle<uint>;
using rect  = common::rectangle<f32>;