#include "TaskRegistry.h"

#include "ShopTask.h"
#include "states/StMainMenuToShop.h"

namespace {

QList<TaskDefinition> buildTaskDefinitions()
{
    return {
        {
            "Daily",
            "Shop",
            "ShopTask",
            "StMainMenuToShop",
            [](VisionEngine* vision, DeviceController* device) -> TaskBase* {
                ShopTask* task = new ShopTask();
                task->setInitialState(new StMainMenuToShop(vision, device));
                return task;
            }
        }
    };
}

}

const QList<TaskDefinition>& TaskRegistry::definitions()
{
    static const QList<TaskDefinition> taskDefinitions = buildTaskDefinitions();
    return taskDefinitions;
}

const TaskDefinition* TaskRegistry::findByTaskTypeName(const QString& taskTypeName)
{
    for (const TaskDefinition& definition : definitions()) {
        if (definition.taskTypeName == taskTypeName) {
            return &definition;
        }
    }

    return nullptr;
}

TaskBase* TaskRegistry::createTask(const QString& taskTypeName,
                                   VisionEngine* vision,
                                   DeviceController* device)
{
    const TaskDefinition* definition = findByTaskTypeName(taskTypeName);
    if (!definition || !definition->createTask) {
        return nullptr;
    }

    return definition->createTask(vision, device);
}