#include "stworldmapbootstrap.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "StWorldMapResolveCurrentZone.h"
#include "steps/WorldMapSteps.h"

#include "../core/worldmap/WorldMapTypes.h"
#include "../core/worldmap/WorldMapRuntimeContext.h"
#include "../core/worldmap/WorldMapTransform.h"
#include "../core/worldmap/WorldZoneCatalog.h"

#include <QDebug>

StWorldMapBootstrap::StWorldMapBootstrap(VisionEngine* vision,
                                         DeviceController* device,
                                         QObject* parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_zoneCatalog(std::make_shared<WorldZoneCatalog>())
    , m_worldMapTransform(std::make_shared<WorldMapTransform>(m_vision))
    , m_worldMapRuntimeContext(std::make_shared<WorldMapRuntimeContext>())
{
    WorldMapGotoRequest initialWorldMapRequest; //初始请求可以是一个默认值，后续步骤会根据实际情况更新这个请求
    initialWorldMapRequest.targetZoneId = 22;
    
    m_worldMapRuntimeContext->gotoRequest = initialWorldMapRequest;

    addStep(std::make_unique<InitializeWorldMapStep>(
        m_vision,
        m_zoneCatalog.get(),
        m_worldMapTransform.get(),
        "Initialize world map context"));
}

StWorldMapBootstrap::~StWorldMapBootstrap()
{
    qDebug() << "StWorldMapBootstrap destroyed";
}

QString StWorldMapBootstrap::name() const
{
    return "StWorldMapBootstrap";
}

StepFlowState* StWorldMapBootstrap::onFlowFinished()
{
    setRuntimeMessage("[StWorldMapBootstrap] finished");
    return new StWorldMapResolveCurrentZone(
        m_vision,
        m_device,
        m_zoneCatalog,
        m_worldMapTransform,
        m_worldMapRuntimeContext);
}