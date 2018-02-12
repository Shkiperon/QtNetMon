#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    nameOS=QSysInfo::kernelType();
    bool isWIN32=false;
    if(nameOS==QString("winnt"))
    {
        nameOS="Windows NT";
        isWIN32=true;
    }
    else
    {
        if (nameOS==QString("linux"))
        {
            nameOS="Linux";
        }
        else
        {
            if(nameOS==QString("freebsd"))
            {
                nameOS="FreeBSD";
            }
            else
            {
                nameOS="Other OS";
            }
        }
    }
    if(isWIN32==true)
    {
        pingParam<<"-n"<<"1"<<"-w"<<"1000";
    }
    else
    {
        pingParam<<"-c"<<"1"<<"-w"<<"1";
    }

    ui->setupUi(this);
    this->setWindowTitle("QtNetMon for "+nameOS);

    defConfig=QApplication::applicationDirPath()+"/default.ini";
    getSettings(defConfig);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pingByTimer()
{
    qRegisterMetaType<QVector<int> >("QVector<int>");
    QThread *timerThread = new QThread(0);
    QTimer *timer = new QTimer(0);
    timer->setInterval(1000*timerSecs);
    timer->moveToThread(timerThread);
    connect(timer, &QTimer::timeout, this, &MainWindow::updNodesStatus, Qt::DirectConnection);
    timer->connect(timerThread, SIGNAL(started()), SLOT(start()));
    connect(timerThread, &QThread::started, this, &MainWindow::updNodesStatus, Qt::DirectConnection);
    connect(this, &MainWindow::sgnFinishUpd, timerThread, &QThread::quit);
    timerThread->start();
}

void MainWindow::slotAddGroupItem(QString nameForBtn, int numOfNodes)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(nameForBtn.replace(QRegExp("(\\|).*"),""));
    item->setToolTip("0/"+QString::number(numOfNodes));
    item->setBackground(Qt::red);
    ui->listWidget->addItem(item);
}

void MainWindow::slotAddNodeItem(QString nodeName, int nodeStatus)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(nodeName);
    if(nodeStatus==1)
    {
        item->setBackground(Qt::green);
    }
    else
    {
        item->setBackground(Qt::red);
    }
    ui->listWidget_2->addItem(item);
}

void MainWindow::slotShowGroup()
{
    ui->listWidget_2->clear();
    int vectID = ui->listWidget->currentIndex().row();
    for(int i=0;i<nodeParams[vectID].size();++i)
    {
        slotAddNodeItem(nodeParams[vectID][i][0], nodeParams[vectID][i][2].toInt());
    }
}

void MainWindow::updNodesStatus()
{
    int exitCode=0,i,j,zall,zcur;
    for(i=0; i != nodeParams.size(); ++i)
    {
        ui->listWidget->item(i)->setBackground(Qt::blue);
        zall=nodeParams[i].size();
        zcur=0;
        QStringList *dataOfVector=nodeParams[i].data();
        for(j=0; j != nodeParams[i].size(); ++j)
        {
            exitCode = QProcess::execute("ping", QStringList() << pingParam << nodeParams[i][j][1]);
            if (exitCode==0)
            {
                dataOfVector[j][2]="1";
                ++zcur;
            }
            else
            {
                dataOfVector[j][2]="0";
            }
        }
        ui->listWidget->item(i)->setToolTip(QString::number(zcur)+"/"+QString::number(zall));
        if(zcur != 0)
        {
            if(zall==zcur)
            {
                ui->listWidget->item(i)->setBackground(Qt::green);
            }
            else
            {
                ui->listWidget->item(i)->setBackground(Qt::yellow);
            }
        }
        else
        {
            ui->listWidget->item(i)->setBackground(Qt::red);
        }
    }
}

void MainWindow::sorter(QStringList listForSort, QStringList &listForResult)
{
    QVector<QString> sortVector = QVector<QString>::fromList(listForSort);
    QVector<QString> sortVectorFiltered;
    int i,rez=0;
    bool ok;
    if(sortVector.size()!=1)
    {
        for(i=0; i<sortVector.size(); ++i)
        {
            if(sortVector[i].section("|",1,1).toInt(&ok,10))
            {
                sortVectorFiltered.append(sortVector[i]);
                sortVector.remove(i);
                --i;
            }
        }
        if(sortVectorFiltered.size() > 1)
        {
            for(i=1; i<sortVectorFiltered.size(); ++i)
            {
                rez=sortVectorFiltered[i].section("|",1,1).toInt(&ok,10) - sortVectorFiltered[i-1].section("|",1,1).toInt(&ok,10);
                if(rez<0)
                {
                    sortVectorFiltered.insert(i-1,sortVectorFiltered[i]);
                    sortVectorFiltered.remove(i+1);
                    i=0;
                }
            }
        }
        else
        {
            if(sortVectorFiltered.size() == 1)
            {
                sortVector.prepend(sortVectorFiltered[0]);
                sortVectorFiltered.remove(0);
            }
        }
        for(i=0; i<sortVectorFiltered.size(); ++i)
        {
            sortVector.insert(i,sortVectorFiltered[i]);
        }
        sortVectorFiltered.clear();
        sortVectorFiltered.squeeze();
    }
    listForResult = sortVector.toList();
    sortVector.clear();
    sortVector.squeeze();
}

void MainWindow::getSettings(QString filename)
{
    if(QFile::exists(filename)==true)
    {
        QSettings *settings = new QSettings(filename,QSettings::IniFormat);
        if(settings->contains("title"))
        {
            this->setWindowTitle(settings->value("title").toString());
        }
        sorter(settings->childGroups(),hostgroups);
        nodeParams.resize(hostgroups.size());
        int i,j;
        for(i=0; i<hostgroups.size(); ++i)
        {
            settings->beginGroup(hostgroups[i]);
            hostnodes = settings->childKeys();
            slotAddGroupItem(hostgroups[i],hostnodes.size());
            nodeParams[i].resize(hostnodes.size());
            for(j=0; j<hostnodes.size(); ++j)
            {
                nodeParams[i][j] << hostnodes[j];
                nodeParams[i][j] << settings->value(hostnodes[j]).toString();
                nodeParams[i][j] << "0";
            }
            settings->endGroup();
        }
        connect(ui->listWidget,&QListWidget::itemClicked,this,&MainWindow::slotShowGroup);
        if((timerSecs=settings->value("timer").toInt())==0)
        {
            timerSecs=30;
        }
        pingByTimer();
    }
    else
    {
        QMessageBox::information(this,"Внимание", "Не удалось найти файл default.ini. Необходимо загрузить конфигурационный файл.");
    }
}

void MainWindow::cleanLists()
{
    ui->listWidget_2->clear();
    ui->listWidget->clear();
}

void MainWindow::openConfFile()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Выбор конфигурационного файла"),QApplication::applicationDirPath(),tr("INI files (*.ini)"));
    if(filename != "")
    {
        emit(sgnFinishUpd());
        cleanLists();
        getSettings(filename);
    }
}

void MainWindow::aboutMyProgram()
{
    QMessageBox::information(this,"О программе","<p align='center'>Версия 1.0<br>Автор: Артем Друзь (aka Shkiperon)<br><br>Идея программы позаимствована у Дмитрия Бачило (aka Bocha)</p>");
}
