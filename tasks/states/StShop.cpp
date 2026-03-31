#include "stshop.h"
#include "visionengine.h"
#include "devicecontroller.h"

#include <QDebug>

StShop::StShop(VisionEngine *vision, DeviceController *device, QObject *parent)
    : TaskState(parent)
{
    m_vision = vision;
    m_device = device;
}

StShop::~StShop()
{
    qDebug()<<"StShop destroyed";
}

QString StShop::name() const
{
    return "StShop";
}

TaskState* StShop::update(const QImage &frame)
{
    //判定当前是否已经进入商店界面(根据商店主题)
    QPoint pt;
    bool found = m_vision->findTemplate(
                frame,
                "resources/templates/shop_title.png",
                pt,
                0.9);

    if(found)
    {
        // 已经进入商店,结束任务(后续扩展为从商店购买物品)
        qDebug()<<"Entered shop, task completed";
        return nullptr;
    }

    return this;
}