#ifndef STSHOP_H
#define STSHOP_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;

class StShop : public StepFlowState
{
public:
    StShop(VisionEngine* vision,
           DeviceController* device, QObject* parent = nullptr);
    ~StShop() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;

};

#endif