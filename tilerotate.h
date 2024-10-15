#ifndef TILEROTATE_H
#define TILEROTATE_H

#include <cstdint>
#include <cmath>
#include <type_traits>
#include <limits>
#include <functional>

#include <QPointF>

/*
 * TileRotate
 */

enum class TileRotate : uint8_t // 2 bit
{
    Deg0 = 0,   // South
    Deg90 = 1,  // West
    Deg180 = 2, // Nord
    Deg270 = 3, // East
};

inline constexpr std::underlying_type_t<TileRotate> toRotateInt(TileRotate r)
{
    return static_cast<std::underlying_type_t<TileRotate>>(r);
}

constexpr TileRotate operator +(TileRotate lhs, TileRotate rhs)
{
    return static_cast<TileRotate>((toRotateInt(lhs) + toRotateInt(rhs)) % 4);
}

constexpr TileRotate& operator +=(TileRotate& lhs, TileRotate rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

constexpr TileRotate operator -(TileRotate lhs, TileRotate rhs)
{
    return static_cast<TileRotate>(((toRotateInt(lhs) + 4 - (toRotateInt(rhs)) % 4)) % 4);
}

constexpr TileRotate& operator -=(TileRotate& lhs, TileRotate rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline TileRotate diff(TileRotate a, TileRotate b)
{
    return static_cast<TileRotate>(std::abs(toRotateInt(a) - toRotateInt(b)));
}

constexpr uint16_t toDeg(TileRotate value)
{
    return static_cast<uint16_t>(toRotateInt(value)) * 90;
}

constexpr TileRotate fromDeg(uint16_t value)
{
    return static_cast<TileRotate>((value / 90) % 4);
}


/*
 * TileLocation
 */

struct TileLocation
{
    static const TileLocation invalid;

    static constexpr double Size = 100.0;

    int16_t x;
    int16_t y;

    bool isValid() const
    {
        return *this != invalid;
    }

    bool operator ==(const TileLocation& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator !=(const TileLocation& other) const
    {
        return x != other.x || y != other.y;
    }

    TileLocation adjusted(int16_t dx, int16_t dy)
    {
        return {static_cast<int16_t>(x + dx), static_cast<int16_t>(y + dy)};
    }

    static TileLocation fromPoint(const QPointF& p)
    {
        return {static_cast<int16_t>(std::round(p.x() / Size)),
                static_cast<int16_t>(std::round(p.y() / Size))};
    }

    QPointF toPoint() const
    {
        return QPointF(double(x) * Size,
                       double(y) * Size);
    }
};

inline const TileLocation TileLocation::invalid{std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::min()};

struct TileLocationHash
{
    std::size_t operator()(const TileLocation& key) const
    {
        return std::hash<int16_t>()(key.x) ^ std::hash<int16_t>()(key.y);
    }
};


/*
 * Connector
 */

struct Connector
{
    enum class Direction : uint8_t
    {
        North = 1,
        East = 2,
        South = 3,
        West = 4,
    };

    TileLocation location;
    Direction direction;

    Connector(TileLocation location_, Direction direction_);

    Connector(TileLocation location_, TileRotate rotate);

    bool operator ==(const Connector& other) const
    {
        return location == other.location &&
               direction == other.direction;
    }

    Connector opposite() const;
};

constexpr Connector::Direction operator ~(Connector::Direction value)
{
    const auto n = static_cast<std::underlying_type_t<Connector::Direction>>(value);
    return static_cast<Connector::Direction>(n <= 2 ? n + 2 : n - 2);
}

constexpr Connector::Direction toConnectorDirection(TileRotate value)
{
    const auto r = toRotateInt(value);
    return static_cast<Connector::Direction>(r < 2 ? r + 3 : r - 1);
}

constexpr TileLocation operator +(TileLocation location, Connector::Direction direction)
{
    switch(direction)
    {
    case Connector::Direction::North:
        return {location.x, static_cast<int16_t>(location.y - 1)};

    case Connector::Direction::East:
        return {static_cast<int16_t>(location.x + 1), location.y};

    case Connector::Direction::South:
        return {location.x, static_cast<int16_t>(location.y + 1)};

    case Connector::Direction::West:
        return {static_cast<int16_t>(location.x - 1), location.y};

    }
    return location;
}

#endif // TILEROTATE_H