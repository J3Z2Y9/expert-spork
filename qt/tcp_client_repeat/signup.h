#ifndef SIGNUP_H
#define SIGNUP_H

#include <QDialog>
#include<QTcpSocket>
#include<QPainter>
#include<QPixmap>
namespace Ui {
class Signup;
}

class Signup : public QDialog
{
    Q_OBJECT

public:
    explicit Signup(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *) override
           {
               QPixmap pixmap(":/C:/Users/99510/Desktop/pic/login.png"); // 图片文件在项目资源中的路径
               QPainter painter(this);
               painter.drawPixmap(0, 0, width(), height(), pixmap);
           };

    ~Signup();

private slots:
    void on_pushButton_clicked();
    void readMessage();

private:
    Ui::Signup *ui;
    QTcpSocket *tcpSocket;
};

#endif // SIGNUP_H
