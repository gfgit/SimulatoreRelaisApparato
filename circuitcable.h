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
        Bifilar1,
        Bifilar2,
        BifilarBoth
    };

    enum class Pole
    {
        First = 0,
        Second
    };

    enum class Side
    {
        A = 0,
        B = 1
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

    struct CableContact
    {
        CircuitCable *cable = nullptr;
        CircuitCable::Side side = CircuitCable::Side::A;
        CircuitCable::Pole pole = CircuitCable::Pole::First;
    };

    explicit CircuitCable(QObject *parent = nullptr);
    ~CircuitCable();

    Mode mode() const;
    void setMode(Mode newMode);

    Power powered();

    void addCircuit(ClosedCircuit *circuit, Pole pole);
    void removeCircuit(ClosedCircuit *circuit);

    inline CableEnd getNode(Side s) const
    {
        switch (s)
        {
        case Side::A:
            return mNodeA;
        case Side::B:
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
    void setNode(Side s, CableEnd node);

private:
    Mode mMode = Mode::Unifilar;

    QVector<ClosedCircuit *> mFirstCableCirctuits;
    QVector<ClosedCircuit *> mSecondCableCirctuits;

    CableEnd mNodeA;
    CableEnd mNodeB;
};

inline CircuitCable::Side operator ~(CircuitCable::Side s)
{
    return s == CircuitCable::Side::A ? CircuitCable::Side::B :
                                        CircuitCable::Side::A;
}

constexpr CircuitCable::Pole operator ~(CircuitCable::Pole value)
{
    if(value == CircuitCable::Pole::First)
        return CircuitCable::Pole::Second;
    return CircuitCable::Pole::First;
}

#endif // CIRCUITCABLE_H
