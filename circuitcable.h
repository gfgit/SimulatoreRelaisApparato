#ifndef CIRCUITCABLE_H
#define CIRCUITCABLE_H

#include <QObject>
#include <QVector>

class ClosedCircuit;
class AbstractCircuitNode;

class CircuitCable : public QObject
{
    Q_OBJECT
public:

    enum class Mode
    {
        Unifilar = 0,
        Bifilar = 1
    };

    enum class Side
    {
        A1 = 0,
        A2 = 1,
        B1 = 2,
        B2 = 3
    };

    enum class Power
    {
        None = 0,
        FirstCable = 1,
        SecondCable = 2,
        BothOn = 3
    };

    struct CableEnd
    {
        AbstractCircuitNode *node = nullptr;
        int nodeContact = 0;
    };

    explicit CircuitCable(QObject *parent = nullptr);
    ~CircuitCable();

    Mode mode() const;
    void setMode(Mode newMode);

    Power powered();

    void addCircuit(ClosedCircuit *circuit, Side s);
    void removeCircuit(ClosedCircuit *circuit);

    inline CableEnd getNode(Side s) const
    {
        switch (s)
        {
        case Side::A1:
            return mNodeA1;
        case Side::A2:
            return mNodeA2;
        case Side::B1:
            return mNodeB1;
        case Side::B2:
            return mNodeB2;
        default:
            break;
        }

        Q_UNREACHABLE();
        return {};
    }

    static inline Side oppositeSide(Side s)
    {
        switch (s)
        {
        case Side::A1:
            return Side::B1;
        case Side::A2:
            return Side::B2;
        case Side::B1:
            return Side::A1;
        case Side::B2:
            return Side::A2;
        default:
            break;
        }

        Q_UNREACHABLE();
        return Side::B1;
    }

signals:
    void modeChanged(Mode m);
    void powerChanged(Power p);

private:
    friend class AbstractCircuitNode;
    void setNode(Side s, CableEnd node);

private:
    Mode mMode = Mode::Bifilar;

    QVector<ClosedCircuit *> mFirstCableCirctuits;
    QVector<ClosedCircuit *> mSecondCableCirctuits;

    CableEnd mNodeA1;
    CableEnd mNodeA2;
    CableEnd mNodeB1;
    CableEnd mNodeB2;
};

#endif // CIRCUITCABLE_H
