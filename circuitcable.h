#ifndef CIRCUITCABLE_H
#define CIRCUITCABLE_H

#include <QObject>
#include <QVector>

#include "enums/circuittypes.h"
#include "enums/cabletypes.h"

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

    enum class Power
    {
        None = 0,
        FirstCable = 1,
        SecondCable = 2,
        BothOn = 3
    };

    explicit CircuitCable(QObject *parent = nullptr);
    ~CircuitCable();

    Mode mode() const;
    void setMode(Mode newMode);

    Power powered();

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
    void powerChanged(Power p);
    void nodesChanged();

private:
    friend class AbstractCircuitNode;
    void setNode(CableSide s, CableEnd node);

private:
    Mode mMode = Mode::Unifilar;

    QVector<ElectricCircuit *> mFirstCableCirctuits;
    QVector<ElectricCircuit *> mSecondCableCirctuits;

    CableEnd mNodeA;
    CableEnd mNodeB;
};

#endif // CIRCUITCABLE_H
