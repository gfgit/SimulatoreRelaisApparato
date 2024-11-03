#ifndef MODEMANAGER_H
#define MODEMANAGER_H

#include <QObject>

#include "../enums/filemodes.h"

class NodeEditFactory;
class CircuitListModel;

class RelaisModel;

class QJsonObject;

class ModeManager : public QObject
{
    Q_OBJECT
public:
    explicit ModeManager(QObject *parent = nullptr);

    inline FileMode mode() const
    {
        return mMode;
    }

    void setMode(FileMode newMode);

    inline bool fileNeedsSaving() const
    {
        // Prevent showing modified file while loading
        if(mMode == FileMode::LoadingFile)
            return false;
        return mFileWasEdited;
    }

    void setFileEdited();
    void resetFileEdited();

    NodeEditFactory *circuitFactory() const;

    CircuitListModel *circuitList() const;

    bool loadFromJSON(const QJsonObject &obj);
    void saveToJSON(QJsonObject &obj) const;
    void clearAll();

    RelaisModel *relaisModel() const;

signals:
    void modeChanged(FileMode newMode, FileMode oldMode);
    void fileEdited(bool val);

private:
    FileMode mMode = FileMode::Editing;

    NodeEditFactory *mCircuitFactory;
    CircuitListModel *mCircuitList;

    RelaisModel *mRelaisModel;

    bool mFileWasEdited = false;
};

#endif // MODEMANAGER_H
