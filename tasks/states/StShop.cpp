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

StShop::~StShop()
{
    qDebug()<<"StShop destroyed";
}

QString StShop::name() const
{
    return "StShop";
}

StepFlowState* StShop::onFlowFinished()
{
    setRuntimeMessage("[StShop] flow completed: shop page confirmed");
    return nullptr;
}