#include "tilerotate.h"

Connector::Connector(TileLocation location_, Direction direction_, int contact)
    : location{location_}
    , direction{direction_}
    , nodeContact(contact)
{
}

Connector::Connector(TileLocation location_, TileRotate rotate, int contact)
    : location{location_}
    , direction{toConnectorDirection(rotate)}
    , nodeContact(contact)
{
}
