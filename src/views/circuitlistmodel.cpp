#include "circuitlistmodel.h"

#include "../graph/circuitscene.h"

#include "modemanager.h"

#include <QJsonObject>
#include <QJsonArray>

CircuitListModel::CircuitListModel(ModeManager *mgr, QObject *parent)
    : QAbstractTableModel(parent)
    , mModeMgr(mgr)
{
    connect(mModeMgr, &ModeManager::modeChanged,
            this, &CircuitListModel::setMode);
}

CircuitListModel::~CircuitListModel()
{
    clear();
}

QVariant CircuitListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case NameCol:
            return tr("Name");
        case LongNameCol:
            return tr("Long Name");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int CircuitListModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mCircuitScenes.size();
}

int CircuitListModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant CircuitListModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mCircuitScenes.size())
        return QVariant();

    CircuitScene *scene = mCircuitScenes.at(idx.row());

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (idx.column())
        {
        case NameCol:
            return scene->circuitSheetName();
        case LongNameCol:
            return scene->circuitSheetLongName();
        default:
            break;
        }
    }

    return QVariant();
}

bool CircuitListModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return false;

    if (!idx.isValid() || idx.row() >= mCircuitScenes.size())
        return false;

    CircuitScene *scene = mCircuitScenes.at(idx.row());

    if(role == Qt::EditRole)
    {
        switch (idx.column())
        {
        case NameCol:
            return scene->setCircuitSheetName(value.toString());
        case LongNameCol:
            // Always valid
            scene->setCircuitSheetLongName(value.toString());
            return true;
        default:
            break;
        }
    }

    return false;
}

Qt::ItemFlags CircuitListModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mCircuitScenes.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(mModeMgr->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

bool CircuitListModel::isNameAvailable(const QString &name) const
{
    return std::none_of(mCircuitScenes.cbegin(),
                        mCircuitScenes.cend(),
                        [name](CircuitScene *s) -> bool
    {
        return s->circuitSheetName() == name;
    });
}

void CircuitListModel::resetHasUnsavedChanges()
{
    if(!mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = false;

    for(CircuitScene *scene : std::as_const(mCircuitScenes))
        scene->setHasUnsavedChanges(false);
}

CircuitScene *CircuitListModel::addCircuitScene(const QString &name)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return nullptr;

    QString trimmedName = name.trimmed();

    if(!isNameAvailable(trimmedName))
        return nullptr;

    CircuitScene *scene = new CircuitScene(this);
    scene->setCircuitSheetName(trimmedName);

    int row = mCircuitScenes.size();
    beginInsertRows(QModelIndex(), row, row);

    mCircuitScenes.append(scene);

    endInsertRows();

    connect(scene, &CircuitScene::nameChanged,
            this, &CircuitListModel::onSceneNameChanged);
    connect(scene, &CircuitScene::longNameChanged,
            this, &CircuitListModel::onSceneNameChanged);

    onSceneEdited();

    return scene;
}

bool CircuitListModel::removeSceneAtRow(int row)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return false;

    Q_ASSERT(row >= 0);

    beginRemoveRows(QModelIndex(), row, row);

    CircuitScene *scene = mCircuitScenes.takeAt(row);
    delete scene;

    endRemoveRows();

    onSceneEdited();

    return true;
}

void CircuitListModel::clear()
{
    beginResetModel();

    qDeleteAll(mCircuitScenes);
    mCircuitScenes.clear();

    endResetModel();
}

bool CircuitListModel::loadFromJSON(const QJsonObject &obj)
{
    beginResetModel();

    qDeleteAll(mCircuitScenes);
    mCircuitScenes.clear();

    const QJsonArray arr = obj.value("scenes").toArray();
    for(const QJsonValue& v : arr)
    {
        CircuitScene *scene = new CircuitScene(this);
        scene->loadFromJSON(v.toObject(), modeMgr()->circuitFactory());

        connect(scene, &CircuitScene::nameChanged,
                this, &CircuitListModel::onSceneNameChanged);
        mCircuitScenes.append(scene);
    }

    // Initially sort by name
    std::sort(mCircuitScenes.begin(),
              mCircuitScenes.end(),
              [](CircuitScene *a, CircuitScene *b) -> bool
    {
        return a->circuitSheetName() < b->circuitSheetName();
    });

    endResetModel();

    return true;
}

void CircuitListModel::saveToJSON(QJsonObject &obj) const
{
    QJsonArray arr;

    for(CircuitScene *scene : std::as_const(mCircuitScenes))
    {
        QJsonObject sceneObj;
        scene->saveToJSON(sceneObj);
        arr.append(sceneObj);
    }

    obj["scenes"] = arr;
}

void CircuitListModel::onSceneNameChanged(const QString &, CircuitScene *scene)
{
    int row = mCircuitScenes.indexOf(scene);
    Q_ASSERT(row >= 0);

    QModelIndex first = index(row, NameCol);
    QModelIndex last = index(row, LongNameCol);
    emit dataChanged(first, last);
}

void CircuitListModel::setMode(FileMode newMode, FileMode oldMode)
{
    for(CircuitScene *scene : std::as_const(mCircuitScenes))
    {
        scene->setMode(newMode, oldMode);
    }
}

void CircuitListModel::onSceneEdited()
{
    if(mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = true;
    modeMgr()->setFileEdited();
}
