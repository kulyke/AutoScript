#ifndef STWORLDMAPBOOTSTRAP_H
#define STWORLDMAPBOOTSTRAP_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;

class StWorldMapBootstrap : public StepFlowState
{
public:
    StWorldMapBootstrap(VisionEngine* vision,
                        DeviceController* device,
                        WorldZoneCatalog* zoneCatalog,
                        WorldMapTransform* transform,
                        QObject* parent = nullptr);
    ~StWorldMapBootstrap() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;
};

#endif