#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QTcpSocket>
#include<QDateTime>
#include<QFile>
#include<QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //连接MySQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1"); // MySQL服务器的IP地址
    db.setDatabaseName("new_schema"); // 数据库名称
    db.setUserName("jzy"); // 数据库用户名
    db.setPassword("jzy030329"); // 数据库密码
    if (!db.open()) {
        qDebug() << "Database connection failed.";
        return;
    }
    else qDebug()<< "Database have connected.";

    loadChatRecordsFromDatabase();
    //开启tcp_server
    server = new QTcpServer(this);
    if(!server->listen(QHostAddress::LocalHost, 6666))
    {  //本地主机的6666端口，如果出错就输出错误信息，并关闭
        qDebug() << server->errorString();
        close();
    }
//------------------
    fileserver=new QTcpServer(this);
    if(!fileserver->listen(QHostAddress::LocalHost, 10000))
    {  //本地主机10000端口，如果出错就输出错误信息，并关闭
        qDebug() << fileserver->errorString();
        close();
    }
//----------------------

    // 连接信号和相应槽函数，新链接信号与槽
    connect(server,SIGNAL(newConnection()), this, SLOT(connected()));

    //--------------------------
    connect(fileserver,&QTcpServer::newConnection, [=]()
    {
        qDebug()<<"new connection";
        filesocket=fileserver->nextPendingConnection();

        bool isstart=true;
        connect(filesocket,&QTcpSocket::readyRead,[=]() mutable
        {
            QByteArray array=filesocket->readAll();
            if(isstart)
            {
                //解析头部信息
                filename=QString(array).section("##",0,0);
                filesize=QString(array).section("##",1,1).toInt();
                //socketToAccount.value()
                receive_size=0;
                isstart=false;
                qDebug()<<QString(array);
                qDebug()<<receive_size<<"  "<<filesize<<"  "<<filename;

//                ui->progressBar->setVisible(true);//设置进度条
//                ui->progressBar->setValue(0);
//                ui->progressBar->setMaximu m(filesize);

                //打开文件
                file.setFileName(filename);
                if(!file.open(QIODevice::WriteOnly))
                {
                    qDebug()<<"writeonly error";
                }

            }
            else//文件信息
            {
                 //qDebug()<<"wenjianneirong";
                qint64 len=file.write(array);
                receive_size+=len;
                //ui->progressBar->setValue(receive_size);
                if(receive_size==filesize)
                {
                    file.close();
                    QMessageBox::information(this,"完成","文件接收完成");
                    isstart=true;//坑死我了，不能连续发两次文件
                    //ui->progressBar->setVisible(false);
                }
            }

        });
    });//--
}
void MainWindow::clientDisconnected()
{
    QTcpSocket *disconnectedSocket = qobject_cast<QTcpSocket*>(sender());
   //if (disconnectedSocket && clientIPAccountMap.contains(disconnectedSocket)) {
    if(disconnectedSocket && socketToAccount.contains(disconnectedSocket)){
        //QString disconnectedAccount = clientIPAccountMap.value(disconnectedSocket);
        QString disconnectedAccount=socketToAccount.value(disconnectedSocket);
        qDebug() << "Client" << disconnectedAccount << "disconnected";
      //  clientIPAccountMap.remove(disconnectedSocket);
        socketToAccount.remove(disconnectedSocket);
        accountToSocket.remove(disconnectedAccount);
        disconnectedSocket->deleteLater();
    }
    if (disconnectedSocket) {
        clients.removeOne(disconnectedSocket);
        disconnectedSocket->deleteLater();
    }
}
void MainWindow::connected()
{
        // 获取已经建立的连接的子套接字（socket）
        QTcpSocket *clientConnection = server->nextPendingConnection();
        clients.append(clientConnection);
        // 获取对方的端口号和ip地址，并且显示在文本编辑框中。
        qDebug() << QString("[%1]用户使用%2端口连接成功").arg(clientConnection->peerAddress().toString()).arg(clientConnection->peerPort());
        ui->listWidget->addItem(QString("[%1]:用户使用%2端口连接成功").arg(clientConnection->peerAddress().toString()).arg(clientConnection->peerPort()));
        // 设置接受数据信号与槽
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readMessage()));

        // 设置连接断开信号与槽
        connect(clientConnection, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    //connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));


        // 发送初始数据给客户端
        QString message = "This is server's message.";
        clientConnection->write(message.toUtf8().data());
}


bool MainWindow::checkIfAccountExists(const QString &account)
{
    QSqlQuery query;
    query.prepare("SELECT account FROM qq_account WHERE account = ?");
    query.addBindValue(account);
    if (!query.exec()) {
        qDebug() << "Failed to execute query:" << query.lastError().text();
        return false; // 返回 false 表示查询失败
    }

    // 如果查询结果不为空，表示账户已存在
    if (query.next()) {
        qDebug() << "Account already exists:" << account;
        return true;
    } else {
        qDebug() << "Account does not exist:" << account;
        return false;
    }
}

bool MainWindow::checkLogin(const QString &account, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM qq_account WHERE account = ?");
    query.addBindValue(account);
    if (!query.exec()) {
        qDebug() << "Failed to execute query:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        QString storedPassword = query.value(0).toString();
        return (password == storedPassword);
    } else {
        return false;
    }
}

void MainWindow::readMessage()
{
    QTcpSocket *clientConnection = qobject_cast<QTcpSocket*>(sender());
    if(!(clientConnection->bytesAvailable()>0)){
        qDebug()<<"fault!";
    }
    QByteArray data = clientConnection->readAll(); // 读取从客户端发送的数据
    QDataStream datastream1(data);
        QString message = QString::fromUtf8(data);    // 将字节数组转换为字符串
        qDebug() << "Received message:" << message; // 将接收到的消息输出到调试区


        // 假设您使用特定的格式来标识注册账户和聊天信息
        //可能的bug:账户密码聊天记录中不能含有”：“
        if (message.startsWith("REGISTER|")) {
                QStringList parts = message.split("|");
                qDebug()<<parts.size();
                if (parts.size() == 4) {
                    QString account = parts[1];
                    QString name = parts[2];
                    QString password = parts[3];

                    qDebug() << "Received registration data:"
                             << "Account:" << account
                             << "Name:" << name
                             << "Password:" << password;

                    // 检查账号是否已存在（假设您有一种方法来检查账号是否存在）
                    bool accountExists = checkIfAccountExists(account);

                    if (accountExists) {
                        qDebug() << "Account already exists. Sending response to client.";
                        QString responseMessage = "REGISTER_FAIL:Account already exists.";
                        if (clientConnection->state() != QAbstractSocket::ConnectedState) {
                            qDebug()<<"连接断开";
                            //clientConnection->connectToHost(ip, port);
                        }
                        clientConnection->write(responseMessage.toUtf8().data());
                    //    ip = clientConnection->peerAddress().toString();
                     //   qint16 port = clientConnection->peerPort();
                     //   qDebug()<<"sented ip:"<<ip<<" port: "<<port;

                    } else {
                        qDebug() << "Account does not exist. Inserting into database.";

                        // 将注册账户信息插入到MySQL注册表中（假设有一个名为 "registration_table" 的表）
                        QSqlQuery query;
                        query.prepare("INSERT INTO qq_account (account, name, password) VALUES (?, ?, ?)");
                        query.addBindValue(account);
                        query.addBindValue(name);
                        query.addBindValue(password);
                        if (!query.exec()) {
                            qDebug() << "Failed to insert registration data into database:" << query.lastError().text();
                        } else {
                            qDebug() << "Registration successful.";
                            QString responseMessage = "REGISTER_SUCCESS:Account registered successfully.";
                            clientConnection->write(responseMessage.toUtf8().data());
                        }
                    }
                }
            }
        else if (message.startsWith("LOGIN|")) {
            QStringList parts = message.split("|");
            if (parts.size() == 3) {
                QString account = parts[1];
                QString password = parts[2];


                bool loginSuccessful = checkLogin(account, password);

                QString responseMessage;
                if (loginSuccessful) {
                    responseMessage = QString("LOGIN_SUCCESS|%1").arg(account);
                } else {
                    responseMessage = "LOGIN_FAIL";
                }

                clientConnection->write(responseMessage.toUtf8());
            }
        }
        else if(message.startsWith("LOGEDIN|")){
            QString account = message.mid(QString("LOGEDIN|").length()); // 提取账号信息
            qDebug() << "Client logged in with account:" << account;

            // 1. 将客户端的IP和账号绑定存储到一个map中
           // clientIPAccountMap.insert(clientConnection, account);
            socketToAccount.insert(clientConnection,account);
            accountToSocket.insert(account,clientConnection);
            // 2. 从数据库表qq_friends(a,b,zu)和数据表qq_account(account,name,password)中连接查询account的好友账号、组别和名称
            // FRIEND|账号1,分组,名称|账号2...
            QString friendsQuery = "SELECT f.b, f.zu, a.name FROM qq_friends f JOIN qq_account a ON f.b = a.account WHERE f.a = ?";
            QSqlQuery query;
            query.prepare(friendsQuery);
            query.addBindValue(account);
            if (query.exec()) {
                QStringList friendsList;
                while (query.next()) {
                    QString friendAccount = query.value(0).toString();
                    QString zu = query.value(1).toString();
                    QString name = query.value(2).toString();
                    friendsList.append(QString("%1,%2,%3").arg(friendAccount).arg(zu).arg(name));
                }
                QString friendsMessage = "FRIEND_LIST|" + friendsList.join("|");
                clientConnection->write(friendsMessage.toUtf8());
            } else {
                qDebug() << "Failed to retrieve friends from database:" << query.lastError().text();
            }


        }
        else if(message.startsWith("GROUP_LIST")){
            qDebug()<<"receive group list";
            QString account=socketToAccount.value(clientConnection) ;
            QSqlQuery query2;
            QString queryString = QString("SELECT qq_groupmember.groupnumber, qq_groupmember.zu, qq_group.name "
                                          "FROM qq_groupmember "
                                          "INNER JOIN qq_group ON qq_groupmember.groupnumber = qq_group.number "
                                          "WHERE qq_groupmember.qq_number = '%1'")
                                      .arg(account);

            if (query2.exec(queryString)) {
                QStringList groupList;
                while (query2.next()) {
                    QString groupNumber = query2.value(0).toString();
                    QString zu = query2.value(1).toString();
                    QString name = query2.value(2).toString();

                    groupList.append( QString("%1,%2,%3").arg(groupNumber).arg(zu).arg(name));
                }
                QString groupMessage="GROUP_LIST|"+groupList.join("|");
                clientConnection->write(groupMessage.toUtf8());

            } else {
                QString groupMessage="GROUP_LIST|";
                qDebug()<<groupMessage;
                clientConnection->write(groupMessage.toUtf8());
                qDebug() << "Query execution failed:" << query2.lastError().text();
            }
        }
        else if(message.startsWith("GROUP_ADD|")){
//            QString account=socketToAccount.value(clientConnection);
//            QSqlQuery query;
//            QString groupnumber=message.split("|")[1];
//                query.prepare("INSERT INTO qq_groupmember (groupnumber, qq_number) VALUES (?, ?)");
//                query.addBindValue(groupnumber);
//                query.addBindValue(account);

//                // 执行查询
//                if (!query.exec()) {
//                    qDebug() << "Failed to insert group member data into database:" << query.lastError().text();
//                    QString failmessage="GROUP_ADD_FAIL:" + query.lastError().text();
//                    clientConnection->write(failmessage.toUtf8());
//                }
//                else{
//                    QString successmessage="GROUP_ADD_SUCCESS";
//                    clientConnection->write(successmessage.toUtf8());
//                }

                    QString senderAccount = socketToAccount.value(clientConnection); // 获取发送消息的账号
                    QStringList messageParts = message.split("|");
                    if (messageParts.size() != 2) {
                        // 格式不正确，返回错误消息
                        QString responseMessage = "GROUP_ADD_FAIL:INVALID_FORMAT";
                        qDebug() << "GROUP_ADD_FAIL:INVALID_FORMAT";
                        clientConnection->write(responseMessage.toUtf8());
                        return;
                    }

                    QString groupNumber = messageParts[1]; // 获取要添加的群号
                    qDebug() << "要添加的群号" << groupNumber;

                    // 查询群是否存在于 qq_group 中
                    QSqlQuery query;
                    query.prepare("SELECT number, name FROM qq_group WHERE number = ?");
                    query.addBindValue(groupNumber);
                    if (!query.exec()) {
                        qDebug() << "Failed to execute query:" << query.lastError().text();
                        return;
                    }

                    if (query.next()) {
                        // 群存在
                        QString groupName = query.value(1).toString();

                        // 查询群成员关系是否已存在
                        query.prepare("SELECT * FROM qq_groupmember WHERE groupnumber = ? AND qq_number = ?");
                        query.addBindValue(groupNumber);
                        query.addBindValue(senderAccount);
                        if (!query.exec()) {
                            qDebug() << "Failed to execute query:" << query.lastError().text();
                            return;
                        }

                        if (query.next()) {
                            // 群成员关系已存在
                            QString responseMessage = "GROUP_ADD_FAIL:ADDED";
                            clientConnection->write(responseMessage.toUtf8());
                        } else {
                            // 添加群成员关系
                            query.prepare("INSERT INTO qq_groupmember (groupnumber, qq_number, zu) VALUES (?, ?, '未分组')");
                            query.addBindValue(groupNumber);
                            query.addBindValue(senderAccount);
                            if (!query.exec()) {
                                qDebug() << "Failed to insert group member data into database:" << query.lastError().text();
                                return;
                            }

                            // 发送成功消息
                            QString responseMessage = QString("GROUP_ADD_SUCCESS:%1|%2").arg(groupNumber).arg(groupName);
                            clientConnection->write(responseMessage.toUtf8());
                        }
                    } else {
                        // 群不存在
                        QString responseMessage = "GROUP_ADD_FAIL:NOT_EXIST";
                        clientConnection->write(responseMessage.toUtf8());
                    }

        }
        else if(message.startsWith("FRIEND_ADD|")){

            //注意，目前的好友添加和好友删除都是单项添加和单项删除

            QString senderAccount = socketToAccount.value(clientConnection); // 获取发送消息的账号a
            QStringList messageParts = message.split("|");
            if (messageParts.size() != 2) {
                // 格式不正确，返回错误消息
                QString responseMessage = "FRIEND_ADD_FAIL:INVALID_FORMAT";
                qDebug()<<"FRIEND_ADD_FAIL:INVALID_FORMAT";
                clientConnection->write(responseMessage.toUtf8());
                return;
            }

            QString targetAccount = messageParts[1]; // 获取要添加的好友的账号b
            qDebug()<<"要添加的账号"<<targetAccount;
            // 查询账号b是否存在于 qq_account 中
            QSqlQuery query;
            query.prepare("SELECT account, name FROM qq_account WHERE account = ?");
            query.addBindValue(targetAccount);
            if (!query.exec()) {
                qDebug() << "Failed to execute query:" << query.lastError().text();
                return;
            }

            if (query.next()) {
                // 账号b存在
                QString friendName = query.value(1).toString();

                // 查询好友关系是否已存在
                query.prepare("SELECT * FROM qq_friends WHERE a = ? AND b = ?");
                query.addBindValue(senderAccount);
                query.addBindValue(targetAccount);
                if (!query.exec()) {
                    qDebug() << "Failed to execute query:" << query.lastError().text();
                    return;
                }

                if (query.next()) {
                    // 好友关系已存在
                    QString responseMessage = "FRIEND_ADD_FAIL:ADDED";
                    clientConnection->write(responseMessage.toUtf8());
                } else {
                    // 添加好友关系
                    query.prepare("INSERT INTO qq_friends (a, b, zu) VALUES (?, ?, '未分组')");
                    query.addBindValue(senderAccount);
                    query.addBindValue(targetAccount);
                    if (!query.exec()) {
                        qDebug() << "Failed to insert friend data into database:" << query.lastError().text();
                        return;
                    }

                    // 发送成功消息
                    QString responseMessage = QString("FRIEND_ADD_SUCCESS:%1|%2").arg(friendName).arg(targetAccount);
                    clientConnection->write(responseMessage.toUtf8());
                }
            } else {
                // 账号b不存在
                QString responseMessage = "FRIEND_ADD_FAIL:NOT_EXIST";
                clientConnection->write(responseMessage.toUtf8());
            }
        }
        else if(message.startsWith("FRIEND_DELETE|")){
            QString senderAccount = socketToAccount.value(clientConnection); // 获取发送消息的账号a
            QStringList messageParts = message.split("|");
            if (messageParts.size() != 2) {
                // 格式不正确，返回错误消息
                QString responseMessage = "FRIEND_DELETE_FAIL:INVALID_FORMAT";
                clientConnection->write(responseMessage.toUtf8());
                return;
            }

            QString targetAccount = messageParts[1]; // 获取要删除的好友的账号b
            // 查询是否存在好友关系
            QSqlQuery query;
            query.prepare("SELECT * FROM qq_friends WHERE a = ? AND b = ?");
            query.addBindValue(senderAccount);
            query.addBindValue(targetAccount);
            if (!query.exec()) {
                qDebug() << "Failed to execute query:" << query.lastError().text();
                return;
            }

            if (query.next()) {
                // 删除好友关系
                query.prepare("DELETE FROM qq_friends WHERE a = ? AND b = ?");
                query.addBindValue(senderAccount);
                query.addBindValue(targetAccount);
                if (!query.exec()) {
                    qDebug() << "Failed to delete friend data from database:" << query.lastError().text();
                    return;
                }

                // 发送成功消息
                QString responseMessage = "FRIEND_DELETE_SUCCESS";
                clientConnection->write(responseMessage.toUtf8());
            } else {
                // 好友关系不存在，返回错误消息
                QString responseMessage = "FRIEND_DELETE_FAIL:RELATION_NOT_FOUND";
                clientConnection->write(responseMessage.toUtf8());
            }
        }
        else if(message.startsWith("FRIEND_GROUP|")){//此功能有待进一步分析，wwc那里未实现全
            // 解析消息格式："FRIEND_GROUP|账号b1,b1变更后的分组名$账号b2,b2变更后的分组名......"
            QStringList parts = message.split('|');
            QString account = socketToAccount.value(clientConnection); // 获取当前连接的账号a

            QStringList accountGroupPairs = parts[1].split('$');
            QSqlQuery query;

            // 开始事务，确保修改操作的原子性
            if (query.exec("BEGIN;")) {
                foreach (const QString &accountGroupPair, accountGroupPairs) {
                    QStringList accountGroup = accountGroupPair.split(',');
                    if (accountGroup.size() == 2) {
                        QString friendAccount = accountGroup[0];
                        QString newGroup = accountGroup[1];

                        // 更新数据库中的分组信息
                        QString updateQuery = QString("UPDATE qq_friends SET group = '%1' WHERE a = '%2' AND b = '%3';")
                                .arg(newGroup)
                                .arg(account)
                                .arg(friendAccount);

                        if (!query.exec(updateQuery)) {
                            qDebug() << "Failed to update friend group:" << query.lastError().text();
                            // 回滚事务
                            query.exec("ROLLBACK;");
                            // 返回错误消息给客户端
                            QString errorMessage = "FRIEND_GROUP_FAIL:" + query.lastError().text();
                            clientConnection->write(errorMessage.toUtf8());
                            return;
                        }
                    }
                }

                // 提交事务
                if (query.exec("COMMIT;")) {
                    qDebug() << "Friend group changes committed to database.";
                    // 返回成功消息给客户端
                    QString successMessage = "FRIEND_GROUP_SUCCESS";
                    clientConnection->write(successMessage.toUtf8());
                } else {
                    qDebug() << "Failed to commit friend group changes:" << query.lastError().text();
                    // 返回错误消息给客户端
                    QString errorMessage = "FRIEND_GROUP_FAIL:" + query.lastError().text();
                    clientConnection->write(errorMessage.toUtf8());
                }
            } else {
                qDebug() << "Failed to start transaction:" << query.lastError().text();
                // 返回错误消息给客户端
                QString errorMessage = "FRIEND_GROUP_FAIL:" + query.lastError().text();
                clientConnection->write(errorMessage.toUtf8());
            }
        }
        else if(message.startsWith("GROUP_CREATE|")){
                    QStringList parts = message.split('|');
                    if (parts.size() != 4) {
                        // 无效的消息格式
                        qDebug() << "Invalid GROUP_CREATE message:" << message;
                        // 返回错误消息给客户端
                        QString errorMessage = "GROUP_CREATE_FAIL:Invalid message format";
                        clientConnection->write(errorMessage.toUtf8());
                        return;
                    }

                    QString groupNumber = parts[1];
                    QString groupName = parts[2];
                    QString creatorAccount = parts[3];

                    QSqlQuery query,query2;

                    // 插入新的群记录到数据库
                    QString insertQuery = QString("INSERT INTO qq_group (number, name, creator) VALUES ('%1', '%2', '%3');")
                            .arg(groupNumber)
                            .arg(groupName)
                            .arg(creatorAccount);

                    if (query.exec(insertQuery)) {
                        // 插入新的群成员记录到数据库

                        QString insertGroupMemberQuery = QString("INSERT INTO qq_groupmember (groupnumber, qq_number,zu) VALUES ('%1', '%2','未分组');")
                                .arg(groupNumber)
                                .arg(creatorAccount);
                        query2.exec(insertGroupMemberQuery);
                       if(!query2.exec())
                            qDebug()<<"didnot insert"<<query.lastError().text()  ;                      qDebug() << "Group created successfully.";
                        // 返回成功消息给客户端
                        QString successMessage = "GROUP_CREATE_SUCCESS";
                        clientConnection->write(successMessage.toUtf8());
                    } else {
                        qDebug() << "Failed to create group:" << query.lastError().text();
                        // 返回错误消息给客户端
                        QString errorMessage = "GROUP_CREATE_FAIL:" + query.lastError().text();
                        clientConnection->write(errorMessage.toUtf8());
                    }
            }
        else if (message.startsWith("CHAT_PERSON|")) {
                // 解析聊天信息
                QStringList parts = message.split("|");
                if (parts.size() == 2) {
                    QString chatInfo = parts[1];
                    QStringList chatDetails = chatInfo.split("$");
                    if (chatDetails.size() == 4) {
                        QString sender = chatDetails[0];
                        QString receiver = chatDetails[1];
                        QString chatMessage = chatDetails[2];
                        QString sendTime = chatDetails[3];
                        qDebug() << "Received chat data:"
                                 << "Sender:" << sender
                                 << "Receiver:" << receiver
                                 << "Message:" << chatMessage
                                 << "Send Time:" << sendTime;
                        // 存储到数据库
                        QSqlQuery query;
                        query.prepare("INSERT INTO qq_chat_record (sender, receiver, message, send_time) VALUES (?, ?, ?, ?)");
                        query.addBindValue(sender);
                        query.addBindValue(receiver);
                        query.addBindValue(chatMessage);
                        query.addBindValue(sendTime);
                        if (!query.exec()) {
                            qDebug() << "Failed to insert chat data into database:" << query.lastError().text();
                        }

                        // 发送给目标客户端
                        if(accountToSocket.contains(receiver)){
                            QTcpSocket* socket=accountToSocket.value(receiver);
                            socket->write(message.toUtf8());
                        }
                    }
                }
            }
        else if(message.startsWith("CHAT_REQUEST|")){



        QString accountB = message.mid(QString("CHAT_REQUEST|").length());
        QString accountA = socketToAccount.value(clientConnection);

        // Perform database query to retrieve chat history for both directions
        QSqlQuery query;
        query.prepare("SELECT sender, receiver, message, send_time FROM qq_chat_record "
                      "WHERE (sender = :senderA AND receiver = :receiverB) "
                      "OR (sender = :senderB AND receiver = :receiverA)"
                      "ORDER BY send_time ASC");
        query.bindValue(":senderA", accountA);
        query.bindValue(":receiverB", accountB);
        query.bindValue(":senderB", accountB);
        query.bindValue(":receiverA", accountA);

        if (!query.exec()) {
            qDebug() << "Failed to fetch chat history:" << query.lastError().text();
            return;
        }

        QStringList messages;

        while (query.next()) {
            QString sender = query.value("sender").toString();
            QString receiver = query.value("receiver").toString();
            QString message = query.value("message").toString();
            QString sendTime = query.value("send_time").toString();
            QString chatEntry = QString("%1$%2$%3$%4").arg(sender).arg(receiver).arg(message).arg(sendTime);
            messages.append(chatEntry);
        }

        QString response = "CHAT_PERSON|" + messages.join("&");
        clientConnection->write(response.toUtf8());
    }
        else if (message.startsWith("CHAT_GROUP|")) {

            // 解析聊天信息
            QStringList parts = message.split("|");
            if (parts.size() == 2) {
                QString chatInfo = parts[1];
                QStringList chatDetails = chatInfo.split("$");
                if (chatDetails.size() == 4) {
                    QString groupnumber = chatDetails[1];
                    QString  sender= chatDetails[0];
                    QString chatMessage = chatDetails[2];
                    QString sendTime = chatDetails[3];
                    qDebug() << "Received chat data:"
                             << "Sender:" << sender
                             << "groupnumber:" << groupnumber
                             << "Message:" << chatMessage
                             << "Send Time:" << sendTime;
                    // 存储到数据库
                    QSqlQuery query;
                    query.prepare("INSERT INTO qq_groupmessage (group_number, sender, send_time, message) VALUES (?, ?, ?, ?)");
                    query.addBindValue(groupnumber);
                    query.addBindValue(sender);
                    query.addBindValue(sendTime);
                    query.addBindValue(chatMessage);
                    if (!query.exec()) {
                        qDebug() << "Failed to insert chat data into database:" << query.lastError().text();
                    }
                    QSqlQuery query2;
                           query2.prepare("SELECT qq_number FROM qq_groupmember WHERE groupnumber = ?");
                           query2.addBindValue( groupnumber);
                           query2.exec();

                           while (query2.next()) {
                                                  QString qqNumber = query2.value(0).toString();

                                                  // 查找对应的QTcpSocket*，如果存在则转发消息
                                                  if (accountToSocket.contains(qqNumber)) {
                                                      QTcpSocket *receiverSocket = accountToSocket.value(qqNumber);
                                                      receiverSocket->write(message.toUtf8()); // 转发消息
                                                      qDebug()<<"zhuanfa:"<<qqNumber<<" :"<<message;
                                                  }
                                               }

                    }
            }
    }
        else if(message.startsWith("GROUP_CHAT_REQUEST|")){

                   QString groupNumber = message.mid(QString("GROUP_CHAT_REQUEST|").length());

                   QSqlQuery query;
                   QString queryString = QString("SELECT group_number,sender, send_time, message "
                                                 "FROM qq_groupmessage "
                                                 "WHERE group_number = '%1' ORDER BY send_time ASC")
                                         .arg(groupNumber);

                   if (query.exec(queryString)) {
                       QStringList groupList;
                       while (query.next()) {
                           QString groupNumber=query.value(0).toString();
                           QString sender = query.value(1).toString();
                           QString sendTime = query.value(2).toString();
                           QString text = query.value(3).toString();
                            groupList.append(QString("%1$%2$%3$%4").arg(sender).arg( groupNumber).arg( text).arg(sendTime));

                       }
                    QString chatRequest="CHAT_GROUP|"+groupList.join("&");
                       clientConnection->write(chatRequest.toUtf8());

                   } else {
                       qDebug() << "Query execution failed:" << query.lastError().text();
                   }

        }
    else{
          //---------------------------

            qint64 fileNameSize=0;
            QDataStream in(clientConnection);//-- 还要定义bytesreceived
            in.setVersion(QDataStream::Qt_4_6);
            qint64 totalBytes;//--

            QString fileName;//--
            QFile *localFile;//--
            if(bytesReceived <= sizeof(qint64)*2)
            { //如果接收到的数据小于16个字节，那么是刚开始接收数据，我们保存到//来的头文件信息
                if((clientConnection->bytesAvailable() >= sizeof(qint64)*2)//--
                        && (fileNameSize == 0))
                { //接收数据总大小信息和文件名大小信息
                    in >> totalBytes >> fileNameSize;
                    bytesReceived += sizeof(qint64) * 2;
                }
                if((clientConnection->bytesAvailable() >= fileNameSize)//--
                        && (fileNameSize != 0))
                {  //接收文件名，并建立文件
                    in >> fileName;
                    //ui->label->setText(tr("接收文件 %1 ...").arg(fileName));
                    bytesReceived += fileNameSize;
                    localFile= new QFile("D:\\result\\" + fileName);
                    if(!localFile->open(QFile::WriteOnly))
                    {
                        qDebug() << "open file error!";
                        return;
                    }
                }
                else return;
            }
            if(bytesReceived < totalBytes)
            {  //如果接收的数据小于总数据，那么写入文件
                QByteArray inBlock;//--
                bytesReceived += clientConnection->bytesAvailable();//--
                inBlock= clientConnection->readAll();//--
                localFile->write(inBlock);
                inBlock.resize(0);
            }
            //更新进度条
//            ui->progressBar->setMaximum(totalBytes);
//            ui->progressBar->setValue(bytesReceived);

            if(bytesReceived == totalBytes)
            { //接收数据完成时
                clientConnection->close();
                localFile->close();
                //ui->label->setText(tr("接收文件 %1 成功！").arg(fileName));
                this->totalBytes = 0;
                this->bytesReceived = 0;
            }
            //------------------

//        QDataStream dataStream(&data, QIODevice::ReadOnly);
//        // 读取账号信息长度和账号信息
//        qint32 accountSize;
//        dataStream >> accountSize;
//        QByteArray accountBytes(accountSize, Qt::Uninitialized);
//        dataStream.readRawData(accountBytes.data(), accountSize);
//        QString accountInfo = QString::fromUtf8(accountBytes);

//        // 读取文件名长度和文件名
//        qint32 fileNameSize;
//        dataStream >> fileNameSize;
//        QByteArray fileNameBytes(fileNameSize, Qt::Uninitialized);
//        dataStream.readRawData(fileNameBytes.data(), fileNameSize);
//        QString fileName = QString::fromUtf8(fileNameBytes);


//        // 打开文件以写入数据
//        QFile file(fileName);
//        if (!file.open(QIODevice::WriteOnly)) {
//            qDebug() << "Failed to open file for writing: " << fileName;
//            return;
//        }

//        // 读取并写入文件数据
//        while (!dataStream.atEnd()) {
//            QByteArray fileData;
//            dataStream >> fileData;
//            file.write(fileData);
//        }

//        // 关闭文件
//        file.close();

//        qDebug() << "File received: " << fileName;
//        qDebug() << "Account Info: " << accountInfo;
    }
}


//删除组
//

//创建组
//CHAT_GROUP_REQUEST



//每当初始化服务器端时，执行loadChatRecordsFromDatabase函数，实现消息记录漫游。同时，此函数只在初始化被执行依1次，防止检索出来的数据产生重复。
void MainWindow::loadChatRecordsFromDatabase()
{
    // 连接数据库表"qq_chat_record"
    QSqlQuery query("SELECT sender, receiver, message, send_time FROM qq_chat_record ORDER BY send_time");

    if (!query.exec()) {
        qDebug() << "Failed to fetch chat records:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        QString sender = query.value(0).toString();
        QString receiver = query.value(1).toString();
        QString message = query.value(2).toString();
        QString sendTime = query.value(3).toString();

        // 将检索到的记录显示在 listWidget 中
        QString listItem = QString("[%1] [%2 -> %3]: %4").arg(sendTime).arg(sender).arg(receiver).arg(message);
        qDebug() << "Loaded chat record:" << listItem;
        QListWidgetItem *item = new QListWidgetItem(listItem);
        ui->listWidget->addItem(item);
    }
    qDebug() << "Chat records loaded from database.";
}

MainWindow::~MainWindow()
{
    delete ui;
}



