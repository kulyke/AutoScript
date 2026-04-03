#include "taskmanager.h"
#include "taskbase.h"
#include <QThread>
#include <QUuid>

namespace {

QString taskStatusToText(TaskBase::TaskStatus status)
{
    switch (status) {
    case TaskBase::Idle:
        return "Idle";
    case TaskBase::Waiting:
        return "Waiting";
    case TaskBase::Running:
        return "Running";
    case TaskBase::OK:
        return "OK";
    case TaskBase::NG:
        return "NG";
    }
    return "Unknown";
}

}


TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{

}

TaskManager::~TaskManager()
{
    while (!m_tasks.isEmpty()) {
        TaskBase* task = m_tasks.dequeue();
        delete task;
    }
}

TaskBase* TaskManager::currentTask() const
{
    return m_tasks.isEmpty() ? nullptr : m_tasks.head();
}

void TaskManager::addTask(TaskBase *task)
{
    if (!task) {
        return;
    }

    if (task->taskId().isEmpty()) {
        task->setTaskId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    }

    if (task->thread() != this->thread()) {
        task->moveToThread(this->thread());
    }

    m_tasks.append(task);
    emit taskStatusChanged(task->taskId(),
                           task->name(),
                           taskStatusToText(task->status()),
                           task->currentStateName());
}

void TaskManager::removeTask(TaskBase *task)
{
    if (m_tasks.contains(task)) {
        const QString removedId = task->taskId();
        const QString removedName = task->name();
        m_tasks.removeOne(task);
        delete task;
        task = nullptr;

        emit taskFinished(removedId, removedName, "Removed");
    } else {
        emit logMessage("Task not found in TaskManager");
    }
}

void TaskManager::removeTaskById(const QString& taskId)
{
    TaskBase* foundTask = nullptr;

    for (int i = 0; i < m_tasks.size(); ++i) {
        TaskBase* task = m_tasks.at(i);
        if (task && task->taskId() == taskId) {
            foundTask = task;
            break;
        }
    }

    if (foundTask) {
        removeTask(foundTask);
        return;
    }
    emit logMessage(QString("Task id '%1' not found in TaskManager").arg(taskId));
}

void TaskManager::start()
{
    m_running = true;
    m_frameTimer.restart();
    emit logMessage("TaskManager started");
}

void TaskManager::stop()
{
    m_running = false;
    emit logMessage("TaskManager stopped");
}

void TaskManager::onFrameReady(const QImage &frame)
{
    if(!m_running)
        return;

    // 高帧率时限流，避免状态机被过快驱动
    if (m_frameTimer.isValid() && m_frameTimer.elapsed() < m_minFrameIntervalMs)
        return;
    m_frameTimer.restart();

    // 防止重入（长耗时识别导致上一帧未处理完）
    if (m_processingFrame)
        return;
    m_processingFrame = true;

    TaskBase* task = currentTask();
    if (task) {
        const QString taskId = task->taskId();
        const QString taskName = task->name();
        task->execute(frame);
        const QString runtimeMessage = task->takeRuntimeMessage();
        if (!runtimeMessage.isEmpty()) {
            emit logMessage(QString("Task '%1' %2").arg(taskName, runtimeMessage));
        }
        emit taskStatusChanged(taskId,
                               taskName,
                               taskStatusToText(task->status()),
                               task->currentStateName());
        if(task->status() == TaskBase::OK) {
            TaskBase* finishedTask = m_tasks.dequeue(); //任务完成，从队列中移除
            delete finishedTask;
            finishedTask = nullptr;

            emit taskFinished(taskId, taskName, "OK");
        } else if (task->status() == TaskBase::NG) {
            const QString failedId = task->taskId();
            const QString failedName = task->name();
            const QString failedState = task->currentStateName();
            const QString failedReason = task->failureReason();
            TaskBase* failedTask = m_tasks.dequeue();
            delete failedTask;
            failedTask = nullptr;

            QString failMessage = QString("Task '%1' failed").arg(failedName);
            if (!failedState.isEmpty()) {
                failMessage += QString(" in state '%1'").arg(failedState);
            }
            if (!failedReason.isEmpty()) {
                failMessage += QString(": %1").arg(failedReason);
            }

            emit logMessage(failMessage);
            emit taskFinished(failedId, failedName, "NG");
            if (TaskBase* nextTask = currentTask()) {
                emit logMessage(QString("Continue with next task '%1'").arg(nextTask->name()));
                emit taskStatusChanged(nextTask->taskId(),
                                       nextTask->name(),
                                       taskStatusToText(nextTask->status()),
                                       nextTask->currentStateName());
            }
        }
        
    }

    m_processingFrame = false;
}