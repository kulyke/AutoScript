#ifndef STWORLDOCEANPLANBATTLEMODE_H
#define STWORLDOCEANPLANBATTLEMODE_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;

class StWorldOceanPlanBattleMode : public StepFlowState
{
public:
    StWorldOceanPlanBattleMode(VisionEngine* vision,
                               DeviceController* device,
                               QObject* parent = nullptr);
    ~StWorldOceanPlanBattleMode() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
};

#endif