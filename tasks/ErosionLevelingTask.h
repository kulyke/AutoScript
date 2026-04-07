#ifndef EROSIONLEVELINGTASK_H
#define EROSIONLEVELINGTASK_H

#include "taskbase.h"

#include <memory>

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;

class ErosionLevelingTask : public TaskBase
{
    Q_OBJECT
public:
    explicit ErosionLevelingTask(VisionEngine* vision, DeviceController* device, QObject *parent = nullptr);
    ~ErosionLevelingTask();

    QString name() const override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;
    std::unique_ptr<WorldZoneCatalog> m_zoneCatalog;
    std::unique_ptr<WorldMapTransform> m_worldMapTransform;

};

#endif