#include "stworldoceantoworldmap.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include "StWorldMapBootstrap.h"

#include <QDebug>

StWorldOceanToWorldMap::StWorldOceanToWorldMap(VisionEngine *vision,
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
                "worldZone.worldMap.button",
                -1.0,
                "Click world zone world map button"),
            3,
            "Timeout click world zone world map button"),
        2,
        "Retry click world zone world map button"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait page transition"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutStep>(
            std::make_unique<WaitTemplateStep>(
                m_vision,
                "worldMap.title",
                -1.0,
                "Wait world map title"),
            3,
            "Timeout wait world map title"),
        1,
        "Retry wait world map title"));
}

StWorldOceanToWorldMap::~StWorldOceanToWorldMap()
{
    qDebug()<<"StWorldOceanToWorldMap destroyed";
}

QString StWorldOceanToWorldMap::name() const
{
    return "StWorldOceanToWorldMap";
}

StepFlowState* StWorldOceanToWorldMap::onFlowFinished()
{
    setRuntimeMessage("[StWorldOceanToWorldMap] finished");
    return new StWorldMapBootstrap(
        m_vision,
        m_device);

}