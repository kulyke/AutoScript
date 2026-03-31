#include "taskmanager.h"
#include "taskbase.h"
#include <QThread>


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
    m_currentTask = nullptr;
}

void TaskManager::addTask(TaskBase *task)
{
    if (!task) {
        return;
    }

    if (task->thread() != this->thread()) {
        task->moveToThread(this->thread());
    }

    m_tasks.append(task);
    //更新当前任务
    m_currentTask = m_tasks.isEmpty() ? nullptr : m_tasks.head();
}

void TaskManager::removeTask(TaskBase *task)
{
    if (m_tasks.contains(task)) {
        const bool wasCurrent = (task == m_currentTask);
        m_tasks.removeOne(task);
        delete task;
        task = nullptr;

        //更新当前任务
        if (wasCurrent) {
            m_currentTask = m_tasks.isEmpty() ? nullptr : m_tasks.head();
        }
    } else {
        emit logMessage("Task not found in TaskManager");
    }
}

void TaskManager::removeTaskByName(const QString& taskName)
{
    TaskBase* foundTask = nullptr;

    for (int i = 0; i < m_tasks.size(); ++i) {
        TaskBase* task = m_tasks.at(i);
        if (task && task->name().contains(taskName)) {
            foundTask = task;
            break;
        }
    }

    if (foundTask) {
        removeTask(foundTask);
        return;
    }
    emit logMessage(QString("Task '%1' not found in TaskManager").arg(taskName));
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

    if (m_currentTask) {
        m_currentTask->execute(frame);
        if(m_currentTask->status() == TaskBase::OK) {
            TaskBase* task = m_tasks.dequeue(); //任务完成，从队列中移除
            delete task;
            task = nullptr;

            //更新当前任务 
            m_currentTask = m_tasks.isEmpty() ? nullptr : m_tasks.head();
        } else if (m_currentTask->status() == TaskBase::NG) {
            const QString failedName = m_currentTask->name();
            TaskBase* failedTask = m_tasks.dequeue();
            delete failedTask;
            failedTask = nullptr;

            m_currentTask = m_tasks.isEmpty() ? nullptr : m_tasks.head();

            emit logMessage(QString("Task '%1' failed").arg(failedName));
            //任务失败，停止整个任务管理器
            stop();
        }
        
    }

    m_processingFrame = false;
}