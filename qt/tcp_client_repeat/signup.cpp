#include "signup.h"
#include "ui_signup.h"
#include <QtDebug>
#include<QMessageBox>
#include<QThread>

extern QString ip;
extern int port;

Signup::Signup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Signup)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));

    ip = "127.0.0.1";
    port = 6666;
}

Signup::~Signup()
{
    delete ui;
}

void Signup::on_pushButton_clicked()
{
    QString name = ui->lineEdit->text();
    QString account = ui->lineEdit_2->text();
    QString password = ui->lineEdit_3->text();
    QString confirmPassword = ui->lineEdit_4->text();

    if (name.isEmpty() || account.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::critical(this, "Error", "All fields must be filled.");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::critical(this, "Error", "Passwords do not match.");
        return;
    }

    qDebug() << "Sending registration request...";
    QString message = QString("REGISTER|%1|%2|%3").arg(name).arg(account).arg(password);
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        tcpSocket->connectToHost(ip, port);
        qDebug() << "Connecting to server...";
        qDebug() <<message;
    }
    tcpSocket->write(message.toUtf8().data());
    qDebug()<<"send finished";
}


void Signup::readMessage()
{
    QByteArray data = tcpSocket->readAll();
    QString responseMessage = QString::fromUtf8(data);
    qDebug() << "Received from server:" << data;
    if (responseMessage == "REGISTER_SUCCESS:Account registered successfully.")
    {
        QMessageBox::information(this, "注册成功", "恭喜您，注册成功！");
        this->close();
    }
    else if (responseMessage == "REGISTER_FAIL:Account already exists.")
    {
        QMessageBox::critical(this, "注册失败", "很抱歉，该账户已存在，请重新选择账号。");
        // You can clear the input fields or perform other actions here
    }
    else
    {
        qDebug() << "Received unknown response message: " << responseMessage;
    }
}

