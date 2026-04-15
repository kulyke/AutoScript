#include "stworldmapfocustargetzone.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "StWorldMapEnterTargetZone.h"
#include "steps/WorldMapSteps.h"

#include <QDebug>

StWorldMapFocusTargetZone::StWorldMapFocusTargetZone(VisionEngine* vision,
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
    addStep(std::make_unique<FocusTargetWorldZoneStep>(
        m_device,
        m_zoneCatalog.get(),
        m_transform.get(),
        m_runtimeContext.get(),
        "Focus target world zone"));
}

StWorldMapFocusTargetZone::~StWorldMapFocusTargetZone()
{
    qDebug() << "StWorldMapFocusTargetZone destroyed";
}

QString StWorldMapFocusTargetZone::name() const
{
    return "StWorldMapFocusTargetZone";
}

StepFlowState* StWorldMapFocusTargetZone::onFlowFinished()
{
    setRuntimeMessage("[StWorldMapFocusTargetZone] finished");
    return new StWorldMapEnterTargetZone(
        m_vision,
        m_device,
        m_zoneCatalog,
        m_transform,
        m_runtimeContext);
}