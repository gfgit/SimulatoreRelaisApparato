#ifndef CABLETYPES_H
#define CABLETYPES_H

#include "circuittypes.h"

#include <QVector>

class AbstractCircuitNode;
class CircuitCable;

enum class CableSide
{
    A = 0,
    B = 1
};

enum class ContactType
{
    NotConnected = 0,
    Connected,
    Passthrough
};

enum class CablePowerPole
{
    None = 0,
    First = 1,
    Second = 2,
    Both = 3,
};

enum class CablePower
{
    None = 0
};

struct CableEnd
{
    AbstractCircuitNode *node = nullptr;
    int nodeContact = 0;
};

struct CableContact
{
    CircuitCable *cable = nullptr;
    CableSide side = CableSide::A;
    CircuitPole pole = CircuitPole::First;
};

struct CableItem
{
    CableContact cable;
    int nodeContact = 0;
};

struct NodeItem
{
    enum { InvalidContact  = -1 };
    AbstractCircuitNode *node = nullptr;
    int fromContact = InvalidContact;
    int toContact   = InvalidContact;
    CircuitPole fromPole = CircuitPole::First;
    CircuitPole toPole   = CircuitPole::First;
};

typedef QVector<NodeItem> NodeOccurences;

inline CableSide operator ~(CableSide s)
{
    return s == CableSide::A ? CableSide::B :
                               CableSide::A;
}

constexpr CircuitPole operator ~(CircuitPole value)
{
    if(value == CircuitPole::First)
        return CircuitPole::Second;
    return CircuitPole::First;
}

constexpr CablePower operator |(CablePowerPole power, CircuitType type)
{
    return static_cast<CablePower>(static_cast<int>(power) | (static_cast<int>(type) << 7));
}

constexpr CablePowerPole toCablePowerPole(CablePower power)
{
    return static_cast<CablePowerPole>(static_cast<int>(power) & 0x7F);
}

constexpr CircuitType toCircuitType(CablePower power)
{
    return static_cast<CircuitType>(static_cast<int>(power) >> 7);
}

constexpr bool operator ==(const CableContact& lhs, const CableContact& rhs)
{
    return lhs.cable == rhs.cable &&
            lhs.side == rhs.side &&
            lhs.pole == rhs.pole;
}

constexpr bool operator ==(const NodeItem& lhs, const NodeItem& rhs)
{
    return lhs.node == rhs.node &&
            lhs.fromContact == rhs.fromContact &&
            lhs.fromPole == rhs.fromPole &&
            lhs.toContact == rhs.toContact &&
            lhs.toPole == rhs.toPole;
}

#endif // CABLETYPES_H
