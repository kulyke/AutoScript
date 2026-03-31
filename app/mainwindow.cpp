#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>

#include "shoptask.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    m_taskTree = ui->taskTree;
    m_emulatorView = ui->emulatorView;
    m_taskStatus = ui->taskStatus;
    m_logEdit = ui->logEdit;
    
    createUI();

    /* Toolbar */
    connect(ui->actionStart,&QAction::triggered,
            this,&MainWindow::onStart);

    connect(ui->actionStop,&QAction::triggered,
            this,&MainWindow::onStop);

    m_adbConfig = std::make_shared<AdbConfig>();

    /* Screen Capture */
    m_capture = new ScreenCapture(this);
    m_capture->setConfig(m_adbConfig);
    connect(m_capture, &ScreenCapture::frameReady,this,
        [this](const QImage& img)->void {//更新当前帧并显示在界面上
            m_currentFrame = img;
            ui->emulatorView->setPixmap(QPixmap::fromImage(img)
                .scaled(ui->emulatorView->size(),
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation));
        });
    connect(m_capture, &ScreenCapture::captureError,this,
        [this](const QString& msg)->void {
            appendLog(msg);
        });
        
    /* Device Controller */
    m_device = new DeviceController(this);
    m_device->setConfig(m_adbConfig);
    connect(m_device, &DeviceController::actionExecuted,this,
        [this](const QString& action)->void {
            appendLog(action);
        });
    connect(m_device, &DeviceController::actionError,this,
        [this](const QString& err)->void {
            appendLog(err);
        });

    /* Vision Engine */
    m_vision = new VisionEngine(this);

    /* Task Manager */
    m_taskManager = new TaskManager(this);
    connect(m_taskManager, &TaskManager::logMessage,this,
        [this](const QString& msg)->void {
            appendLog(msg);
        });

    /* 连接屏幕捕获器和任务管理器，使得每当有新帧时，任务管理器都能收到并处理 */
    connect(m_capture, &ScreenCapture::frameReady,
            m_taskManager, &TaskManager::onFrameReady);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createUI()
{
    this->setFixedSize(1248, 768);

    ui->vboxLayout->setStretch(0, 3);
    ui->vboxLayout->setStretch(0, 1);

    /* 初始化任务树 */
    //活动
    {
        QTreeWidgetItem* campaign = new QTreeWidgetItem(m_taskTree);
        campaign->setText(0,"Campaign");

        QTreeWidgetItem* c1 = new QTreeWidgetItem(campaign);
        c1->setText(0,"12-4 Farm");
    }
    //日常(委托，科研，商店)
    {
        QTreeWidgetItem* daily = new QTreeWidgetItem(m_taskTree);
        daily->setText(0,"Daily");

        //委托
        QTreeWidgetItem* d1 = new QTreeWidgetItem(daily);
        d1->setText(0,"Commission");
        //科研
        QTreeWidgetItem* d2 = new QTreeWidgetItem(daily);
        d2->setText(0,"Research");
        //商店
        QTreeWidgetItem* d3 = new QTreeWidgetItem(daily);
        d3->setText(0,"Shop");
    }

    m_taskTree->expandAll();

    //连接任务树的点击事件
    connect(m_taskTree, &QTreeWidget::itemDoubleClicked,
        [this](QTreeWidgetItem* item, int column)->void {
            QString taskName = item->text(0);
            appendLog(QString("Task '%1' clicked, column: %2").arg(taskName).arg(column));
            //双击为任务列表添加任务
            if(taskName == "Shop") {
                bool flag = true;
                for (int i = 0; i < m_taskStatus->rowCount(); i++) {
                    QTableWidgetItem* item = m_taskStatus->item(i,0);
                    if (item->text() == taskName) {//判定当前的任务列表中是否存在重复的任务
                        flag = false;
                        break;
                    }
                }
                if (!flag) {
                    appendLog(QString("Task '%1' already exists").arg(taskName));
                    return;
                }
                //创建一个商店任务并添加到任务管理器
                ShopTask* task = new ShopTask();
                TaskState* state = new StMainMenuToShop(m_vision,m_device,task);
                task->setInitialState(state);
                m_taskManager->addTask(task);

                //更新table_taskStatus显示
                m_taskStatus->insertRow(m_taskStatus->rowCount());
                m_taskStatus->setItem(m_taskStatus->rowCount()-1,0,new QTableWidgetItem(taskName));
                m_taskStatus->setItem(m_taskStatus->rowCount()-1,1,new QTableWidgetItem("Idle"));
                m_taskStatus->setItem(m_taskStatus->rowCount()-1,2,new QTableWidgetItem("X"));

            } else {
                appendLog(QString("No task associated with '%1'").arg(taskName));
            }
        });

    /* 初始化状态表 */
    m_taskStatus->setColumnCount(3);
    m_taskStatus->setHorizontalHeaderLabels({"Task", "Status", "close"});
    // 设置列宽自适应内容
    m_taskStatus->horizontalHeader()->setStretchLastSection(true);
    m_taskStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

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
            if (col!=2) {
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
            QString taskName = m_taskStatus->item(row,0)->text();
            //找到任务管理器中对应的任务并移除
            for(TaskBase* task : m_taskManager->tasks()) {
                //"ShopTask"->"Shop"
                if(task->name().contains(taskName)) {
                    m_taskManager->removeTask(task);
                    appendLog(QString("Task '%1' removed").arg(taskName));
                    break;
                }
            }
            //从状态表中移除对应行
            m_taskStatus->removeRow(row);
        });

}

void MainWindow::onStart()
{
    appendLog("Connecting to device");
    m_device->disconnect();
    m_device->connect();

    appendLog("Start capture");
    m_capture->start(200);

    // appendLog("Task started");
    // m_taskManager->start();

    //测试模拟器点击
    // m_device->tap(500,500);

    // 首帧到达后再执行模板匹配，避免 m_currentFrame 为空
    QMetaObject::Connection* oneShotConn = new QMetaObject::Connection;
    *oneShotConn = connect(m_capture, &ScreenCapture::frameReady,
                           this,
                           [this, oneShotConn](const QImage& img)->void {
        QObject::disconnect(*oneShotConn);
        delete oneShotConn;

        appendLog("Find shop button");
        QPoint pt;
        bool found = m_vision->findTemplate(
                    img,
                    "resources/templates/shop.png",
                    pt,
                    0.9
                    );
        if(found) {
            appendLog(QString("Shop button found (%1,%2)").arg(pt.x()).arg(pt.y()));
            m_device->tap(pt.x(),pt.y());
        }
        else {
            appendLog("Shop button not found");
        }
    });
    
}

void MainWindow::onStop()
{
    // appendLog("Task stopped");
    // m_taskManager->stop();

    appendLog("Stop capture");
    m_capture->stop();
}

void MainWindow::appendLog(const QString &msg)
{
    QString time = QDateTime::currentDateTime()
            .toString("hh:mm:ss");

    m_logEdit->append(QString("[%1] %2")
                      .arg(time)
                      .arg(msg));
}