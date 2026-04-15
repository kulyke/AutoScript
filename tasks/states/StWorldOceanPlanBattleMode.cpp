#include "stworldoceanplanbattlemode.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include <QDebug>

StWorldOceanPlanBattleMode::StWorldOceanPlanBattleMode(VisionEngine *vision,
                                                       DeviceController *device,
                                                   QObject *parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
{
    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.planBattle.button",
                -1.0,
                "Click plan battle button on world zone page"),
            3,
            "Timeout click plan battle button on world zone page"),
        2,
        "Retry click plan battle button on world zone page"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait page transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "planBattle.title",
                -1.0,
                "Wait plan battle title"),
            3,
            "Timeout wait plan battle title"),
        1,
        "Retry wait plan battle title"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "planBattle.confirm.button",
                -1.0,
                "Click confirm battle plan button on plan battle page"),
            3,
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
    return nullptr;
}