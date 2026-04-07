#ifndef SHOPTASK_H
#define SHOPTASK_H

#include "taskbase.h"

class VisionEngine;
class DeviceController;
/**
 * @brief  商店任务，负责完成商店相关的操作
 * 
 */
class ShopTask : public TaskBase
{
    Q_OBJECT
public:
    explicit ShopTask(VisionEngine* vision, DeviceController* device, QObject *parent = nullptr);
    ~ShopTask();

    QString name() const override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;
};

#endif