#ifndef STATTACKMENUTOWORLDOCEAN_H
#define STATTACKMENUTOWORLDOCEAN_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;

class StAttackMenuToWorldOcean : public StepFlowState
{
public:
    StAttackMenuToWorldOcean(VisionEngine* vision,
                            DeviceController* device,
                            WorldZoneCatalog* zoneCatalog,
                            WorldMapTransform* transform,
                            QObject* parent = nullptr);
    ~StAttackMenuToWorldOcean() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;

};

#endif