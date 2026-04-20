#include "WorldOceanSteps.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "../states/WorldOceanPlanBattleRuntimeContext.h"

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