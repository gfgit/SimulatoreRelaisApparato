/**
 * src/network/traintastic-simulator/protocol.hpp
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_SIMULATOR_PROTOCOL_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_SIMULATOR_PROTOCOL_HPP

#include <cstdint>

#ifdef _MSC_VER
  #define ATTRIBUTE_PACKED
  #define PRAGMA_PACK_PUSH_1 __pragma(pack(push, 1))
  #define PRAGMA_PACK_POP __pragma(pack(pop))
#else
  #define ATTRIBUTE_PACKED __attribute__((packed))
  #define PRAGMA_PACK_PUSH_1
  #define PRAGMA_PACK_POP
#endif

namespace SimulatorProtocol
{

static constexpr uint16_t DefaultPort = 5741;

enum class OpCode : uint8_t
{
  Power = 1,
  LocomotiveSpeedDirection = 2,
  SensorChanged = 3,
  AccessorySetState = 4,
};

struct Message
{
  OpCode opCode;
  uint8_t size;

  Message(OpCode opCode_, uint8_t size_)
    : opCode{opCode_}
    , size{size_}
  {
  }
};
static_assert(sizeof(Message) == 2);

struct Power : Message
{
  uint8_t powerOn;

  Power(bool on)
    : Message(OpCode::Power, sizeof(Power))
    , powerOn(on ? 1 : 0)
  {
  }
};
static_assert(sizeof(Power) == 3);

PRAGMA_PACK_PUSH_1

struct SensorChanged : Message
{
  uint16_t channel;
  uint16_t address;
  uint8_t value;

  SensorChanged(uint16_t ch, uint16_t addr, bool val)
    : Message(OpCode::SensorChanged, sizeof(SensorChanged))
    , channel{ch}
    , address{addr}
    , value(val ? 1 : 0)
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(SensorChanged) == 7);

struct AccessorySetState : Message
{
  uint16_t channel;
  uint16_t address;
  uint8_t state;

  AccessorySetState(uint16_t ch, uint16_t addr, uint8_t st)
    : Message(OpCode::AccessorySetState, sizeof(AccessorySetState))
    , channel{ch}
    , address{addr}
    , state{st}
  {
  }
} ATTRIBUTE_PACKED;
static_assert(sizeof(AccessorySetState) == 7);

PRAGMA_PACK_POP

}

#endif
