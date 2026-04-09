#ifndef EROSIONLEVELINGTASK_H
#define EROSIONLEVELINGTASK_H

#include "taskbase.h"

class VisionEngine;
class DeviceController;

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
};

#endif