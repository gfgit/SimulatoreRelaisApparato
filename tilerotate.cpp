#include "tilerotate.h"

Connector::Connector(TileLocation location_, Direction direction_)
    : location{location_}
    , direction{direction_}
{
}

Connector::Connector(TileLocation location_, TileRotate rotate)
    : location{location_}
    , direction{toConnectorDirection(rotate)}
{
}

Connector Connector::opposite() const
{
    return {location + direction, ~direction};
}
