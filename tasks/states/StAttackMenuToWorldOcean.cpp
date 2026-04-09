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
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "attackMenu.worldOcean.button",
                -1.0,
                "Click attackMenu world ocean button"),
            3,
            "Timeout click attackMenu world ocean button"),
        2,
        "Retry click attackMenu world ocean button"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait page transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "worldOcean.title",
                -1.0,
                "Wait world ocean title"),
            3,
            "Timeout wait world ocean title"),
        1,
        "Retry wait world ocean title"));
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