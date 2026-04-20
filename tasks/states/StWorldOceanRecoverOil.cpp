#include "stworldoceanrecoveroil.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include "StWorldOceanPlanBattleMode.h"
#include "WorldOceanPlanBattleRuntimeContext.h"

#include <QDebug>

StWorldOceanRecoverOil::StWorldOceanRecoverOil(VisionEngine* vision,
                                               DeviceController* device,
                                               std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                                               QObject* parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_runtimeContext(std::move(runtimeContext))
{
    //结束计划作战(关闭自律寻敌)
    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.planBattle.stop.button",
                -1.0,
                "Click stop plan battle button"),
            5,
            "Timeout click stop plan battle button"),
        2,
        "Retry click stop plan battle button"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait stop plan battle transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "worldZone.title",
                -1.0,
                "Wait world zone title before oil refill"),
            5,
            "Timeout wait world zone title before oil refill"),
        1,
        "Retry wait world zone title before oil refill"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.oil.add.button",
                -1.0,
                "Click oil add button"),
            5,
            "Timeout click oil add button"),
        2,
        "Retry click oil add button"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait oil refill dialog"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.oil.refill.confirm.button",
                -1.0,
                "Click oil refill confirm button"),
            5,
            "Timeout click oil refill confirm button"),
        2,
        "Retry click oil refill confirm button"));

    addStep(std::make_unique<DelayFramesStep>(5, "Wait oil refill complete"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "worldZone.title",
                -1.0,
                "Wait world zone title after oil refill"),
            5,
            "Timeout wait world zone title after oil refill"),
        1,
        "Retry wait world zone title after oil refill"));
}

StWorldOceanRecoverOil::~StWorldOceanRecoverOil()
{
    qDebug() << "StWorldOceanRecoverOil destroyed";
}

QString StWorldOceanRecoverOil::name() const
{
    return "StWorldOceanRecoverOil";
}

StepFlowState* StWorldOceanRecoverOil::onFlowFinished()
{
    if (m_runtimeContext) {
        ++m_runtimeContext->oilRecoveryCount;
    }

    setRuntimeMessage("[StWorldOceanRecoverOil] finished");
    return new StWorldOceanPlanBattleMode(
        m_vision,
        m_device,
        m_runtimeContext);
}