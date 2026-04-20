#ifndef STWORLDOCEANHANDLEMEOWFFICERSHOP_H
#define STWORLDOCEANHANDLEMEOWFFICERSHOP_H

#include "StepFlowState.h"

#include <memory>

class VisionEngine;
class DeviceController;
struct WorldOceanPlanBattleRuntimeContext;

class StWorldOceanHandleMeowfficerShop : public StepFlowState
{
public:
    StWorldOceanHandleMeowfficerShop(VisionEngine* vision,
                                     DeviceController* device,
                                     std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                                     QObject* parent = nullptr);
    ~StWorldOceanHandleMeowfficerShop() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    std::shared_ptr<WorldOceanPlanBattleRuntimeContext> m_runtimeContext;
};

#endif