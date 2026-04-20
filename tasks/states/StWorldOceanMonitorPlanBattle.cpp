#include "stworldoceanmonitorplanbattle.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "StWorldOceanHandleMeowfficerShop.h"
#include "StWorldOceanRecoverOil.h"
#include "WorldOceanPlanBattleRuntimeContext.h"

#include <QDebug>

StWorldOceanMonitorPlanBattle::StWorldOceanMonitorPlanBattle(VisionEngine* vision,
                                                             DeviceController* device,
                                                             std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                                                             QObject* parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_runtimeContext(std::move(runtimeContext))
{
}

StWorldOceanMonitorPlanBattle::~StWorldOceanMonitorPlanBattle()
{
    qDebug() << "StWorldOceanMonitorPlanBattle destroyed";
}

QString StWorldOceanMonitorPlanBattle::name() const
{
    return "StWorldOceanMonitorPlanBattle";
}

StepFlowState* StWorldOceanMonitorPlanBattle::update(const QImage& frame)
{
    if (!m_runtimeContext) {
        setRuntimeMessage("[StWorldOceanMonitorPlanBattle] runtime context is null");
        return this;
    }

    if (m_runtimeContext->supplyBoxDepleted) {
        setRuntimeMessage("[StWorldOceanMonitorPlanBattle] supply boxes depleted; stop plan battle");
        return nullptr;
    }

    ++m_observeFrames;

    // 每3帧观察一次石油值，过于频繁可能导致性能问题(这里可能存在场景切换和战斗状态识别不及时的问题，后续可以考虑增加额外的状态来专门处理场景切换和战斗结束的识别)
    if (m_vision && (m_observeFrames == 1 || m_observeFrames % 3 == 0)) {
        const std::optional<int> oilValue = m_vision->readWorldZoneOilCount(frame);
        if (oilValue.has_value()) {
            qDebug()<<QString("Observed oil value: %1").arg(*oilValue);
            m_runtimeContext->lastObservedOil = *oilValue;
            if (*oilValue <= 10) {
                setRuntimeMessage(QString("[StWorldOceanMonitorPlanBattle] oil=%1 <= 10; recover oil")
                                      .arg(*oilValue));
                return new StWorldOceanRecoverOil(
                    m_vision,
                    m_device,
                    m_runtimeContext);
            }
        }
    }

    // if (hasTemplate(frame, "worldZone.planBattle.noAutoEvent.message")) {
    //     if (hasTemplate(frame, "worldZone.meowfficer.button")) {
    //         setRuntimeMessage("[StWorldOceanMonitorPlanBattle] meowfficer shop detected; handle supply purchase");
    //         return new StWorldOceanHandleMeowfficerShop(
    //             m_vision,
    //             m_device,
    //             m_runtimeContext);
    //     }

    //     setRuntimeMessage("[StWorldOceanMonitorPlanBattle] plan battle stopped without a supported recovery path");
    //     return nullptr;
    // }

    if (m_observeFrames % 180 == 0) { // 每180帧更新一次监视状态消息
        setRuntimeMessage(QString("[StWorldOceanMonitorPlanBattle] monitoring plan battle; oil=%1 oilRecoveries=%2 supplyPurchases=%3")
                              .arg(m_runtimeContext->lastObservedOil)
                              .arg(m_runtimeContext->oilRecoveryCount)
                              .arg(m_runtimeContext->supplyBoxPurchaseCount));
    }

    return this;
}

StepFlowState* StWorldOceanMonitorPlanBattle::onFlowFinished()
{
    return nullptr;
}

bool StWorldOceanMonitorPlanBattle::hasTemplate(const QImage& frame, const QString& key) const
{
    if (!m_vision) {
        return false;
    }

    QPoint matchPoint;
    return m_vision->findTemplate(frame, key, matchPoint, -1.0);
}