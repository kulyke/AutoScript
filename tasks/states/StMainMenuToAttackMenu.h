#ifndef STMAINMENUTOATTACKMENU_H
#define STMAINMENUTOATTACKMENU_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;

class StMainMenuToAttackMenu : public StepFlowState
{
public:
    StMainMenuToAttackMenu(VisionEngine* vision,
                            DeviceController* device,
                            WorldZoneCatalog* zoneCatalog,
                            WorldMapTransform* transform,
                            QObject* parent = nullptr);
    ~StMainMenuToAttackMenu() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;

};

#endif