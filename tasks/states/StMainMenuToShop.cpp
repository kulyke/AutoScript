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

    addStep(std::make_unique<TimeoutStep>(
        std::make_unique<ClickTemplateStep>(
            m_vision,
            m_device,
            "resources/templates/shop_button.png",
            0.9,
            "Click shop button"),
        10,
        "Timeout click shop button"));

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

TaskState* StMainMenuToShop::onFlowFinished()
{
    return new StShop(m_vision, m_device);
}