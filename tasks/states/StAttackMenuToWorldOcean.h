#ifndef STATTACKMENUTOWORLDOCEAN_H
#define STATTACKMENUTOWORLDOCEAN_H

#include "StepFlowState.h"

class VisionEngine;
class DeviceController;

class StAttackMenuToWorldOcean : public StepFlowState
{
public:
    StAttackMenuToWorldOcean(VisionEngine* vision,
                             DeviceController* device,
                             QObject* parent = nullptr);
    ~StAttackMenuToWorldOcean() override;

    QString name() const override;

private:
    StepFlowState* onFlowFinished() override;

    VisionEngine* m_vision;
    DeviceController* m_device;
};

#endif