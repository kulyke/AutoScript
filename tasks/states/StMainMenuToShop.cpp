#include "stmainmenutoshop.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include <QDebug>
#include <memory>

StMainMenuToShop::StMainMenuToShop(VisionEngine *vision, DeviceController *device, QObject *parent)
    : StepFlowState(parent)
{
    m_vision = vision;
    m_device = device;

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "shop.button",
                -1.0,
                "Click shop button"),
            3,
            "Timeout click shop button"),
        2,
        "Retry click shop button"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait page transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "shop.title",
                -1.0,
                "Wait shop title"),
            5,
            "Timeout wait shop title"),
        1,
        "Retry wait shop title"));
}

StMainMenuToShop::~StMainMenuToShop()
{
    qDebug()<<"StMainMenuToShop destroyed";
}

QString StMainMenuToShop::name() const
{
    return "StMainMenuToShop";
}

StepFlowState* StMainMenuToShop::onFlowFinished()
{
    setRuntimeMessage("[StMainMenuToShop] finished");
    return nullptr;
}