#ifndef STWORLDMAPENTERTARGETZONE_H
#define STWORLDMAPENTERTARGETZONE_H

#include "StepFlowState.h"

#include <memory>

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;
struct WorldMapRuntimeContext;

class StWorldMapEnterTargetZone : public StepFlowState
{
public:
    StWorldMapEnterTargetZone(VisionEngine* vision,
                              DeviceController* device,
                              std::shared_ptr<WorldZoneCatalog> zoneCatalog,
                              std::shared_ptr<WorldMapTransform> transform,
                              std::shared_ptr<WorldMapRuntimeContext> runtimeContext,
                              QObject* parent = nullptr);
    ~StWorldMapEnterTargetZone() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    std::shared_ptr<WorldZoneCatalog> m_zoneCatalog;
    std::shared_ptr<WorldMapTransform> m_transform;
    std::shared_ptr<WorldMapRuntimeContext> m_runtimeContext;
};

#endif