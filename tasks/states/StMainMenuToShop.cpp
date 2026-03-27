#include "stmainmenutoshop.h"
#include "visionengine.h"
#include "devicecontroller.h"
#include "stshop.h"

#include <QDebug>

StMainMenuToShop::StMainMenuToShop(VisionEngine *vision, DeviceController *device, QObject *parent)
    : TaskState(parent)
{
    m_vision = vision;
    m_device = device;
}

StMainMenuToShop::~StMainMenuToShop()
{
    qDebug()<<"StMainMenuToShop destroyed";
}

QString StMainMenuToShop::name() const
{
    return "StMainMenuToShop";
}

TaskState* StMainMenuToShop::update(const QImage &frame)
{
    //在主界面寻找商店按钮
    QPoint pt;
    bool found = m_vision->findTemplate(
                frame,
                "templates/shop_button.png",
                pt,
                0.9);

    if(found)
    {
        m_device->tap(pt.x(),pt.y());
        // 进入商店状态
        return new StShop(m_vision,m_device,this);
    }

    return this;
}