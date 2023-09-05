#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QColorDialog>
#include <QDialog>
#include <QColor>
#include <QRgb>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QErrorMessage>
#include <QCoreApplication>
#include <QWizard>

#include<QTcpServer>
#include<QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void readMessage();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
};
#endif // MAINWINDOW_H