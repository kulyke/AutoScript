#include "stworldmapbootstrap.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/WorldMapSteps.h"

#include <QDebug>
#include <memory>

StWorldMapBootstrap::StWorldMapBootstrap(VisionEngine* vision,
                                         DeviceController* device,
                                         WorldZoneCatalog* zoneCatalog,
                                         WorldMapTransform* transform,
                                         QObject* parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_zoneCatalog(zoneCatalog)
    , m_transform(transform)
{
    addStep(std::make_unique<InitializeWorldMapStep>(
        m_vision,
        m_zoneCatalog,
        m_transform,
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
    Q_UNUSED(m_device);
    setRuntimeMessage("[StWorldMapBootstrap] finished");
    return nullptr;
}