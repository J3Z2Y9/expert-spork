#ifndef CLIENTMAININTERFACE_H
#define CLIENTMAININTERFACE_H

#include <QDialog>
#include<QTcpSocket>
#include<QListWidget>
#include<QListWidgetItem>

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
    void loadFriendsList(const QStringList &friends);

private slots:
    void readMessage();
    void haveConnected();

private:
    Ui::ClientMainInterface *ui;
    QTcpSocket *tcpSocket;
    QString account;
};

#endif // CLIENTMAININTERFACE_H
