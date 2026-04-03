#ifndef TASKREGISTRY_H
#define TASKREGISTRY_H

#include <QList>
#include <QString>

#include <functional>

class TaskBase;
class VisionEngine;
class DeviceController;

/**
 * @brief 任务定义结构体，包含任务的基本信息和创建函数
 * 
 */
struct TaskDefinition
{
    QString category;
    QString displayName;
    QString taskTypeName;
    QString initialStateName;
    std::function<TaskBase*(VisionEngine* vision, DeviceController* device)> createTask;
};

/**
 * @brief 任务注册表，负责维护所有任务的定义和创建
 * 
 */
class TaskRegistry
{
public:
    /**
     * @brief 获取所有任务定义
     * @return 任务定义列表
     */
    static const QList<TaskDefinition>& definitions();
    /**
     * @brief 根据任务类型名称查找任务定义
     * @param taskTypeName 任务类型名称
     * @return 任务定义指针，如果未找到则返回nullptr
     */
    static const TaskDefinition* findByTaskTypeName(const QString& taskTypeName);
    /**
     * @brief 创建任务实例
     * @param taskTypeName 任务类型名称
     * @param vision 视觉引擎指针
     * @param device 设备控制器指针
     * @return 任务实例指针，如果创建失败则返回nullptr
     */
    static TaskBase* createTask(const QString& taskTypeName,
                                VisionEngine* vision,
                                DeviceController* device);
};

#endif