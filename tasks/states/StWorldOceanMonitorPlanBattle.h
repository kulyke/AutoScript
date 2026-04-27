#ifndef STWORLDOCEANMONITORPLANBATTLE_H
#define STWORLDOCEANMONITORPLANBATTLE_H

#include "StepFlowState.h"

#include <QElapsedTimer>
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
    StepFlowState* resolvePostBattleOutcome(const QImage& frame);

    VisionEngine* m_vision;
    DeviceController* m_device;
    std::shared_ptr<WorldOceanPlanBattleRuntimeContext> m_runtimeContext;
    QElapsedTimer m_postBattleTransitionTimer;
    bool m_waitingPostBattleOutcome = false;
};

#endif