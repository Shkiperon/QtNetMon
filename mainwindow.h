#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QProcess>
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int timerSecs=0;
    QString defConfig;
    QString nameOS;
    QStringList pingParam;
    QStringList hostgroups = QStringList()<<"";
    QStringList hostnodes;
    QVector < QVector < QStringList > > nodeParams;
    void sorter(QStringList listForSort, QStringList &listForResult);

signals:
    void sgnFinishUpd();

private slots:
    void slotAddGroupItem(QString nameForBtn, int numOfNodes);
    void slotAddNodeItem(QString nodeName, int nodeStatus);
    void slotShowGroup();
    void getSettings(QString filename);
    void openConfFile();
    void aboutMyProgram();
    void updNodesStatus();
    void pingByTimer();
    void cleanLists();
};

#endif // MAINWINDOW_H
