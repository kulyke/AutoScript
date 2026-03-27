#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
// #include <QList>
#include <QQueue>
#include <QImage>

class TaskBase;
/**
 * @brief 任务管理器，负责管理和调度所有任务
 * 
 */
class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    /**
     * @brief 添加任务
     * @param task 任务指针
     */
    void addTask(TaskBase* task);
    /**
     * @brief 移除任务
     * @param task 任务指针
     */
    void removeTask(TaskBase* task);
    /**
     * @brief 获取所有任务
     * @return 任务列表
     */
    QList<TaskBase*> tasks() const { return m_tasks; }
    /**
     * @brief 设置当前任务
     * @param task 任务指针
     */
    void setCurrentTask(TaskBase* task) { m_currentTask = task; }
    /**
     * @brief 获取当前正在执行的任务
     * @return 当前任务指针
     */
    TaskBase* currentTask() const { return m_currentTask; }
    /**
     * @brief 启动所有任务
     */
    void start();
    /**
     * @brief 停止所有任务
     */
    void stop();

public slots:
    void onFrameReady(const QImage& frame);

signals:
    void logMessage(const QString& msg);

private:
    QList<TaskBase*> m_idleTasks;//空闲任务列表
    // QList<TaskBase*> m_tasks;//正在执行&等待的任务列表
    QQueue<TaskBase*> m_tasks;//任务队列，按照优先级排序
    bool m_running = false;
    TaskBase* m_currentTask = nullptr; //当前正在执行的任务

};

#endif