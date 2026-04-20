#include "stworldoceanhandlemeowfficershop.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"
#include "steps/WorldOceanSteps.h"

#include "StWorldOceanPlanBattleMode.h"
#include "WorldOceanPlanBattleRuntimeContext.h"

#include <QDebug>

StWorldOceanHandleMeowfficerShop::StWorldOceanHandleMeowfficerShop(VisionEngine* vision,
                                                                   DeviceController* device,
                                                                   std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                                                                   QObject* parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_runtimeContext(std::move(runtimeContext))
{
    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.meowfficer.button",
                -1.0,
                "Click meowfficer shop button"),
            5,
            "Timeout click meowfficer shop button"),
        2,
        "Retry click meowfficer shop button"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait meowfficer shop transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "meowfficerShop.title",
                -1.0,
                "Wait meowfficer shop title"),
            5,
            "Timeout wait meowfficer shop title"),
        1,
        "Retry wait meowfficer shop title"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<PurchaseEnergySupplyBoxStep>(
                m_vision,
                m_device,
                m_runtimeContext.get(),
                "Purchase energy supply box"),
            20,
            "Timeout purchase energy supply box"),
        1,
        "Retry purchase energy supply box"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait purchase settle"));

    addStep(std::make_unique<KeyEventStep>(
        m_device,
        4,
        "Back from meowfficer shop"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait return from meowfficer shop"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "worldZone.title",
                -1.0,
                "Wait world zone title after meowfficer shop"),
            5,
            "Timeout wait world zone title after meowfficer shop"),
        1,
        "Retry wait world zone title after meowfficer shop"));
}

StWorldOceanHandleMeowfficerShop::~StWorldOceanHandleMeowfficerShop()
{
    qDebug() << "StWorldOceanHandleMeowfficerShop destroyed";
}

QString StWorldOceanHandleMeowfficerShop::name() const
{
    return "StWorldOceanHandleMeowfficerShop";
}

StepFlowState* StWorldOceanHandleMeowfficerShop::onFlowFinished()
{
    setRuntimeMessage("[StWorldOceanHandleMeowfficerShop] finished");

    if (m_runtimeContext && m_runtimeContext->supplyBoxDepleted) {
        return nullptr;
    }

    return new StWorldOceanPlanBattleMode(
        m_vision,
        m_device,
        m_runtimeContext);
}