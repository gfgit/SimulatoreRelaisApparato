#ifndef CABLETYPES_H
#define CABLETYPES_H

#include "circuittypes.h"

class AbstractCircuitNode;
class CircuitCable;

enum class CableSide
{
    A = 0,
    B = 1
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

enum class ContactType
{
    NotConnected = 0,
    Connected,
    Passthrough
};

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

#endif // CABLETYPES_H
