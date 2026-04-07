#include "erosionlevelingtask.h"
#include "visionengine.h"
#include "devicecontroller.h"

#include "../core/worldmap/WorldMapTransform.h"
#include "../core/worldmap/WorldZoneCatalog.h"

#include "states/StMainMenuToAttackMenu.h"

#include <QDebug>

ErosionLevelingTask::ErosionLevelingTask(VisionEngine* vision, DeviceController* device, QObject *parent)
    : TaskBase(parent), m_vision(vision), m_device(device)
{
    m_zoneCatalog = std::make_unique<WorldZoneCatalog>();
    m_worldMapTransform = std::make_unique<WorldMapTransform>(m_vision);

    this->setInitialState(new StMainMenuToAttackMenu(
        m_vision,
        m_device,
        m_zoneCatalog.get(),
        m_worldMapTransform.get()));
}

ErosionLevelingTask::~ErosionLevelingTask()
{
    qDebug()<<"ErosionLevelingTask destroyed";
}

QString ErosionLevelingTask::name() const
{
    return "ErosionLevelingTask";
}