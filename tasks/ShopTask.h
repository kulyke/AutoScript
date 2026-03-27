#ifndef SHOPTASK_H
#define SHOPTASK_H

#include "taskbase.h"

#include "states/stmainmenutoshop.h"
#include "states/stshop.h"

/**
 * @brief  商店任务，负责完成商店相关的操作
 * 
 */
class ShopTask : public TaskBase
{
    Q_OBJECT
public:
    explicit ShopTask(QObject *parent = nullptr);
    ~ShopTask();

    QString name() const override;

};

#endif