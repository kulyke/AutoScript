#include "stworldoceanplanbattlemode.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include "StWorldOceanMonitorPlanBattle.h"
#include "WorldOceanPlanBattleRuntimeContext.h"

#include <QDebug>

StWorldOceanPlanBattleMode::StWorldOceanPlanBattleMode(VisionEngine *vision,
                                                       DeviceController *device,
                                                       std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                                                   QObject *parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_runtimeContext(runtimeContext ? std::move(runtimeContext)
                                      : std::make_shared<WorldOceanPlanBattleRuntimeContext>())
{
    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.planBattle.button",
                -1.0,
                "Click plan battle button on world zone page"),
            1800,
            "Timeout click plan battle button on world zone page"),
        2,
        "Retry click plan battle button on world zone page"));

    addStep(std::make_unique<DelayMillisecondsStep>(100, "Wait page transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "planBattle.title",
                -1.0,
                "Wait plan battle title"),
            3200,
            "Timeout wait plan battle title"),
        1,
        "Retry wait plan battle title"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "planBattle.confirm.button",
                -1.0,
                "Click confirm battle plan button on plan battle page"),
            1800,
            "Timeout click confirm battle plan button on plan battle page"),
        2,
        "Retry click confirm battle plan button on plan battle page"));
}

StWorldOceanPlanBattleMode::~StWorldOceanPlanBattleMode()
{
    qDebug()<<"StWorldOceanPlanBattleMode destroyed";
}

QString StWorldOceanPlanBattleMode::name() const
{
    return "StWorldOceanPlanBattleMode";
}

StepFlowState* StWorldOceanPlanBattleMode::onFlowFinished()
{
    setRuntimeMessage("[StWorldOceanPlanBattleMode] finished");
    return new StWorldOceanMonitorPlanBattle(
        m_vision,
        m_device,
        m_runtimeContext);
}