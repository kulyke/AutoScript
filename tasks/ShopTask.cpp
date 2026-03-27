#include "shoptask.h"
#include "visionengine.h"
#include "devicecontroller.h"

#include <QDebug>

ShopTask::ShopTask(QObject *parent)
    : TaskBase(parent)
{

}

ShopTask::~ShopTask()
{
    qDebug()<<"ShopTask destroyed";
}

QString ShopTask::name() const
{
    return "ShopTask";
}