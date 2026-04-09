#include "stworldmapresolvecurrentzone.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "StWorldMapFocusTargetZone.h"
#include "steps/WorldMapSteps.h"

#include <QDebug>

StWorldMapResolveCurrentZone::StWorldMapResolveCurrentZone(VisionEngine* vision,
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
    addStep(std::make_unique<ResolveCurrentWorldZoneStep>(
        m_zoneCatalog.get(),
        m_transform.get(),
        m_runtimeContext.get(),
        "Resolve current world zone"));
}

StWorldMapResolveCurrentZone::~StWorldMapResolveCurrentZone()
{
    qDebug() << "StWorldMapResolveCurrentZone destroyed";
}

QString StWorldMapResolveCurrentZone::name() const
{
    return "StWorldMapResolveCurrentZone";
}

StepFlowState* StWorldMapResolveCurrentZone::onFlowFinished()
{
    setRuntimeMessage("[StWorldMapResolveCurrentZone] finished");
    return new StWorldMapFocusTargetZone(
        m_device,
        m_zoneCatalog,
        m_transform,
        m_runtimeContext);
}