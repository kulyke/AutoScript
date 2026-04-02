#include "stshop.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include <QDebug>
#include <memory>

StShop::StShop(VisionEngine *vision, DeviceController *device, QObject *parent)
    : StepFlowState(parent)
{
    m_vision = vision;
    m_device = device;

    addStep(std::make_unique<TimeoutStep>(
        std::make_unique<WaitTemplateStep>(
            m_vision,
            "resources/templates/shop_title.png",
            0.9,
            "Wait shop title"),
        10,
        "Timeout wait shop title"));
}

StShop::~StShop()
{
    qDebug()<<"StShop destroyed";
}

QString StShop::name() const
{
    return "StShop";
}

TaskState* StShop::onFlowFinished()
{
    qDebug() << "Entered shop, task completed";
    return nullptr;
}