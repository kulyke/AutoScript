#ifndef STMAINMENUTOSHOP_H
#define STMAINMENUTOSHOP_H

#include "taskstate.h"

class VisionEngine;
class DeviceController;

class StMainMenuToShop : public TaskState
{
public:
    StMainMenuToShop(VisionEngine* vision,
                     DeviceController* device, QObject* parent = nullptr);
    ~StMainMenuToShop() override;

    QString name() const override;

    TaskState* update(const QImage& frame) override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;

};

#endif