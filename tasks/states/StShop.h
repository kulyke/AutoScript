#ifndef STSHOP_H
#define STSHOP_H

#include "taskstate.h"

class VisionEngine;
class DeviceController;

class StShop : public TaskState
{
public:
    StShop(VisionEngine* vision,
           DeviceController* device, QObject* parent = nullptr);
    ~StShop() override;

    QString name() const override;

    TaskState* update(const QImage& frame) override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;

};

#endif