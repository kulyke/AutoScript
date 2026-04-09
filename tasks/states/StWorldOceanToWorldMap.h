#ifndef STWORLDOCEANTOWORLDMAP_H
#define STWORLDOCEANTOWORLDMAP_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;

class StWorldOceanToWorldMap : public StepFlowState
{
public:
    StWorldOceanToWorldMap(VisionEngine* vision,
                           DeviceController* device,
                           QObject* parent = nullptr);
    ~StWorldOceanToWorldMap() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
};

#endif