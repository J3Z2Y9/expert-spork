#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QTcpServer>
#include<QtSql/QSqlDatabase>
#include <QtNetwork/QTcpSocket>
#include <QDebug>
#include<QtWidgets/QListWidget>
#include<QtSql/QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include<qfile.h>
#include<qtimer.h>
#include<qiodevice.h>
#include<qmessagebox.h>
#include<qlist.h>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    bool checkIfAccountExists(const QString &account);
    bool checkLogin(const QString &account, const QString &password);
    void loadChatRecordsFromDatabase();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
   // QTcpSocket *clientConnection;可能是这行代码让我错了很久，删了试试
    //改成下面这行
    QList<QTcpSocket*> clients;
    QList<QTcpSocket*> peer_file_socket_list;//--

    QHash<QString, QTcpSocket*> accountToSocket;
    QHash<QTcpSocket*, QString> socketToAccount;
    qint64 bytesReceived=0;//--
    qint64 totalBytes;  //存放总大小信息--

    //-----------
    QFile file;
    QString filename;
    qint64 send_size;
    qint64 receive_size;
    QTcpSocket*filesocket;
    QTcpServer*fileserver;
    qint64 filesize;
    QString file_receiver;
    QHash<QString, QTcpSocket*> filie_accountToSocket;
    QHash<QTcpSocket*, QString> file_socketToAccount;



    //-------------

private slots:
    void connected();
    void readMessage();
    void clientDisconnected();

};
#endif // MAINWINDOW_H
