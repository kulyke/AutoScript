#ifndef STWORLDOCEANRECOVEROIL_H
#define STWORLDOCEANRECOVEROIL_H

#include "StepFlowState.h"

#include <memory>

class VisionEngine;
class DeviceController;
struct WorldOceanPlanBattleRuntimeContext;

class StWorldOceanRecoverOil : public StepFlowState
{
public:
    StWorldOceanRecoverOil(VisionEngine* vision,
                           DeviceController* device,
                           std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                           QObject* parent = nullptr);
    ~StWorldOceanRecoverOil() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    std::shared_ptr<WorldOceanPlanBattleRuntimeContext> m_runtimeContext;
};

#endif