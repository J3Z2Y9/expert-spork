#include "clientmaininterface.h"
#include "ui_clientmaininterface.h"
#include<QListWidget>
#include<QListWidgetItem>


ClientMainInterface::ClientMainInterface(QString Account,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientMainInterface),account(Account)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(haveConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
    tcpSocket->connectToHost("127.0.0.1", 6666); // 修改为服务器地址和端口
}

// 处理从服务器接收的数据
void ClientMainInterface::readMessage() {
    QByteArray data = tcpSocket->readAll();
    QString message = QString::fromUtf8(data);

    // 处理从服务器接收的不同类型的消息
    if (message.startsWith("FRIENDS|")) {
        QStringList friends = message.split("|");
        friends.removeFirst(); // 移除消息类型标识
        loadFriendsList(friends);
    } else {
        // 处理其他类型的消息
    }
}

// 在连接成功后发送账号信息给服务器
void ClientMainInterface::haveConnected() {
    QString message = "LOGEDIN|" + account;
    sendMessage(message);
    qDebug()<<"account message:"<<message;
}


// 发送消息给服务器
void ClientMainInterface::sendMessage(const QString &message) {
    tcpSocket->write(message.toUtf8());
}

// 加载好友列表到界面
void ClientMainInterface::loadFriendsList(const QStringList &friends) {
    ui->listWidget->clear();
    foreach (const QString &friendInfo, friends) {
        QStringList friendData = friendInfo.split(",");
        if (friendData.size() >= 3) {
            QString friendName = friendData[0];
            QString group = friendData[1];
            // 你可以根据需要添加更多的信息，例如在线状态等
            QString displayText = QString("[%1] %2 (%3)").arg(group).arg(friendName).arg(friendData[2]);
            QListWidgetItem *item = new QListWidgetItem(displayText);
            ui->listWidget->addItem(item);
        }
    }
}
ClientMainInterface::~ClientMainInterface()
{
    delete ui;
}

