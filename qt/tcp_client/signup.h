#ifndef SIGNUP_H
#define SIGNUP_H

#include <QDialog>
#include<QTcpSocket>

namespace Ui {
class Signup;
}

class Signup : public QDialog
{
    Q_OBJECT

public:
    explicit Signup(QWidget *parent = nullptr);
    ~Signup();

private slots:
    void on_pushButton_clicked();
    void readMessage();

private:
    Ui::Signup *ui;
    QTcpSocket *tcpSocket;
};

#endif // SIGNUP_H
