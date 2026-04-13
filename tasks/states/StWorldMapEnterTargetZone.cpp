#include "stworldmapentertargetzone.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"
#include "steps/WorldMapSteps.h"

#include <QDebug>

StWorldMapEnterTargetZone::StWorldMapEnterTargetZone(VisionEngine* vision,
                                                     DeviceController* device,
                                                     std::shared_ptr<WorldZoneCatalog> zoneCatalog,
                                                     std::shared_ptr<WorldMapTransform> transform,
                                                     std::shared_ptr<WorldMapRuntimeContext> runtimeContext,
                                                     QObject* parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_zoneCatalog(std::move(zoneCatalog))
    , m_transform(std::move(transform))
    , m_runtimeContext(std::move(runtimeContext))
{
    addStep(std::make_unique<RetryStep>(
        std::make_unique<TapTargetWorldZoneStep>(
            m_device,
            m_zoneCatalog.get(),
            m_transform.get(),
            m_runtimeContext.get(),
            "Tap target world zone"),
        1,
        "Retry tap target world zone"));

    addStep(std::make_unique<DelayFramesStep>(3, "Wait target zone transition"));

    // addStep(std::make_unique<RetryStep>(
    //     std::make_unique<TimeoutStep>(
    //         std::make_unique<VerifyTargetWorldZoneEntryStep>(
    //             m_vision,
    //             m_zoneCatalog.get(),
    //             m_runtimeContext.get(),
    //             "Verify target world zone entry"),
    //         20,
    //         "Timeout verify target world zone entry"),
    //     1,
    //     "Retry verify target world zone entry"));
}

StWorldMapEnterTargetZone::~StWorldMapEnterTargetZone()
{
    qDebug() << "StWorldMapEnterTargetZone destroyed";
}

QString StWorldMapEnterTargetZone::name() const
{
    return "StWorldMapEnterTargetZone";
}

StepFlowState* StWorldMapEnterTargetZone::onFlowFinished()
{
    setRuntimeMessage("[StWorldMapEnterTargetZone] finished");
    return nullptr;
}