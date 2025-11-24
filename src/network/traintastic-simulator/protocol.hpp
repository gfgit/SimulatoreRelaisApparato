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
    Handshake = 5,
    HandshakeResponse = 6,
    SignalSetState = 7,
    OwnSignal = 8,
    RequestChannel = 9,
    OwnSpawn = 10,
    SpawnStateChange = 11
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

struct HandShake : Message
{
    HandShake(bool reply)
        : Message(reply ? OpCode::HandshakeResponse : OpCode::Handshake,
                  sizeof(HandShake))
    {
    }
};
static_assert(sizeof(HandShake) == 2);

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

struct SignalSetState : Message
{
    uint16_t channel;
    uint16_t address;

    enum Color
    {
        Red = 0,
        Yellow,
        Green
    };

    enum State
    {
        Off = 0,
        On,
        Blink,
        BlinkReverse,
    };

    enum RappelState
    {
        Rappel_Off = 0,
        OneLine_60,
        TwoLines_100
    };

    static constexpr uint8_t StateMask = uint8_t(State::BlinkReverse);
    static constexpr uint8_t ArrowLight = 0x04;

    struct LightState
    {
        uint8_t color;
        uint8_t state;
    };

    LightState lights[3];
    float speed = 0.0;
    uint8_t advanceSignalStateAndArrow = 0;
    uint8_t rappelState = RappelState::Rappel_Off;
    char directionIndication = ' ';

    inline State getAdvanceSignalState() const { return State(advanceSignalStateAndArrow & StateMask); }
    inline void setAdvanceSignalState(State s)
    {
        advanceSignalStateAndArrow = (advanceSignalStateAndArrow & ~StateMask) | uint8_t(s);
    }

    inline bool isArrowLightOn() const { return (advanceSignalStateAndArrow & ArrowLight) == ArrowLight; }
    inline void setArrowLightOn(bool on)
    {
        if(on)
            advanceSignalStateAndArrow |= ArrowLight;
        else
            advanceSignalStateAndArrow &= ~ArrowLight;
    }

    SignalSetState(uint16_t ch, uint16_t addr)
        : Message(OpCode::SignalSetState, sizeof(SignalSetState))
        , channel{ch}
        , address{addr}
    {
        lights[0] = {Color::Red, State::Off};
        lights[1] = {Color::Red, State::Off};
        lights[2] = {Color::Red, State::Off};
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(SignalSetState) == 19);

struct OwnSignal : Message
{
    uint16_t channel;
    uint16_t address;

    OwnSignal(uint16_t ch, uint16_t addr)
        : Message(OpCode::OwnSignal, sizeof(OwnSignal))
        , channel{ch}
        , address{addr}
    {
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(OwnSignal) == 6);

struct RequestChannel : Message
{
    uint16_t channel;

    RequestChannel(uint16_t ch)
        : Message(OpCode::RequestChannel, sizeof(RequestChannel))
        , channel{ch}
    {
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(RequestChannel) == 4);

struct OwnSpawn : Message
{
    uint16_t address;

    OwnSpawn(uint16_t addr)
        : Message(OpCode::OwnSpawn, sizeof(OwnSpawn))
        , address{addr}
    {
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(OwnSpawn) == 4);

struct SpawnStateChange : Message
{
    enum State
    {
        Ready = 1,
        WaitingReset = 2,

        RequestActivate = 20,
        Reset = 21
    };

    uint16_t address;
    uint8_t state;


    SpawnStateChange(uint16_t addr, uint8_t s)
        : Message(OpCode::SpawnStateChange, sizeof(SpawnStateChange))
        , address{addr}
        , state{s}
    {
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(SpawnStateChange) == 5);

PRAGMA_PACK_POP

}

#endif
