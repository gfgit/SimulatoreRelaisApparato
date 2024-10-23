#ifndef CIRCUITCABLE_H
#define CIRCUITCABLE_H

#include <QObject>
#include <QVector>

#include "../enums/circuittypes.h"
#include "../enums/cabletypes.h"

class ElectricCircuit;
class AbstractCircuitNode;

class CircuitCable : public QObject
{
    Q_OBJECT
public:

    enum class Mode
    {
        Unifilar = 0,
        Bifilar1,
        Bifilar2,
        BifilarBoth
    };

    explicit CircuitCable(QObject *parent = nullptr);
    ~CircuitCable();

    Mode mode() const;
    void setMode(Mode newMode);

    CablePower powered();

    void addCircuit(ElectricCircuit *circuit, CircuitPole pole);
    void removeCircuit(ElectricCircuit *circuit);

    inline CableEnd getNode(CableSide s) const
    {
        switch (s)
        {
        case CableSide::A:
            return mNodeA;
        case CableSide::B:
            return mNodeB;
        default:
            break;
        }

        Q_UNREACHABLE();
        return {};
    }

signals:
    void modeChanged(Mode m);
    void powerChanged(CablePower p);
    void nodesChanged();

private:
    friend class AbstractCircuitNode;
    void setNode(CableSide s, CableEnd node);

private:
    Mode mMode = Mode::Unifilar;

    typedef QVector<ElectricCircuit *> CircuitList;

    inline CircuitList& getCircuits(CircuitType type, CircuitPole pole)
    {
        if(pole == CircuitPole::First)
            return type == CircuitType::Closed ?
                        mFirstPoleCirctuitsClosed :
                        mFirstPoleCirctuitsOpen;

        return type == CircuitType::Closed ?
                    mSecondPoleCirctuitsClosed :
                    mSecondPoleCirctuitsOpen;
    }

    inline const CircuitList& getCircuits(CircuitType type, CircuitPole pole) const
    {
        if(pole == CircuitPole::First)
            return type == CircuitType::Closed ?
                        mFirstPoleCirctuitsClosed :
                        mFirstPoleCirctuitsOpen;

        return type == CircuitType::Closed ?
                    mSecondPoleCirctuitsClosed :
                    mSecondPoleCirctuitsOpen;
    }

    CircuitList mFirstPoleCirctuitsClosed;
    CircuitList mSecondPoleCirctuitsClosed;
    CircuitList mFirstPoleCirctuitsOpen;
    CircuitList mSecondPoleCirctuitsOpen;

    CableEnd mNodeA;
    CableEnd mNodeB;
};

#endif // CIRCUITCABLE_H
