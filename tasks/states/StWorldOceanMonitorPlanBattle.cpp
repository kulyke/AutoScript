#include "stworldoceanmonitorplanbattle.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "StWorldOceanHandleMeowfficerShop.h"
#include "StWorldOceanRecoverOil.h"
#include "WorldOceanPlanBattleRuntimeContext.h"

#include <QDebug>

namespace {

constexpr int kPostBattleOutcomeSettleMs = 300;
constexpr int kPostBattleOutcomeTimeoutMs = 4000;

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

    if (m_waitingPostBattleOutcome) {
        return resolvePostBattleOutcome(frame);
    }

    // 监测停战条件：如果检测到停战提示，优先点击停战界面的离开奖励按钮（如果有的话），因为它的停战按钮位置和正常停战后返回世界页的停战按钮位置重叠，容易误点导致监测到停战后直接结束计划作战了。
    if (hasTemplate(frame, "worldZone.planBattle.noAutoEvent.message")) {
        QPoint point;
        if (m_vision->findTemplate(frame, "worldZone.planBattle.leaveReward.button", point)) {
            if (m_device->tap(point.x(), point.y())) {
                m_waitingPostBattleOutcome = true;
                m_postBattleTransitionTimer.restart();
                setRuntimeMessage("[StWorldOceanMonitorPlanBattle] plan battle ended; tapped leave reward button and waiting for updated frame");
                return this;
            } else {
                setRuntimeMessage("[StWorldOceanMonitorPlanBattle] plan battle ended; failed to tap leave reward button");
                return nullptr;
            }
        } else {
            setRuntimeMessage("[StWorldOceanMonitorPlanBattle] plan battle ended; no leave reward button detected");
            return nullptr;
        }
    }

    return this;
}

StepFlowState* StWorldOceanMonitorPlanBattle::resolvePostBattleOutcome(const QImage& frame)
{
    if (!m_postBattleTransitionTimer.isValid()) {
        m_postBattleTransitionTimer.start();
        return this;
    }

    const qint64 elapsedMs = m_postBattleTransitionTimer.elapsed();
    if (elapsedMs < kPostBattleOutcomeSettleMs) {
        return this;
    }

    const auto clearPendingOutcome = [this]() {
        m_waitingPostBattleOutcome = false;
        m_postBattleTransitionTimer.invalidate();
    };

    if (!hasTemplate(frame, "worldZone.title")) {
        if (elapsedMs >= kPostBattleOutcomeTimeoutMs) {
            clearPendingOutcome();
            setRuntimeMessage("[StWorldOceanMonitorPlanBattle] timed out waiting for world zone page after tapping leave reward button");
            return nullptr;
        }
        return this;
    }

    if (hasTemplate(frame, "worldZone.meowfficer.button")) {
        clearPendingOutcome();
        setRuntimeMessage("[StWorldOceanMonitorPlanBattle] meowfficer shop detected after plan battle ended; handle supply purchase");
        return new StWorldOceanHandleMeowfficerShop(
            m_vision,
            m_device,
            m_runtimeContext);
    }

    const std::optional<int> oilValue = m_vision ? m_vision->readWorldZoneOilCount(frame) : std::nullopt;
    if (oilValue.has_value()) {
        qDebug() << QString("Observed oil value: %1").arg(*oilValue);
        m_runtimeContext->lastObservedOil = *oilValue;
        clearPendingOutcome();
        if (*oilValue <= 10) {
            setRuntimeMessage(QString("[StWorldOceanMonitorPlanBattle] oil=%1 <= 10; recover oil")
                                  .arg(*oilValue));
            return new StWorldOceanRecoverOil(
                m_vision,
                m_device,
                m_runtimeContext);
        }

        setRuntimeMessage(QString("[StWorldOceanMonitorPlanBattle] plan battle ended; oil=%1 and no recovery is needed")
                              .arg(*oilValue));
        return nullptr;
    }

    if (elapsedMs >= kPostBattleOutcomeTimeoutMs) {
        clearPendingOutcome();
        setRuntimeMessage("[StWorldOceanMonitorPlanBattle] failed to read oil value after plan battle ended; end monitoring without recovery");
        return nullptr;
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