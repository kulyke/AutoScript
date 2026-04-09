#ifndef STWORLDMAPBOOTSTRAP_H
#define STWORLDMAPBOOTSTRAP_H

#include "StepFlowState.h"

#include <memory>

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;
struct WorldMapRuntimeContext;

class StWorldMapBootstrap : public StepFlowState
{
public:
    StWorldMapBootstrap(VisionEngine* vision,
                        DeviceController* device,
                        QObject* parent = nullptr);
    ~StWorldMapBootstrap() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    std::shared_ptr<WorldZoneCatalog> m_zoneCatalog;
    std::shared_ptr<WorldMapTransform> m_worldMapTransform;
    std::shared_ptr<WorldMapRuntimeContext> m_worldMapRuntimeContext;
};

#endif