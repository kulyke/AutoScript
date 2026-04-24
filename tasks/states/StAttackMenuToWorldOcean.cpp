#include "stattackmenutoworldocean.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include "StWorldOceanToWorldMap.h"

#include <QDebug>

StAttackMenuToWorldOcean::StAttackMenuToWorldOcean(VisionEngine *vision,
                                                   DeviceController *device,
                                                   QObject *parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
{
    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "attackMenu.worldZone.button",
                -1.0,
                "Click attackMenu world zone button"),
            1800,
            "Timeout click attackMenu world zone button"),
        2,
        "Retry click attackMenu world zone button"));

    addStep(std::make_unique<DelayMillisecondsStep>(100, "Wait page transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "worldZone.title",
                -1.0,
                "Wait world zone title"),
            4200,
            "Timeout wait world zone title"),
        1,
        "Retry wait world zone title"));
}

StAttackMenuToWorldOcean::~StAttackMenuToWorldOcean()
{
    qDebug()<<"StAttackMenuToWorldOcean destroyed";
}

QString StAttackMenuToWorldOcean::name() const
{
    return "StAttackMenuToWorldOcean";
}

StepFlowState* StAttackMenuToWorldOcean::onFlowFinished()
{
    setRuntimeMessage("[StAttackMenuToWorldOcean] finished");
    return new StWorldOceanToWorldMap(
        m_vision,
        m_device);
}