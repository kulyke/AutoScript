#ifndef STWORLDOCEANMONITORPLANBATTLE_H
#define STWORLDOCEANMONITORPLANBATTLE_H

#include "StepFlowState.h"

#include <memory>

class VisionEngine;
class DeviceController;
struct WorldOceanPlanBattleRuntimeContext;

class StWorldOceanMonitorPlanBattle : public StepFlowState
{
public:
    StWorldOceanMonitorPlanBattle(VisionEngine* vision,
                                  DeviceController* device,
                                  std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                                  QObject* parent = nullptr);
    ~StWorldOceanMonitorPlanBattle() override;

    QString name() const override;
    StepFlowState* update(const QImage& frame) override;

private:
    StepFlowState* onFlowFinished() override;
    bool hasTemplate(const QImage& frame, const QString& key) const;

    VisionEngine* m_vision;
    DeviceController* m_device;
    std::shared_ptr<WorldOceanPlanBattleRuntimeContext> m_runtimeContext;
    int m_observeFrames = 0; // 监视状态已观察的帧数
};

#endif