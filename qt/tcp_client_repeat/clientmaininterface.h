#ifndef CLIENTMAININTERFACE_H
#define CLIENTMAININTERFACE_H
#include <QItemSelectionModel>
#include <QDialog>
#include<QTcpSocket>
#include<QListWidget>
#include<QListWidgetItem>
#include<QMessageBox>
#include <QStandardItemModel>
#include<QMenu>
#include <QInputDialog>
#include<QStringListModel>
#include <QWidget>
#include <QVBoxLayout>
#include<QPainter>
#include<QPixmap>
#include<qfile.h>
#include<qmessagebox.h>
#include<qtimer.h>
#include<qhostaddress.h>
#include<qtcpserver.h>
namespace Ui {
class ClientMainInterface;
}

class ClientMainInterface : public QDialog
{
    Q_OBJECT

public:
    explicit ClientMainInterface(QString Account,QWidget *parent = nullptr);
    ~ClientMainInterface();
    void sendMessage(const QString &message);
//    void loadFriendsList(const QStringList &friends);
    void init(QString &message);
    void init_group_talking(QString &message);

    void paintEvent(QPaintEvent *) override
           {
               QPixmap pixmap(":/C:/Users/99510/Desktop/pic/chat.png"); // 图片文件在项目资源中的路径
               QPainter painter(this);
               painter.drawPixmap(0, 0, width(), height(), pixmap);
           };
    void sendfile();//--
private slots:
    void readMessage();
    void haveConnected();

    void slotMenuPopup(const QPoint &pos);

    void slotMenuPopup_group(const QPoint &pos);

    void addGroup(QStandardItemModel  *model);


    void moveMemberToGroup(const QModelIndex &memberIndex, const QModelIndex &groupIndex,QStandardItemModel  *model);

    void DropGroup(const QModelIndex &groupIndex,QStandardItemModel  *model);

    void Dropfriend(const QModelIndex &memberIndex,QStandardItemModel  *model);

    void on_pushButton_clicked();




    void on_pushButton_2_clicked();


    void on_treeView_clicked(const QModelIndex &index);

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_treeView_2_clicked(const QModelIndex &index);

    void on_tabWidget_tabBarClicked(int index);

    void on_pushButton_6_clicked();

private:
    Ui::ClientMainInterface *ui;
    QTcpSocket *tcpSocket;
    QString account;
    QStandardItem* item_record;

    QStandardItemModel  *m_pModel; //treeview

    QStandardItemModel  *m_pModel_group; //treeview_2

    QHash<QStandardItem*, QListWidget*> itemToListWidget;
    QHash<QStandardItem*, QListWidget*> itemToListWidget_group;


    //-----------
    QFile file;
    QString filename;
    qint64 send_size;
    qint64 receive_size;
    QTcpSocket*filesocket;
    //QTcpServer*fileserver;
    qint64 filesize;
    QTimer timer;
    QTcpServer* file_tcpserver;
    //-------------
};

#endif // CLIENTMAININTERFACE_H
