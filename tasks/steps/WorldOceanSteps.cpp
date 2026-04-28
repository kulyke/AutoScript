#include "WorldOceanSteps.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "../states/WorldOceanPlanBattleRuntimeContext.h"

#include <algorithm>

namespace {

constexpr int kOilRefillStageSettleMs = 180;

struct SupplyPlan
{
    int blueCount = 0;// 计划使用的蓝色补给数量
    int purpleCount = 0;// 计划使用的紫色补给数量
    int yellowCount = 0;// 计划使用的黄色补给数量
    int totalAddedOil = 0;// 计划使用的补给总油量
    bool reachesTarget = false;// 是否能达到或超过目标油量（101），如果不能达到目标油量则优先满足总补给量最大化
};

int supplyValue(UseOilRefillSuppliesStep::SupplyType supplyType)
{
    switch (supplyType) {
    case UseOilRefillSuppliesStep::SupplyType::Blue:
        return 20;
    case UseOilRefillSuppliesStep::SupplyType::Purple:
        return 50;
    case UseOilRefillSuppliesStep::SupplyType::Yellow:
        return 100;
    case UseOilRefillSuppliesStep::SupplyType::None:
        break;
    }
    return 0;
}
// 判断 candidate 方案是否比 currentBest 方案更优，deficit 是距离目标油量的差值
bool isBetterPlan(const SupplyPlan& candidate, const SupplyPlan& currentBest, int deficit)
{
    if (candidate.reachesTarget != currentBest.reachesTarget) {
        return candidate.reachesTarget;
    }

    if (candidate.reachesTarget) {
        const int candidateOvershoot = candidate.totalAddedOil - deficit;
        const int bestOvershoot = currentBest.totalAddedOil - deficit;
        if (candidateOvershoot != bestOvershoot) {
            return candidateOvershoot < bestOvershoot;
        }
    } else if (candidate.totalAddedOil != currentBest.totalAddedOil) {
        return candidate.totalAddedOil > currentBest.totalAddedOil;
    }

    const int candidateBoxCount = candidate.blueCount + candidate.purpleCount + candidate.yellowCount;
    const int bestBoxCount = currentBest.blueCount + currentBest.purpleCount + currentBest.yellowCount;
    if (candidateBoxCount != bestBoxCount) {
        return candidateBoxCount < bestBoxCount;
    }

    if (candidate.blueCount != currentBest.blueCount) {
        return candidate.blueCount < currentBest.blueCount;
    }
    if (candidate.purpleCount != currentBest.purpleCount) {
        return candidate.purpleCount < currentBest.purpleCount;
    }

    return candidate.yellowCount < currentBest.yellowCount;
}
// 构建补给使用方案，优先满足达到目标油量，其次是总补给量最少，再其次是蓝紫黄优先级
SupplyPlan buildSupplyPlan(int currentOil,
                           int blueCount,
                           int purpleCount,
                           int yellowCount,
                           int blueReserve)
{
    const int deficit = std::max(0, 101 - currentOil);
    const int usableBlue = std::max(0, blueCount - blueReserve);

    SupplyPlan bestPlan;
    bool hasPlan = false;

    for (int yellowUsed = 0; yellowUsed <= yellowCount; ++yellowUsed) {
        for (int purpleUsed = 0; purpleUsed <= purpleCount; ++purpleUsed) {
            for (int blueUsed = 0; blueUsed <= usableBlue; ++blueUsed) {
                const int totalAdded = yellowUsed * 100 + purpleUsed * 50 + blueUsed * 20;
                if (totalAdded <= 0) {
                    continue;
                }

                SupplyPlan candidate;
                candidate.blueCount = blueUsed;
                candidate.purpleCount = purpleUsed;
                candidate.yellowCount = yellowUsed;
                candidate.totalAddedOil = totalAdded;
                candidate.reachesTarget = totalAdded >= deficit;

                if (!hasPlan || isBetterPlan(candidate, bestPlan, deficit)) {
                    bestPlan = candidate;
                    hasPlan = true;
                }
            }
        }
    }

    return bestPlan;
}
// 根据补给使用方案选择下一步要使用的补给类型，优先级为黄 > 紫 > 蓝
UseOilRefillSuppliesStep::SupplyType chooseNextSupplyType(const SupplyPlan& plan)
{
    if (plan.yellowCount > 0) {
        return UseOilRefillSuppliesStep::SupplyType::Yellow;
    }
    if (plan.purpleCount > 0) {
        return UseOilRefillSuppliesStep::SupplyType::Purple;
    }
    if (plan.blueCount > 0) {
        return UseOilRefillSuppliesStep::SupplyType::Blue;
    }
    return UseOilRefillSuppliesStep::SupplyType::None;
}
// 获取补给类型名称字符串
QString supplyTypeName(UseOilRefillSuppliesStep::SupplyType supplyType)
{
    switch (supplyType) {
    case UseOilRefillSuppliesStep::SupplyType::Blue:
        return "blue";
    case UseOilRefillSuppliesStep::SupplyType::Purple:
        return "purple";
    case UseOilRefillSuppliesStep::SupplyType::Yellow:
        return "yellow";
    case UseOilRefillSuppliesStep::SupplyType::None:
        break;
    }
    return "none";
}
// 根据补给类型获取对应的点击位置
QPoint supplyTapPoint(const OilRefillDialogInfo& dialogInfo,
                      UseOilRefillSuppliesStep::SupplyType supplyType)
{
    switch (supplyType) {
    case UseOilRefillSuppliesStep::SupplyType::Blue:
        return dialogInfo.blueSupplyPoint;
    case UseOilRefillSuppliesStep::SupplyType::Purple:
        return dialogInfo.purpleSupplyPoint;
    case UseOilRefillSuppliesStep::SupplyType::Yellow:
        return dialogInfo.yellowSupplyPoint;
    case UseOilRefillSuppliesStep::SupplyType::None:
        break;
    }
    return {};
}

}

PurchaseEnergySupplyBoxStep::PurchaseEnergySupplyBoxStep(VisionEngine* vision,
                                                         DeviceController* device,
                                                         WorldOceanPlanBattleRuntimeContext* runtimeContext,
                                                         QString stepName)
    : m_vision(vision)
    , m_device(device)
    , m_runtimeContext(runtimeContext)
    , m_name(std::move(stepName))
{
}

QString PurchaseEnergySupplyBoxStep::name() const
{
    return m_name;
}

FlowStepStatus PurchaseEnergySupplyBoxStep::execute(const QImage& frame)
{
    m_error.clear();
    m_runtimeMessage.clear();

    if (!m_vision) {
        m_error = "vision is null";
        return FlowStepStatus::Failed;
    }
    if (!m_device) {
        m_error = "device is null";
        return FlowStepStatus::Failed;
    }
    if (!m_runtimeContext) {
        m_error = "world ocean runtime context is null";
        return FlowStepStatus::Failed;
    }

    QPoint matchPoint;
    if (m_vision->findTemplate(frame, "meowfficerShop.energySupplyBox.empty", matchPoint, -1.0)) {
        m_runtimeContext->supplyBoxDepleted = true;
        m_runtimeMessage = "energy supply boxes depleted";
        return FlowStepStatus::Done;
    }

    if (!m_waitingConfirm) {
        if (!m_vision->findTemplate(frame, "meowfficerShop.energySupplyBox.button", matchPoint, -1.0)) {
            return FlowStepStatus::Running;
        }

        if (!m_device->tap(matchPoint.x(), matchPoint.y())) {
            m_error = QString("tap (%1,%2) failed for energy supply box")
                .arg(matchPoint.x())
                .arg(matchPoint.y());
            return FlowStepStatus::Failed;
        }

        m_waitingConfirm = true;
        m_runtimeMessage = "opened energy supply box purchase";
        return FlowStepStatus::Running;
    }

    if (!m_vision->findTemplate(frame, "meowfficerShop.buy.confirm.button", matchPoint, -1.0)) {
        return FlowStepStatus::Running;
    }

    if (!m_device->tap(matchPoint.x(), matchPoint.y())) {
        m_error = QString("tap (%1,%2) failed for energy supply box confirm")
            .arg(matchPoint.x())
            .arg(matchPoint.y());
        return FlowStepStatus::Failed;
    }

    ++m_runtimeContext->supplyBoxPurchaseCount;
    m_runtimeMessage = QString("purchased energy supply box #%1")
        .arg(m_runtimeContext->supplyBoxPurchaseCount);
    return FlowStepStatus::Done;
}

void PurchaseEnergySupplyBoxStep::reset()
{
    m_error.clear();
    m_runtimeMessage.clear();
    m_waitingConfirm = false;
}

QString PurchaseEnergySupplyBoxStep::takeRuntimeMessage()
{
    const QString message = m_runtimeMessage;
    m_runtimeMessage.clear();
    return message;
}

QString PurchaseEnergySupplyBoxStep::errorString() const
{
    return m_error;
}

UseOilRefillSuppliesStep::UseOilRefillSuppliesStep(VisionEngine* vision,
                                                   DeviceController* device,
                                                   WorldOceanPlanBattleRuntimeContext* runtimeContext,
                                                   QString stepName)
    : m_vision(vision)
    , m_device(device)
    , m_runtimeContext(runtimeContext)
    , m_name(std::move(stepName))
{
}

QString UseOilRefillSuppliesStep::name() const
{
    return m_name;
}

FlowStepStatus UseOilRefillSuppliesStep::execute(const QImage& frame)
{
    m_error.clear();
    m_runtimeMessage.clear();

    if (!m_vision) {
        m_error = "vision is null";
        return FlowStepStatus::Failed;
    }
    if (!m_device) {
        m_error = "device is null";
        return FlowStepStatus::Failed;
    }

    const std::optional<OilRefillDialogInfo> dialogInfo = m_vision->readOilRefillDialogInfo(frame);
    if (!dialogInfo.has_value()) {
        return FlowStepStatus::Running;
    }

    if (m_runtimeContext && dialogInfo->currentOil >= 0) {
        m_runtimeContext->lastObservedOil = dialogInfo->currentOil;
    }

    if (m_initialBlueSupplyCount < 0 && dialogInfo->blueSupplyCount >= 0) {
        m_initialBlueSupplyCount = dialogInfo->blueSupplyCount;
    }

    if (m_stage == Stage::WaitAfterSelect) {
        if (!m_stageTimer.isValid() || m_stageTimer.elapsed() < kOilRefillStageSettleMs) {
            return FlowStepStatus::Running;
        }
        return tapUseButton(*dialogInfo);
    }

    if (m_stage == Stage::WaitAfterUse) {
        if (!m_stageTimer.isValid() || m_stageTimer.elapsed() < kOilRefillStageSettleMs) {
            return FlowStepStatus::Running;
        }
        m_stage = Stage::AnalyzeDialog;
        m_pendingSupplyType = SupplyType::None;
    }

    if (dialogInfo->currentOil > 100) {
        if (!m_device->tap(dialogInfo->cancelButtonPoint.x(), dialogInfo->cancelButtonPoint.y())) {
            m_error = QString("tap (%1,%2) failed for oil refill cancel button")
                .arg(dialogInfo->cancelButtonPoint.x())
                .arg(dialogInfo->cancelButtonPoint.y());
            return FlowStepStatus::Failed;
        }

        m_runtimeMessage = QString("closed oil refill dialog with oil=%1")
            .arg(dialogInfo->currentOil);
        return FlowStepStatus::Done;
    }

    const int blueReserve = (m_initialBlueSupplyCount >= 7) ? 7 : 0;
    const SupplyPlan plan = buildSupplyPlan(dialogInfo->currentOil,
                                            std::max(0, dialogInfo->blueSupplyCount),
                                            std::max(0, dialogInfo->purpleSupplyCount),
                                            std::max(0, dialogInfo->yellowSupplyCount),
                                            blueReserve);
    const SupplyType nextSupplyType = chooseNextSupplyType(plan);
    if (nextSupplyType == SupplyType::None) {
        if (!m_device->tap(dialogInfo->cancelButtonPoint.x(), dialogInfo->cancelButtonPoint.y())) {
            m_error = QString("tap (%1,%2) failed for oil refill cancel button")
                .arg(dialogInfo->cancelButtonPoint.x())
                .arg(dialogInfo->cancelButtonPoint.y());
            return FlowStepStatus::Failed;
        }

        m_runtimeMessage = QString("closed oil refill dialog without enough supplies; oil=%1 blue=%2 purple=%3 yellow=%4")
            .arg(dialogInfo->currentOil)
            .arg(dialogInfo->blueSupplyCount)
            .arg(dialogInfo->purpleSupplyCount)
            .arg(dialogInfo->yellowSupplyCount);
        return FlowStepStatus::Done;
    }

    return tapSupplyAndPrepareToUse(*dialogInfo, nextSupplyType);
}

FlowStepStatus UseOilRefillSuppliesStep::tapSupplyAndPrepareToUse(const OilRefillDialogInfo& dialogInfo,
                                                                  SupplyType supplyType)
{
    const QPoint tapPoint = supplyTapPoint(dialogInfo, supplyType);
    if (tapPoint.isNull()) {
        m_error = QString("invalid tap point for %1 supply").arg(supplyTypeName(supplyType));
        return FlowStepStatus::Failed;
    }

    if (!m_device->tap(tapPoint.x(), tapPoint.y())) {
        m_error = QString("tap (%1,%2) failed for %3 supply")
            .arg(tapPoint.x())
            .arg(tapPoint.y())
            .arg(supplyTypeName(supplyType));
        return FlowStepStatus::Failed;
    }

    m_pendingSupplyType = supplyType;
    m_stage = Stage::WaitAfterSelect;
    m_stageTimer.restart();
    m_runtimeMessage = QString("selected %1 supply; currentOil=%2 blue=%3 purple=%4 yellow=%5")
        .arg(supplyTypeName(supplyType))
        .arg(dialogInfo.currentOil)
        .arg(dialogInfo.blueSupplyCount)
        .arg(dialogInfo.purpleSupplyCount)
        .arg(dialogInfo.yellowSupplyCount);
    return FlowStepStatus::Running;
}

FlowStepStatus UseOilRefillSuppliesStep::tapUseButton(const OilRefillDialogInfo& dialogInfo)
{
    if (!m_device->tap(dialogInfo.confirmButtonPoint.x(), dialogInfo.confirmButtonPoint.y())) {
        m_error = QString("tap (%1,%2) failed for oil refill confirm button")
            .arg(dialogInfo.confirmButtonPoint.x())
            .arg(dialogInfo.confirmButtonPoint.y());
        return FlowStepStatus::Failed;
    }

    m_stage = Stage::WaitAfterUse;
    m_stageTimer.restart();
    m_runtimeMessage = QString("used %1 supply (+%2 oil)")
        .arg(supplyTypeName(m_pendingSupplyType))
        .arg(supplyValue(m_pendingSupplyType));
    return FlowStepStatus::Running;
}

void UseOilRefillSuppliesStep::reset()
{
    m_error.clear();
    m_runtimeMessage.clear();
    m_stage = Stage::AnalyzeDialog;
    m_pendingSupplyType = SupplyType::None;
    m_stageTimer.invalidate();
    m_initialBlueSupplyCount = -1;
}

QString UseOilRefillSuppliesStep::takeRuntimeMessage()
{
    const QString message = m_runtimeMessage;
    m_runtimeMessage.clear();
    return message;
}

QString UseOilRefillSuppliesStep::errorString() const
{
    return m_error;
}