#include "stworldoceanmonitorplanbattle.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "StWorldOceanHandleMeowfficerShop.h"
#include "StWorldOceanRecoverOil.h"
#include "WorldOceanPlanBattleRuntimeContext.h"

#include <QDebug>

namespace {

constexpr int kOilObserveIntervalMs = 700;
constexpr int kMonitorStatusLogIntervalMs = 10000;

}

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

    // const bool shouldObserveOil = m_vision
    //     && (!m_hasObservedOilOnce
    //         || !m_oilObserveTimer.isValid()
    //         || m_oilObserveTimer.elapsed() >= kOilObserveIntervalMs);

    // if (shouldObserveOil) {
    //     m_hasObservedOilOnce = true;
    //     m_oilObserveTimer.restart();
    //     const std::optional<int> oilValue = m_vision->readWorldZoneOilCount(frame);
    //     if (oilValue.has_value()) {
    //         qDebug()<<QString("Observed oil value: %1").arg(*oilValue);
    //         m_runtimeContext->lastObservedOil = *oilValue;
    //         if (*oilValue <= 10) {
    //             setRuntimeMessage(QString("[StWorldOceanMonitorPlanBattle] oil=%1 <= 10; recover oil")
    //                                   .arg(*oilValue));
    //             return new StWorldOceanRecoverOil(
    //                 m_vision,
    //                 m_device,
    //                 m_runtimeContext);
    //         }
    //     }
    // }

    // 监测停战条件：如果检测到停战提示，优先点击停战界面的离开奖励按钮（如果有的话），因为它的停战按钮位置和正常停战后返回世界页的停战按钮位置重叠，容易误点导致监测到停战后直接结束计划作战了。
    if (hasTemplate(frame, "worldZone.planBattle.noAutoEvent.message")) {
        QPoint point;
        if (m_vision->findTemplate(frame, "worldZone.planBattle.leaveReward.button", point)) {
            if (m_device->tap(point.x(), point.y())) {
                setRuntimeMessage("[StWorldOceanMonitorPlanBattle] plan battle ended; tapped leave reward button");
                // 先检查猫官商店，因为它的停战按钮位置和正常停战后返回世界页的停战按钮位置重叠，容易误点导致监测到停战后直接结束计划作战了。
                if (hasTemplate(frame, "worldZone.meowfficer.button")) {
                    setRuntimeMessage("[StWorldOceanMonitorPlanBattle] meowfficer shop detected; handle supply purchase");
                    return new StWorldOceanHandleMeowfficerShop(
                    m_vision,
                    m_device,
                    m_runtimeContext);
                }
                qDebug() << "No meowfficer shop detected after tapping leave reward button; checking oil value to determine if we should enter recovery";
                // 没有猫官商店，说明是正常的停战了，直接结束监测并进入恢复流程（如果有的话）
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
                } else {
                    setRuntimeMessage("[StWorldOceanMonitorPlanBattle] failed to read oil value after plan battle ended; end monitoring without recovery");
                    return nullptr;
                }
            } else {
                setRuntimeMessage("[StWorldOceanMonitorPlanBattle] plan battle ended; failed to tap leave reward button");
                return nullptr;
            }
        } else {
            setRuntimeMessage("[StWorldOceanMonitorPlanBattle] plan battle ended; no leave reward button detected");
            return nullptr;
        }
    }

    if (!m_statusLogTimer.isValid() || m_statusLogTimer.elapsed() >= kMonitorStatusLogIntervalMs) {
        m_statusLogTimer.restart();
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