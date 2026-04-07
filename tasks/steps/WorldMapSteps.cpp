#include "WorldMapSteps.h"

#include "visionengine.h"
#include "../../core/worldmap/WorldMapTransform.h"
#include "../../core/worldmap/WorldZoneCatalog.h"

#include <QCoreApplication>
#include <QDir>

InitializeWorldMapStep::InitializeWorldMapStep(VisionEngine* vision,
                                               WorldZoneCatalog* zoneCatalog,
                                               WorldMapTransform* transform,
                                               QString stepName)
    : m_vision(vision)
    , m_zoneCatalog(zoneCatalog)
    , m_transform(transform)
    , m_name(std::move(stepName))
{
}

QString InitializeWorldMapStep::name() const
{
    return m_name;
}

FlowStepStatus InitializeWorldMapStep::execute(const QImage& frame)
{
    m_error.clear();
    m_runtimeMessage.clear();

    if (!m_vision) {
        m_error = "vision is null";
        return FlowStepStatus::Failed;
    }
    if (!m_zoneCatalog) {
        m_error = "world zone catalog is null";
        return FlowStepStatus::Failed;
    }
    if (!m_transform) {
        m_error = "world map transform is null";
        return FlowStepStatus::Failed;
    }

    if (m_zoneCatalog->isEmpty()) {
        const QString zoneCatalogPath = QDir(QCoreApplication::applicationDirPath())
            .filePath("resources/world_map/zones.json");
        QString loadError;
        if (!m_zoneCatalog->loadFromJson(zoneCatalogPath, &loadError)) {
            m_error = loadError;
            return FlowStepStatus::Failed;
        }
    }

    if (!m_transform->hasWorldCenter()) {
        const WorldMapCalibration calibration = m_transform->calibration();
        m_transform->setWorldCenter(QPointF(
            calibration.referenceMapSize.width() / 2.0,
            calibration.referenceMapSize.height() / 2.0));
    }

    QString updateError;
    if (!m_transform->update(frame, &updateError)) {
        m_error = updateError;
        return FlowStepStatus::Failed;
    }

    const WorldMapTransformSnapshot snapshot = m_transform->snapshot();
    m_runtimeMessage = QString("world map initialized: center=(%1,%2) screenAnchor=(%3,%4)")
        .arg(snapshot.worldCenter.x(), 0, 'f', 1)
        .arg(snapshot.worldCenter.y(), 0, 'f', 1)
        .arg(snapshot.screenCenter.x(), 0, 'f', 1)
        .arg(snapshot.screenCenter.y(), 0, 'f', 1);
    return FlowStepStatus::Done;
}

QString InitializeWorldMapStep::takeRuntimeMessage()
{
    const QString message = m_runtimeMessage;
    m_runtimeMessage.clear();
    return message;
}

QString InitializeWorldMapStep::errorString() const
{
    return m_error;
}