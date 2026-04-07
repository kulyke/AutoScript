#include "shoptask.h"
#include "visionengine.h"
#include "devicecontroller.h"

#include "states/stmainmenutoshop.h"

#include <QDebug>

ShopTask::ShopTask(VisionEngine* vision, DeviceController* device, QObject *parent)
    : TaskBase(parent), m_vision(vision), m_device(device)
{
    // 设置初始状态为 StMainMenuToShop
    this->setInitialState(new StMainMenuToShop(m_vision, m_device));
}

ShopTask::~ShopTask()
{
    qDebug()<<"ShopTask destroyed";
}

QString ShopTask::name() const
{
    return "ShopTask";
}