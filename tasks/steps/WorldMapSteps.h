#ifndef WORLDMAPSTEPS_H
#define WORLDMAPSTEPS_H

#include "../../core/FlowStep.h"

#include <QString>

class VisionEngine;
class WorldZoneCatalog;
class WorldMapTransform;

class InitializeWorldMapStep : public FlowStep
{
public:
    InitializeWorldMapStep(VisionEngine* vision,
                           WorldZoneCatalog* zoneCatalog,
                           WorldMapTransform* transform,
                           QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    VisionEngine* m_vision;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
};

#endif