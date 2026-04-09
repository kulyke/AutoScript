#ifndef STWORLDMAPFOCUSTARGETZONE_H
#define STWORLDMAPFOCUSTARGETZONE_H

#include "StepFlowState.h"

#include <memory>

class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;
struct WorldMapRuntimeContext;

class StWorldMapFocusTargetZone : public StepFlowState
{
public:
    StWorldMapFocusTargetZone(DeviceController* device,
                              std::shared_ptr<WorldZoneCatalog> zoneCatalog,
                              std::shared_ptr<WorldMapTransform> transform,
                              std::shared_ptr<WorldMapRuntimeContext> runtimeContext,
                              QObject* parent = nullptr);
    ~StWorldMapFocusTargetZone() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    DeviceController* m_device;
    std::shared_ptr<WorldZoneCatalog> m_zoneCatalog;
    std::shared_ptr<WorldMapTransform> m_transform;
    std::shared_ptr<WorldMapRuntimeContext> m_runtimeContext;
};

#endif