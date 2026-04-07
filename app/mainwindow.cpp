#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QMetaObject>
#include <QFile>
#include <QMap>
#include <QResizeEvent>
#include <QTimer>
#include <QUuid>

#include "taskbase.h"
#include "TaskRegistry.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_adbConfig = std::make_shared<AdbConfig>();
    m_taskThread = new QThread(this);
    
    createUI();
    
    /* Toolbar */
    connect(ui->actionStart,&QAction::triggered,
            this,&MainWindow::onStart);

    connect(ui->actionStop,&QAction::triggered,
            this,&MainWindow::onStop);

    /* Screen Capture */
    m_capture = new ScreenCapture();
    m_capture->setConfig(m_adbConfig);
    m_capture->moveToThread(m_taskThread);
    connect(m_taskThread, &QThread::finished,
            m_capture, &QObject::deleteLater);
    connect(m_capture, &ScreenCapture::frameReady,this,
        [this](const QImage& img)->void {//更新当前帧并显示在界面上
            m_currentFrame = img;
            refreshEmulatorView();
        });
    connect(m_capture, &ScreenCapture::captureError,this,
        [this](const QString& msg)->void {
            appendLog(msg);
        });
        
    /* Device Controller */
    m_device = new DeviceController();
    m_device->setConfig(m_adbConfig);
    m_device->moveToThread(m_taskThread);
    connect(m_taskThread, &QThread::finished,
            m_device, &QObject::deleteLater);
    connect(m_device, &DeviceController::actionExecuted,this,
        [this](const QString& action)->void {
            appendLog(action);
        });
    connect(m_device, &DeviceController::actionError,this,
        [this](const QString& err)->void {
            appendLog(err);
        });

    /* Vision Engine */
    m_vision = new VisionEngine();
    m_vision->moveToThread(m_taskThread);
    connect(m_taskThread, &QThread::finished,
            m_vision, &QObject::deleteLater);

    /* Task Manager */
    m_taskManager = new TaskManager();
    m_taskManager->moveToThread(m_taskThread);
    connect(m_taskThread, &QThread::finished,
            m_taskManager, &QObject::deleteLater);
    connect(m_taskManager, &TaskManager::logMessage,this,
        [this](const QString& msg)->void {
            appendLog(msg);
        });
    connect(m_taskManager, &TaskManager::taskStatusChanged,
            this, &MainWindow::onTaskStatusChanged);
    connect(m_taskManager, &TaskManager::taskFinished,
            this, &MainWindow::onTaskFinished);

    /* 连接屏幕捕获器和任务管理器，使得每当有新帧时，任务管理器都能收到并处理 */
    connect(m_capture, &ScreenCapture::frameReady,
            m_taskManager, &TaskManager::onFrameReady,
            Qt::QueuedConnection);

    //启动任务管理器线程
    m_taskThread->start();

}

MainWindow::~MainWindow()
{
    if (m_taskManager) {
        QMetaObject::invokeMethod(m_taskManager,
                                  [this]() {
                                      m_taskManager->stop();
                                      m_capture->stop();
                                  },
                                  Qt::BlockingQueuedConnection);
    }
    if (m_taskThread) {
        m_taskThread->quit();
        m_taskThread->wait();
    }

    delete ui;
}

void MainWindow::createUI()
{
    m_taskTree = ui->taskTree;
    m_emulatorView = ui->emulatorView;
    m_taskStatus = ui->taskStatus;
    m_logEdit = ui->logEdit;

    this->setMinimumSize(1000, 680);
    this->resize(1248, 768);

    QFile qssFile(":/styles/main.qss");
    if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        this->setStyleSheet(QString::fromUtf8(qssFile.readAll()));
        qssFile.close();
    }

    ui->toolBar->setMovable(false);
    ui->mainSplitter->setStretchFactor(0, 0);
    ui->mainSplitter->setStretchFactor(1, 1);
    ui->mainSplitter->setStretchFactor(2, 0);
    ui->mainSplitter->setCollapsible(2, false);

    ui->vboxLayout->setStretch(0, 3);
    ui->vboxLayout->setStretch(1, 1);

    m_taskStatus->setMinimumWidth(360);

    /* 初始化任务树 */
    {
        QMap<QString, QTreeWidgetItem*> categoryItems;
        const QList<TaskDefinition>& taskDefinitions = TaskRegistry::definitions();

        for (const TaskDefinition& definition : taskDefinitions) {
            QTreeWidgetItem* categoryItem = categoryItems.value(definition.category, nullptr);
            if (!categoryItem) {
                categoryItem = new QTreeWidgetItem(m_taskTree);
                categoryItem->setText(0, definition.category);
                categoryItems.insert(definition.category, categoryItem);
            }

            QTreeWidgetItem* taskItem = new QTreeWidgetItem(categoryItem);
            taskItem->setText(0, definition.displayName);
            taskItem->setData(0, Qt::UserRole, definition.taskTypeName);
            taskItem->setData(0, Qt::UserRole + 1, definition.initialStateName);
        }
    }

    m_taskTree->expandAll();

    //连接任务树的点击事件
    connect(m_taskTree, &QTreeWidget::itemDoubleClicked,
        [this](QTreeWidgetItem* item, int column)->void {
            const QString displayName = item->text(0);
            const QString taskTypeName = item->data(0, Qt::UserRole).toString();
            const QString initialStateName = item->data(0, Qt::UserRole + 1).toString();
            appendLog(QString("Task '%1' clicked, column: %2").arg(displayName).arg(column));

            if (taskTypeName.isEmpty()) {
                appendLog(QString("No task associated with '%1'").arg(displayName));
                return;
            }

            const QString taskId = createTaskId(taskTypeName);

            QMetaObject::invokeMethod(m_taskManager,
                                      [this, taskTypeName, taskId]() {
                                          TaskBase* task = TaskRegistry::createTask(taskTypeName, m_vision, m_device);
                                          if (task) {
                                              task->setTaskId(taskId);
                                              m_taskManager->addTask(task);
                                          }
                                      },
                                      Qt::QueuedConnection);

            m_taskStatus->insertRow(m_taskStatus->rowCount());
            QTableWidgetItem* taskItem = new QTableWidgetItem(displayName);
            QTableWidgetItem* statusItem = new QTableWidgetItem("Idle");
            QTableWidgetItem* stateItem = new QTableWidgetItem(initialStateName);
            QTableWidgetItem* closeItem = new QTableWidgetItem("X");
            taskItem->setTextAlignment(Qt::AlignCenter);
            statusItem->setTextAlignment(Qt::AlignCenter);
            stateItem->setTextAlignment(Qt::AlignCenter);
            closeItem->setTextAlignment(Qt::AlignCenter);
            taskItem->setData(Qt::UserRole, taskId);
            taskItem->setData(Qt::UserRole + 1, taskTypeName);
            taskItem->setToolTip(taskId);
            m_taskStatus->setItem(m_taskStatus->rowCount()-1,0,taskItem);
            m_taskStatus->setItem(m_taskStatus->rowCount()-1,1,statusItem);
            m_taskStatus->setItem(m_taskStatus->rowCount()-1,2,stateItem);
            m_taskStatus->setItem(m_taskStatus->rowCount()-1,3,closeItem);
        });

    /* 初始化状态表 */
    m_taskStatus->setColumnCount(4);
    // 设置表头标签和对齐方式
    m_taskStatus->setHorizontalHeaderLabels({"Task", "Status", "State", "Close"});
    m_taskStatus->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_taskStatus->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_taskStatus->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_taskStatus->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_taskStatus->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_taskStatus->setColumnWidth(3, 67);
    
    m_taskStatus->verticalHeader()->setVisible(false);

    m_taskStatus->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskStatus->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_taskStatus->setSelectionMode(QAbstractItemView::SingleSelection);

    m_taskStatus->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_taskStatus->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    //双击状态表的关闭按钮，移除对应任务
    connect(m_taskStatus, &QTableWidget::itemDoubleClicked,
        [this](QTableWidgetItem* item)->void {
            int row = item->row();
            int col = item->column();
            QString cellText = item->text();
            appendLog(QString("Status table clicked: (%1, %2) '%3'").arg(row).arg(col).arg(cellText));
            //只有点击关闭按钮才触发移除任务
            if (col != 3) {
                appendLog("Only 'close' column is clickable");
                return;
            }
            //弹窗提示
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Remove Task", "Are you sure you want to remove this task?",
                                        QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No) {
                appendLog("Task removal cancelled");
                return;
            }
            //获取任务名称
            QTableWidgetItem* taskItem = m_taskStatus->item(row,0);
            const QString taskName = taskItem ? taskItem->text() : QString();
            const QString taskId = taskItem ? taskItem->data(Qt::UserRole).toString() : QString();
            QMetaObject::invokeMethod(m_taskManager,
                                      [this, taskId]() {
                                          m_taskManager->removeTaskById(taskId);
                                      },
                                      Qt::QueuedConnection);
            appendLog(QString("Task '%1' removed").arg(taskName));
            //从状态表中移除对应行
            m_taskStatus->removeRow(row);
        });

}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    refreshEmulatorView();
}

void MainWindow::onStart()
{
    appendLog("Connecting to device");
    appendLog("Start capture");
    appendLog("Task started");
    QMetaObject::invokeMethod(m_taskManager,
                              [this]() {
                                  m_device->disconnect();
                                  m_device->connect();
                                  m_capture->start(50);
                                  m_taskManager->start();
                              },
                              Qt::QueuedConnection);
    
}

void MainWindow::onStop()
{
    appendLog("Task stopped");
    appendLog("Stop capture");
    QMetaObject::invokeMethod(m_taskManager,
                              [this]() {
                                  m_taskManager->stop();
                                  m_capture->stop();
                              },
                              Qt::QueuedConnection);
}

void MainWindow::appendLog(const QString &msg)
{
    QString time = QDateTime::currentDateTime()
            .toString("hh:mm:ss.zzz");

    m_logEdit->append(QString("[%1] %2")
                      .arg(time)
                      .arg(msg));
}

QString MainWindow::createTaskId(const QString& taskTypeName) const
{
    return QString("%1-%2")
        .arg(taskTypeName,
             QUuid::createUuid().toString(QUuid::WithoutBraces));
}

void MainWindow::onTaskStatusChanged(const QString& taskId,
                                     const QString& taskName,
                                     const QString& statusText,
                                     const QString& stateName)
{
    Q_UNUSED(taskName);
    setTaskStatusRow(taskId, statusText, stateName);
}

void MainWindow::onTaskFinished(const QString& taskId,
                                const QString& taskName,
                                const QString& finalStatus)
{
    Q_UNUSED(taskName);
    setTaskStatusRow(taskId, finalStatus, QString());
    if (finalStatus == "Removed") {
        removeTaskStatusRow(taskId);
        return;
    }

    QTimer::singleShot(1200, this, [this, taskId]() {
        removeTaskStatusRow(taskId);
    });
}

int MainWindow::findTaskStatusRow(const QString& taskId) const
{
    for (int row = 0; row < m_taskStatus->rowCount(); ++row) {
        QTableWidgetItem* taskItem = m_taskStatus->item(row, 0);
        if (!taskItem) {
            continue;
        }

        const QString existingTaskId = taskItem->data(Qt::UserRole).toString();
        if (!existingTaskId.isEmpty() && existingTaskId == taskId) {
            return row;
        }
    }
    return -1;
}

void MainWindow::setTaskStatusRow(const QString& taskId,
                                  const QString& statusText,
                                  const QString& stateName)
{
    const int row = findTaskStatusRow(taskId);
    if (row < 0) {
        return;
    }

    QTableWidgetItem* statusItem = m_taskStatus->item(row, 1);
    if (!statusItem) {
        statusItem = new QTableWidgetItem();
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_taskStatus->setItem(row, 1, statusItem);
    }

    QTableWidgetItem* stateItem = m_taskStatus->item(row, 2);
    if (!stateItem) {
        stateItem = new QTableWidgetItem();
        stateItem->setTextAlignment(Qt::AlignCenter);
        m_taskStatus->setItem(row, 2, stateItem);
    }

    statusItem->setText(statusText);
    stateItem->setText(stateName);
    statusItem->setToolTip(stateName.isEmpty()
                           ? statusText
                           : QString("%1 / %2").arg(statusText, stateName));
    stateItem->setToolTip(stateName);
}

void MainWindow::removeTaskStatusRow(const QString& taskId)
{
    const int row = findTaskStatusRow(taskId);
    if (row >= 0) {
        m_taskStatus->removeRow(row);
    }
}

void MainWindow::refreshEmulatorView()
{
    if (m_currentFrame.isNull()) {
        return;
    }

    ui->emulatorView->setPixmap(QPixmap::fromImage(m_currentFrame)
        .scaled(ui->emulatorView->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
}
