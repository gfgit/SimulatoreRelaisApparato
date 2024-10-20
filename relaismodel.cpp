#include "relaismodel.h"

#include "abstractrelais.h"

#include <QColor>

RelaisModel::RelaisModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant RelaisModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && section == 0 && role == Qt::DisplayRole)
    {
        return tr("Relais");
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

int RelaisModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mRelais.size();
}

QVariant RelaisModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mRelais.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return mRelais.at(idx.row())->name();
    case Qt::DecorationRole:
    {
        QColor color = Qt::black;
        switch (mRelais.at(idx.row())->state())
        {
        case AbstractRelais::State::Up:
            color = Qt::red;
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color.setRgb(255, 140, 140); // Light red
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }

        return color;
    }
    default:
        break;
    }

    // FIXME: Implement me!
    return QVariant();
}

bool RelaisModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid() || idx.row() >= mRelais.size() || role != Qt::EditRole)
        return false;

    QString name = value.toString().simplified();
    if(name.isEmpty())
        return false;

    mRelais.at(idx.row())->setName(name);
    emit dataChanged(idx, idx);
    return true;
}

Qt::ItemFlags RelaisModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.row() >= mRelais.size())
        return Qt::ItemFlags();

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void RelaisModel::addRelay(AbstractRelais *r)
{
    int row = mRelais.size();
    beginInsertRows(QModelIndex(), row, row);

    connect(r, &QObject::destroyed, this, &RelaisModel::onRelayDestroyed);
    connect(r, &AbstractRelais::nameChanged, this, &RelaisModel::onRelayChanged);
    connect(r, &AbstractRelais::stateChanged, this, &RelaisModel::onRelayChanged);
    mRelais.append(r);

    endInsertRows();
}

void RelaisModel::removeRelay(AbstractRelais *r)
{
    int row = mRelais.indexOf(r);
    if(row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);

    disconnect(r, &QObject::destroyed, this, &RelaisModel::onRelayDestroyed);
    disconnect(r, &AbstractRelais::nameChanged, this, &RelaisModel::onRelayChanged);
    disconnect(r, &AbstractRelais::stateChanged, this, &RelaisModel::onRelayChanged);
    mRelais.removeAt(row);

    endRemoveRows();
}

AbstractRelais *RelaisModel::relayAt(int row) const
{
    return mRelais.value(row, nullptr);
}

AbstractRelais *RelaisModel::getRelay(const QString &name)
{
    for(int i = 0; i < mRelais.size(); i++)
    {
        if(mRelais.at(i)->name() == name)
            return mRelais.at(i);
    }
    return nullptr;
}

void RelaisModel::onRelayChanged(AbstractRelais *r)
{
    int row = mRelais.indexOf(r);
    Q_ASSERT(row >= 0);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);
}

void RelaisModel::onRelayDestroyed(QObject *obj)
{
    AbstractRelais *r = static_cast<AbstractRelais *>(obj);
    int row = mRelais.indexOf(r);
    Q_ASSERT(row >= 0);

    beginRemoveRows(QModelIndex(), row, row);
    mRelais.removeAt(row);
    endRemoveRows();
}
