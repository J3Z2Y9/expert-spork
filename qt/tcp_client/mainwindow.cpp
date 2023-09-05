#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"signup.h"
#include"clientmaininterface.h"
QString ip;
int port;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);
    //connect(tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    Signup *registerwindow= new Signup;
    registerwindow->show();
}


void MainWindow::on_pushButton_2_clicked()
{
    QString account = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();

    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::critical(this, "Error", "Both fields must be filled.");
        return;
    }

    QString message = QString("LOGIN|%1|%2").arg(account).arg(password);
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug()<<"Disconnected.Trying to connect.";
        tcpSocket->connectToHost("127.0.0.1", 6666);
    }
    tcpSocket->write(message.toUtf8());
    qDebug()<<"have sent:"<<message;
}

void MainWindow::readMessage()
{
    QByteArray data = tcpSocket->readAll();
    QString responseMessage(data);

    if (responseMessage.startsWith("LOGIN_SUCCESS|")) {
        qDebug()<<"Login successful.";
        QString account = responseMessage.mid(QString("LOGIN_SUCCESS|").length());

        QMessageBox::information(this, "Success", "Login successful.");

        ClientMainInterface *client= new ClientMainInterface(account);
        client->show();

    } else if (responseMessage == "LOGIN_FAIL") {
        QMessageBox::critical(this, "Error", "Login failed. Please check your account and password.");
    } else {
        qDebug() << "Received unknown response message: " << responseMessage;
    }
}
