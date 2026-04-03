#include "stmainmenutoshop.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "stshop.h"
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
            5,
            "Timeout click shop button"),
        2,
        "Retry click shop button"));

    addStep(std::make_unique<DelayFramesStep>(8, "Wait page transition"));
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
    setRuntimeMessage("[StMainMenuToShop] transition ready: enter StShop");
    return new StShop(m_vision, m_device);
}