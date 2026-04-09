#ifndef STMAINMENUTOATTACKMENU_H
#define STMAINMENUTOATTACKMENU_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;

class StMainMenuToAttackMenu : public StepFlowState
{
public:
    StMainMenuToAttackMenu(VisionEngine* vision,
                           DeviceController* device,
                           QObject* parent = nullptr);
    ~StMainMenuToAttackMenu() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
};

#endif