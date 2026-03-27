#include "taskmanager.h"
#include "taskbase.h"


TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{

}

void TaskManager::addTask(TaskBase *task)
{
    m_tasks.append(task);
    //更新当前任务
    m_currentTask = m_tasks.head();
}

void TaskManager::removeTask(TaskBase *task)
{
    if (m_tasks.contains(task)) {
        m_tasks.removeOne(task);
        delete task;
        task = nullptr;

        //更新当前任务
        m_currentTask = m_tasks.head();
    } else {
        emit logMessage("Task not found in TaskManager");
    }
}

void TaskManager::start()
{
    m_running = true;
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

    if (m_currentTask) {
        m_currentTask->setStatus(TaskBase::Running);//设置当前任务处于运行状态
        m_currentTask->execute(frame);
        if(m_currentTask->status() == TaskBase::OK) {
            TaskBase* task = m_tasks.dequeue(); //任务完成，从队列中移除
            delete task;
            task = nullptr;

            //更新当前任务 
            m_currentTask = m_tasks.head();
        }
        
    }
}