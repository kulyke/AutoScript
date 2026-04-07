#ifndef STWORLDOCEANTOWORLDMAP_H
#define STWORLDOCEANTOWORLDMAP_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;

class StWorldOceanToWorldMap : public StepFlowState
{
public:
    StWorldOceanToWorldMap(VisionEngine* vision,
                            DeviceController* device,
                            WorldZoneCatalog* zoneCatalog,
                            WorldMapTransform* transform,
                            QObject* parent = nullptr);
    ~StWorldOceanToWorldMap() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;

};

#endif