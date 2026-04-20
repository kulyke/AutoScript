#ifndef STWORLDOCEANPLANBATTLEMODE_H
#define STWORLDOCEANPLANBATTLEMODE_H

#include "StepFlowState.h"

#include <memory>

class VisionEngine;
class DeviceController;
struct WorldOceanPlanBattleRuntimeContext;

class StWorldOceanPlanBattleMode : public StepFlowState
{
public:
    StWorldOceanPlanBattleMode(VisionEngine* vision,
                               DeviceController* device,
                               std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext = nullptr,
                               QObject* parent = nullptr);
    ~StWorldOceanPlanBattleMode() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    std::shared_ptr<WorldOceanPlanBattleRuntimeContext> m_runtimeContext;
};

#endif