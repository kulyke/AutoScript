#ifndef STMAINMENUTOSHOP_H
#define STMAINMENUTOSHOP_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;

class StMainMenuToShop : public StepFlowState
{
public:
    StMainMenuToShop(VisionEngine* vision,
                     DeviceController* device, QObject* parent = nullptr);
    ~StMainMenuToShop() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;

};

#endif