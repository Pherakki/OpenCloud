#pragma once
#include <type_traits>

#include "common/types.h"

namespace common::bits
{
  // returns true if value is aligned to align_to
  template<typename type>
  constexpr bool aligned(type value, uint align_to)
  {
    return (value % static_cast<type>(align_to)) == 0;
  }

  // aligns the value up to align_to
  template<typename type>
  constexpr type align_up(type value, uint align_to)
  {
    const type align = static_cast<type>(align_to);

    return (value + align - 1) / align * align;
  }

  // aligns the value down to align_to
  template<typename type>
  constexpr type align_down(type value, uint align_to)
  {
    const type align = static_cast<type>(align_to);

    return value / align * align;
  }

  template<typename to_type, typename from_type>
  constexpr to_type zext(from_type value)
  {
    static_assert(sizeof(to_type) > sizeof(from_type), "Cannot extend to smaller type");

    using to_unsigned_type = std::make_unsigned_t<to_type>;
    using from_unsigned_type = std::make_unsigned_t<from_type>;

    // cast the parameter value to it's unsigned version
    // ex: s8 -> u8
    const auto a = static_cast<from_unsigned_type>(value);

    // cast the result to the unsigned version of the return type
    // ex: s32 -> u32
    const auto b = static_cast<to_unsigned_type>(a);

    // cast the result to the return type
    const auto c = static_cast<to_type>(b);

    return c;
  }

  // sign extend a value
  template<typename to_type, typename from_type>
  constexpr to_type sext(from_type value)
  {
    static_assert(sizeof(to_type) > sizeof(from_type), "Cannot extend to smaller type");

    using to_signed_type = std::make_signed_t<to_type>;
    using from_unsigned_type = std::make_signed_t<from_type>;

    // cast the parameter to it's signed version
    // ex: u8 -> s8
    const auto a = static_cast<from_unsigned_type>(value);

    // cast the result to the signed version of the return type
    // ex: u32 -> s32
    const auto b = static_cast<to_signed_type>(a);

    // cast the result to the return type
    const auto c = static_cast<to_type>(b);

    return c;
  }

  // truncate a value
  template<typename to_type, typename from_type>
  constexpr to_type trunc(from_type value)
  {
    static_assert(sizeof(to_type) < sizeof(from_type), "Cannot truncate to larger type");

    // unsigned type of the return value
    using to_unsigned_type = std::make_unsigned_t<to_type>;

    // cast the parameter to the unsigned version of the return type
    // ex: s32 -> u32
    const auto a = static_cast<to_unsigned_type>(value);

    // cast the result to the return type
    const auto b = static_cast<to_type>(a);

    return b;
  }

  // zero extend to 16 bits
  template<typename type>
  constexpr u16 zext16(type val)
  {
    return zext<u16, type>(val);
  }

  // zero extend to 32 bits
  template<typename type>
  constexpr u32 zext32(type val)
  {
    return zext<u32, type>(val);
  }

  // zero extend to 64 bits
  template<typename type>
  constexpr u64 zext64(type val)
  {
    return zext<u64, type>(val);
  }

  // zero extend to 16 bits
  template<typename type>
  constexpr u16 sext16(type val)
  {
    return sext<u16, type>(val);
  }

  // zero extend to 32 bits
  template<typename type>
  constexpr u32 sext32(type val)
  {
    return sext<u32, type>(val);
  }

  // zero extend to 64 bits
  template<typename type>
  constexpr u64 sext64(type val)
  {
    return sext<u64, type>(val);
  }

  // cast to bool
  template<typename type>
  constexpr bool to_bool(type val)
  {
    return static_cast<bool>(val);
  }

  // cast away from bool
  template<typename type>
  constexpr type from_bool(bool val)
  {
    return static_cast<type>(val);
  }

  // 8 bit bool
  constexpr b8 bool8(bool val)
  {
    return from_bool<b8>(val);
  }

  // 16 bit bool
  constexpr b16 bool16(bool val)
  {
    return from_bool<b16>(val);
  }

  // 32 bit bool
  constexpr b32 bool32(bool val)
  {
    return from_bool<b32>(val);
  }

  // 64 bit bool
  constexpr b64 bool64(bool val)
  {
    return from_bool<b64>(val);
  }

  // return the index of the lsb
  // undefined behavior if val is 0
  template<typename type>
  inline u64 lsb_index(type val)
  {
    ulong index;
    _BitScanForward64(&index, val);

    return index;
  }

  // return the index of the msb
  // undefined behavior if val is 0
  template<typename type>
  inline u64 msb_index(type val)
  {
    ulong index;
    _BitScanReverse64(&index, val);

    return index;
  }

  // find the lsb
  // undefined behavior if val is 0
  template<typename type>
  inline type lsb(type val)
  {
    return 1 << lsb_index(val);
  }

  // find the msb
  // undefined behavior if val is 0
  template<typename type>
  inline type msb(type val)
  {
    return 1 << msb_index(val);
  }
}
