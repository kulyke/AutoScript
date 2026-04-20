#ifndef WORLDOCEANSTEPS_H
#define WORLDOCEANSTEPS_H

#include "../../core/FlowStep.h"

#include <QString>

class VisionEngine;
class DeviceController;
struct WorldOceanPlanBattleRuntimeContext;

/**
 * @brief 购买补给箱步骤
 */
class PurchaseEnergySupplyBoxStep : public FlowStep
{
public:
    PurchaseEnergySupplyBoxStep(VisionEngine* vision,
                                DeviceController* device,
                                WorldOceanPlanBattleRuntimeContext* runtimeContext,
                                QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldOceanPlanBattleRuntimeContext* m_runtimeContext;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
    bool m_waitingConfirm = false;
};

#endif