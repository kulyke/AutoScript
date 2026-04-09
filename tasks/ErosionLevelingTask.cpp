#include "erosionlevelingtask.h"
#include "visionengine.h"
#include "devicecontroller.h"

#include "states/StMainMenuToAttackMenu.h"

#include <QDebug>

ErosionLevelingTask::ErosionLevelingTask(VisionEngine* vision, DeviceController* device, QObject *parent)
    : TaskBase(parent), m_vision(vision), m_device(device)
{
    this->setInitialState(new StMainMenuToAttackMenu(
        m_vision,
        m_device));
}

ErosionLevelingTask::~ErosionLevelingTask()
{
    qDebug()<<"ErosionLevelingTask destroyed";
}

QString ErosionLevelingTask::name() const
{
    return "ErosionLevelingTask";
}