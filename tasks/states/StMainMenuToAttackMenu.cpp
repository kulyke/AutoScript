#include "stmainmenutoattackmenu.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include "StAttackMenuToWorldOcean.h"

#include <QDebug>

StMainMenuToAttackMenu::StMainMenuToAttackMenu(VisionEngine *vision,
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
                "mainMenu.attack.button",
                -1.0,
                "Click mainMenu attack button"),
            1800,
            "Timeout click mainMenu attack button"),
        2,
        "Retry click mainMenu attack button"));

    addStep(std::make_unique<DelayMillisecondsStep>(100, "Wait page transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "attackMenu.title",
                -1.0,
                "Wait attack menu title"),
            3500,
            "Timeout wait attack menu title"),
        1,
        "Retry wait attack menu title"));
}

StMainMenuToAttackMenu::~StMainMenuToAttackMenu()
{
    qDebug()<<"StMainMenuToAttackMenu destroyed";
}

QString StMainMenuToAttackMenu::name() const
{
    return "StMainMenuToAttackMenu";
}

StepFlowState* StMainMenuToAttackMenu::onFlowFinished()
{
    setRuntimeMessage("[StMainMenuToAttackMenu] finished");
    return new StAttackMenuToWorldOcean(
        m_vision,
        m_device);
}