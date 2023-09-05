#include "clientmaininterface.h"
#include "ui_clientmaininterface.h"
#include<QListWidget>
#include<QListWidgetItem>
#include<QFileDialog>
#include<QToolButton>
#include<QLabel>
#include<QPlainTextEdit>
#include<QDateTime>
ClientMainInterface::ClientMainInterface(QString Account,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientMainInterface),account(Account)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);
    filesocket=new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(haveConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
    tcpSocket->connectToHost("127.0.0.1", 6666); // 修改为服务器地址和端口

    //------------------

    filesocket=new QTcpSocket(this);
    filesocket->connectToHost("127.0.0.1",10000);
    ui->pushButton_2->setEnabled(false);
    connect(filesocket,&QTcpSocket::connected,
            [=]() mutable
            {
                ui->pushButton_2->setEnabled(true);
                //从通信套接字取出内容
                //客户端给服务端发送消息


            }
            );
    connect(&timer,&QTimer::timeout,[=]()
    {
        timer.stop();
        sendfile();
    });

    //----------------------


    ui->tabWidget->setTabText(0,"好友");
    ui->tabWidget->setTabText(1,"群聊");
    ui->tabWidget->setTabEnabled(0, true);
    ui->tabWidget->setTabEnabled(1, true);

    ui->pushButton_3->hide();
    ui->plainTextEdit->hide();
    ui->pushButton_2->hide();
    ui->pushButton_4->hide();
   //init();

}
//---------------------
void ClientMainInterface::sendfile()
{
    qint64 len=0;
        do
        {
            //每次发送数据的大小
            char buf[4*1024]={0};
            len=0;
            //在文件中读数据
            len=file.read(buf,sizeof(buf));
            //读多少发多少
            len=filesocket->write(buf,len);

            send_size+=len;
        }while(len>0);
        //是否文件发送完毕
        if(send_size==filesize)
        {
//            QListWidgetItem *pItem = new QListWidgetItem;

//            QDateTime datetime = QDateTime::currentDateTime();
//            QString date = datetime.toString("hh:mm:ss");
//            pItem->setText("[" + date + "]" + "[我]: 文件发送完毕" );
//            pItem->setTextAlignment(Qt::AlignRight);
//            pItem->setIcon(QIcon(server_info->headPath));

//            ui->read_list->addItem(pItem);
            file.close();
            ui->pushButton_4->setEnabled(false);
            ui->pushButton_2->setEnabled(true);
        }
}
//--------------
void ClientMainInterface::init_group_talking(QString &message){
    qDebug()<<message;
    m_pModel_group=new QStandardItemModel(ui->treeView_2);
    ui->treeView_2->setModel(m_pModel_group);
    m_pModel_group->setHorizontalHeaderLabels(QStringList() << "群聊列表");//设置表头
        //设置展开
        ui->treeView_2->expandAll();
        QStandardItem *item1 = new QStandardItem("未分组");//此组一定存在
        m_pModel_group->setItem(0,0,item1);//设置位置
        item1->setEditable(false);


        //QString send_to_me="FRIEND_LIST|123,未分组,张三|456,未分组,李四|789,特别关注,王五|147,黑名单,马六";
        QString send_to_me=message;
        QStringList strList=send_to_me.split("|");
        if(strList[1]==""){
            ui->treeView_2->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(ui->treeView_2,&QTreeView::customContextMenuRequested,
                          this,&ClientMainInterface::slotMenuPopup_group);
            return;
        }

        strList.removeFirst();
        qDebug()<<strList;

        QStringList group_exist;//已有组名
        group_exist<<"未分组";

        for(int i=0;i<strList.size();i++){
            QStringList new_list=strList[i].split(",");
            QString count=new_list[0];
            QString group=new_list[1];
            QString name=new_list[2];
            qDebug()<<name;

            if(group_exist.contains(group)){//组名已存在

                int row_0=0;//组的索引
                for (row_0 = 0; row_0 < m_pModel_group->rowCount(); ++row_0) {
                        QModelIndex index = m_pModel_group->index(row_0, 0);
                        if (index.isValid()) {
                            QStandardItem *item = m_pModel_group->itemFromIndex(index);
                            if (item && item->text() == group) {
                                break;
                            }
                        }
                    }
                QStandardItem *group_none = m_pModel_group->itemFromIndex(m_pModel_group->index(row_0,0));
                QStandardItem *Item=new QStandardItem(name);
                Item->setData(count);
                //qDebug()<<Item->data().toInt();
                group_none->appendRow(Item);
                Item->setEditable(false);

            }
            else if(!group_exist.contains(group)){

                 QStandardItem *rootItem = m_pModel_group->invisibleRootItem();

                 QStandardItem *groupItem = new QStandardItem(group);

                 group_exist<<group;

                 rootItem->appendRow(groupItem);//添加组

                 groupItem->setEditable(false);

                 QStandardItem *Item=new QStandardItem(name);

                 Item->setData(count);
                 Item->setEditable(false);

                 groupItem->appendRow(Item);

            }

    }

        //右键点击事件
        ui->treeView_2->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->treeView_2,&QTreeView::customContextMenuRequested,
                      this,&ClientMainInterface::slotMenuPopup_group);
}

void ClientMainInterface::init(QString &message){
    QString grouplist="GROUP_LIST";
    tcpSocket->write(grouplist.toUtf8());
    qDebug()<<"have sent group_list";
//    //头像
//    QToolButton* imgbtn = new QToolButton(this);
//    //imgbtn->setIcon(QIcon(picon));
//    imgbtn->setIconSize(QSize(40,40));
//    imgbtn->setGeometry(10,10,40,40);
//    imgbtn->setStyleSheet("QToolButton:{border:none;}");
//    imgbtn->show();

    m_pModel = new QStandardItemModel(ui->treeView);

    //将数据模型设置到树形视图上
    ui->treeView->setModel(m_pModel);
    m_pModel->setHorizontalHeaderLabels(QStringList() << "好友列表");//设置表头
    //设置展开
    ui->treeView->expandAll();
    QStandardItem *item1 = new QStandardItem("未分组");//此组一定存在
    m_pModel->setItem(0,0,item1);//设置位置


    //QString send_to_me="FRIEND_LIST|123,未分组,张三|456,未分组,李四|789,特别关注,王五|147,黑名单,马六";
    QString send_to_me=message;
    QStringList strList=send_to_me.split("|");
    if(strList[1]==""){
        ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->treeView,&QTreeView::customContextMenuRequested,
                      this,&ClientMainInterface::slotMenuPopup);
        return;
    }

    strList.removeFirst();
    qDebug()<<strList;

    QStringList group_exist;//已有组名
    group_exist<<"未分组";

    for(int i=0;i<strList.size();i++){
        QStringList new_list=strList[i].split(",");
        QString count=new_list[0];
        QString group=new_list[1];
        QString name=new_list[2];
        qDebug()<<name;

        if(group_exist.contains(group)){//组名已存在

            int row_0=0;//组的索引
            for (row_0 = 0; row_0 < m_pModel->rowCount(); ++row_0) {
                    QModelIndex index = m_pModel->index(row_0, 0);
                    if (index.isValid()) {
                        QStandardItem *item = m_pModel->itemFromIndex(index);
                        if (item && item->text() == group) {
                            break;
                        }
                    }
                }
            QStandardItem *group_none = m_pModel->itemFromIndex(m_pModel->index(row_0,0));
            QStandardItem *Item=new QStandardItem(name);
            Item->setData(count);
            //qDebug()<<Item->data().toInt();
            group_none->appendRow(Item);

        }
        else if(!group_exist.contains(group)){

             QStandardItem *rootItem = m_pModel->invisibleRootItem();

             QStandardItem *groupItem = new QStandardItem(group);

             group_exist<<group;

             rootItem->appendRow(groupItem);//添加组

             QStandardItem *Item=new QStandardItem(name);

             Item->setData(count);

             groupItem->appendRow(Item);

        }



    }
//    QString grouplist="GROUP_LIST";
//    tcpSocket->write(grouplist.toUtf8());
//    qDebug()<<"have sent group_list";
    //右键点击事件
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView,&QTreeView::customContextMenuRequested,
                  this,&ClientMainInterface::slotMenuPopup);
}
void ClientMainInterface::readMessage() {
    QByteArray data = tcpSocket->readAll();
    QString message = QString::fromUtf8(data);
    qDebug()<<message;

    QList<QByteArray> parts = data.split('|');
    QByteArray command = parts.first();

    //单独处理传输文件部分
    if (command == "SEND_FILE"){
        // Handle file transfer
                if (parts.size() < 4)return;

                    QString receiverAccount=parts[1];
                    QString fileName = parts[2];
                    QByteArray fileData = parts[4];

                    // Save the received file data
                    QFile file("received_" + fileName);
                    if (file.open(QIODevice::WriteOnly))
                    {
                        file.write(fileData);
                        file.close();
                        qDebug() << "Received and saved file:" << fileName;
                    }
                    else
                    {
                        qDebug() << "Failed to save received file:" << fileName;
                    }
    }


    // 处理从服务器接收的不同类型的消息
    if (message.startsWith("FRIEND_LIST|")) {
        init(message);
    }
    else if(message.startsWith("FRIEND_ADD")){
            if(message.startsWith("FRIEND_ADD_SUCCESS")){
            int row_0=0;//“未分组索引”
            for (row_0 = 0; row_0 < m_pModel->rowCount(); ++row_0) {
                    QModelIndex index = m_pModel->index(row_0, 0);
                    if (index.isValid()) {
                        QStandardItem *item = m_pModel->itemFromIndex(index);
                        if (item && item->text() == "未分组") {
                            break;
                        }
                    }
                }
            QStandardItem *group_none = m_pModel->itemFromIndex(m_pModel->index(row_0,0));
            QString name=message.split("|")[0].split(":")[1];
            QString count=message.split("|")[1];
            QStandardItem *Item=new QStandardItem(name);
            Item->setData(count);

            group_none->appendRow(Item);
        }

        else if(message=="FRINED_ADD_FAIL:NOT_EXIST"){
            QMessageBox errorMessage;
            errorMessage.setIcon(QMessageBox::Critical);
            errorMessage.setText("错误");
            errorMessage.setInformativeText("该用户不存在");
            errorMessage.setWindowTitle("Error");
            errorMessage.exec();
        }
        else if(message=="FRINED_ADD_FAIL:FRIEND_EDDED"){
            QMessageBox errorMessage;
            errorMessage.setIcon(QMessageBox::Critical);
            errorMessage.setText("错误");
            errorMessage.setInformativeText("该用户已添加");
            errorMessage.setWindowTitle("Error");
            errorMessage.exec();
        }
    }
    else if(message.startsWith("CHAT_PERSON|")){
            //chat=message.split("|");
            qDebug()<<message;
            if(message.split("|")[1]=="")
                return;
            QString count1=message.split("|")[1].split("$")[0];
            QString count2=message.split("|")[1].split("$")[1];

            QString count;
            QString name;
            if(count1==account){
                count=count2;
            }
            else{
                count=count1;
            }
            // 寻找用户名
            int row=0;
            for (row = 0; row < m_pModel->rowCount(); ++row) {
                    QModelIndex index = m_pModel->index(row, 0);
                    if (index.isValid()) {
                        QStandardItem *groupitem = m_pModel->itemFromIndex(index);

                        for (int row = 0; row < groupitem->rowCount(); ++row) {
                               QStandardItem *memberItem = groupitem->child(row);
                               if (memberItem) {
                                   if(memberItem->data().toString()==count){
                                       QListWidget *associatedListWidget = itemToListWidget.value(memberItem);
                                       if(associatedListWidget==nullptr)
                                            return;
                                       QString chats=message.split("|")[1];
                                       QStringList chat_list=chats.split("&");
                                       foreach (const QString &chat, chat_list){
                                           QStringList chat_message= chat.split("$");
                                           if(chat_message.size()==4){
                                               QString sender=chat_message[0];
                                               QString receiver=chat_message[1];
                                               QString message=chat_message[2];
                                               QString sendTime=chat_message[3];
                                               QString listItem = QString("[%1] [%2 -> %3]: %4").arg(sendTime).arg(sender).arg(receiver).arg(message);
                                               qDebug() << "Loaded chat record:" << listItem;
                                                QListWidgetItem *listWidgetItem = new QListWidgetItem(listItem);
                                                if(sender==account)
                                                      listWidgetItem->setTextAlignment(Qt::AlignRight);
                                                associatedListWidget=itemToListWidget.value(memberItem);
                                               // QListWidget *associatedListWidget = itemToListWidget.value(memberItem);
                                                associatedListWidget->addItem(listWidgetItem);
                                           }
                                       }
                                   }
                               }
                           }
                    }
                }

        }
    else if(message.startsWith("GROUP_LIST|")){

           qDebug()<<"have recived"<<message;
           init_group_talking(message);
        }
        else if(message.startsWith("CHAT_GROUP|")){
        //chat=message.split("|");
                qDebug()<<message;
                if(message.split("|")[1]=="")
                    return;
//                QString id=message.split("|")[1].split("&")[1].split("$")[1];
  //              QString count=message.split("|")[1].split("&")[1].split("$")[0];
                QString count=message.split("|")[1].split("$")[0];
                QString id=message.split("|")[1].split("$")[1];

                // 寻找用户名
                int row=0;
                for (row = 0; row < m_pModel_group->rowCount(); ++row) {

                        QModelIndex index =m_pModel_group->index(row, 0);
                        if (index.isValid()) {
                            QStandardItem *groupitem = m_pModel_group->itemFromIndex(index);

                            for (int row1 = 0; row1 < groupitem->rowCount(); ++row1) {
                                   QStandardItem *memberItem = groupitem->child(row1);

                                   if (memberItem) {
                                       qDebug()<<row<<"here";
                                       if(memberItem->data().toString()==id){

                                           qDebug()<<id;
                                           QListWidget *associatedListWidget = itemToListWidget_group.value(memberItem);
                                           qDebug()<<id;
                                           if(associatedListWidget==nullptr)
                                                return;
                                           QString chats=message.split("|")[1];
                                           QStringList chat_list=chats.split("&");
                                           foreach (const QString &chat, chat_list){
                                               QStringList chat_message= chat.split("$");
                                               if(chat_message.size()==4){
                                                   QString sender=chat_message[0];
                                                   QString receiver=chat_message[1];
                                                   QString message=chat_message[2];
                                                   QString sendTime=chat_message[3];

                                                  // if(account!=count){
                                                       QString listItem = QString("[%1] %2: %3").arg(sendTime).arg(sender).arg(message);
                                                       qDebug() << "Loaded chat record:" << listItem;

                                                        QListWidgetItem *listWidgetItem = new QListWidgetItem(listItem);
                                                        if(account==sender)
                                                            listWidgetItem->setTextAlignment(Qt::AlignRight);
                                                        associatedListWidget=itemToListWidget_group.value(memberItem);
                                                       // QListWidget *associatedListWidget = itemToListWidget.value(memberItem);
                                                        associatedListWidget->addItem(listWidgetItem);
                                                  // }
                                               }
                                           }
                                       }
                                   }
                               }
                        }
                    }
        }
        else if(message.startsWith("GROUP_ADD")){
            if(message.startsWith("GROUP_ADD_SUCCESS")){
            int row_0=0;//“未分组索引”
            for (row_0 = 0; row_0 < m_pModel_group->rowCount(); ++row_0) {
                    QModelIndex index = m_pModel_group->index(row_0, 0);
                    if (index.isValid()) {
                        QStandardItem *item = m_pModel_group->itemFromIndex(index);
                        if (item && item->text() == "未分组") {
                            break;
                        }
                    }
                }
            //QStandardItem *group_none = m_pModel_group->itemFromIndex(m_pModel_group->index(row_0,0));
            qDebug()<<"here"<<message;
            QString name=message.split("|")[1];
            qDebug()<<name;
            QString count=message.split("|")[0].split(":")[1];
            qDebug()<<"here2";
            QStandardItem *Item=new QStandardItem(name);
            Item->setData(count);
             qDebug()<<"here3";
            //group_none->appendRow(Item);
            m_pModel_group->itemFromIndex(m_pModel_group->index(row_0,0))->appendRow(Item);
        }

        else if(message=="GROUP_ADD_FAIL:NOT_EXIST"){
            QMessageBox errorMessage;
            errorMessage.setIcon(QMessageBox::Critical);
            errorMessage.setText("错误");
            errorMessage.setInformativeText("该群不存在");
            errorMessage.setWindowTitle("Error");
            errorMessage.exec();
        }
        else if(message=="GROUP_ADD_FAIL:FRIEND_EDDED"){
            QMessageBox errorMessage;
            errorMessage.setIcon(QMessageBox::Critical);
            errorMessage.setText("错误");
            errorMessage.setInformativeText("该群已添加");
            errorMessage.setWindowTitle("Error");
            errorMessage.exec();
        }
        }
}
void ClientMainInterface::slotMenuPopup(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);//判断右键时刻的位置
    if (index.isValid()) {
        QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
            if (model) {
                QStandardItem *item = model->itemFromIndex(index);

                if (item) {
                    QStandardItem *parentItem = item->parent();

                    if (parentItem) {//存在父母节点：说明是成员。显示移动到新组
                        qDebug() << "This is a child item in group:" << parentItem->text();
                        QModelIndex currentSelectedIndex = index;

                        QMenu contextMenu(this);

                        for (int row = 0; row < model->rowCount(); ++row) {
                                   QStandardItem *groupItem = model->item(row);
                                   if (groupItem) {
                                       QString groupName = groupItem->text();
                                       QAction *moveToGroup1Action = contextMenu.addAction("移动到组："+groupName);
                                       //寻找特定组名的idex
                                       int row=0;
                                       for (row = 0; row < model->rowCount(); ++row) {
                                               QModelIndex index = model->index(row, 0);
                                               if (index.isValid()) {
                                                   QStandardItem *item = model->itemFromIndex(index);
                                                   if (item && item->text() == groupName) {
                                                       break;
                                                   }
                                               }
                                           }

                                       connect(moveToGroup1Action, &QAction::triggered, this, [=]() {
                                           moveMemberToGroup(currentSelectedIndex,model->index(row,0),model);
                                       });

                                   }
                               }

                        //删除好友
                        QAction *moveToGroup1Action = contextMenu.addAction("删除好友");
                        connect(moveToGroup1Action, &QAction::triggered, this, [=]() {
                            Dropfriend(currentSelectedIndex,model);
                        });

                        contextMenu.exec(mapToGlobal(pos));


                    }
                    else {
                        qDebug() << "This is a group item.";
                        QMenu contextMenu(this);
                        QAction *addGroupAction1 = contextMenu.addAction("添加组");
                        QAction *addGroupAction2 = contextMenu.addAction("删除组");

                        connect(addGroupAction1, &QAction::triggered, this, [=]() {
                            addGroup(model);
                        });


                        connect(addGroupAction2, &QAction::triggered, this, [=]() {
                            DropGroup(index,model);
                        });


                        contextMenu.exec(mapToGlobal(pos));
                    }
                }
                else {
                    qDebug() << "Invalid item.";
                }



    }
}
}
void ClientMainInterface::slotMenuPopup_group(const QPoint &pos)
{
    QModelIndex index = ui->treeView_2->indexAt(pos);//判断右键时刻的位置
    if (index.isValid()) {
        QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeView_2->model());
            if (model) {
                QStandardItem *item = model->itemFromIndex(index);

                if (item) {
                    QStandardItem *parentItem = item->parent();

                    if (parentItem) {//存在父母节点：说明是成员。显示移动到新组
                        qDebug() << "This is a child item in group:" << parentItem->text();
                        QModelIndex currentSelectedIndex = index;

                        QMenu contextMenu(this);

                        for (int row = 0; row < model->rowCount(); ++row) {
                                   QStandardItem *groupItem = model->item(row);
                                   if (groupItem) {
                                       QString groupName = groupItem->text();
                                       QAction *moveToGroup1Action = contextMenu.addAction("移动到组："+groupName);
                                       //寻找特定组名的idex
                                       int row=0;
                                       for (row = 0; row < model->rowCount(); ++row) {
                                               QModelIndex index = model->index(row, 0);
                                               if (index.isValid()) {
                                                   QStandardItem *item = model->itemFromIndex(index);
                                                   if (item && item->text() == groupName) {
                                                       break;
                                                   }
                                               }
                                           }

                                       connect(moveToGroup1Action, &QAction::triggered, this, [=]() {
                                           moveMemberToGroup(currentSelectedIndex,model->index(row,0),model);
                                       });

                                   }
                               }

                        //删除好友
                        QAction *moveToGroup1Action = contextMenu.addAction("删除群");
                        connect(moveToGroup1Action, &QAction::triggered, this, [=]() {
                            Dropfriend(currentSelectedIndex,model);
                        });

                        contextMenu.exec(mapToGlobal(pos));


                    }
                    else {
                        qDebug() << "This is a group item.";
                        QMenu contextMenu(this);
                        QAction *addGroupAction1 = contextMenu.addAction("添加组");
                        QAction *addGroupAction2 = contextMenu.addAction("删除组");

                        connect(addGroupAction1, &QAction::triggered, this, [=]() {
                            addGroup(model);
                        });


                        connect(addGroupAction2, &QAction::triggered, this, [=]() {
                            DropGroup(index,model);
                        });


                        contextMenu.exec(mapToGlobal(pos));
                    }
                }
                else {
                    qDebug() << "Invalid item.";
                }



    }
}
}
void ClientMainInterface::addGroup(QStandardItemModel  *model)
   {

       QStandardItem *rootItem = model->invisibleRootItem();
       bool ok;
       QString text = QInputDialog::getText(nullptr, "Input Dialog", "输入新分组名称:", QLineEdit::Normal, "", &ok);
       int row;
       int flag=1;
       for (row = 0; row < model->rowCount(); ++row) {
               QModelIndex index = model->index(row, 0);
               if (index.isValid()) {
                   QStandardItem *item = model->itemFromIndex(index);
                   if (item && item->text() == text) {
                       flag=0;
                       QMessageBox errorMessage;
                       errorMessage.setIcon(QMessageBox::Critical);
                       errorMessage.setText("错误");
                       errorMessage.setInformativeText("分组已经存在");
                       errorMessage.setWindowTitle("Error");
                       errorMessage.exec();
                       break;
                   }
               }
           }
       if(flag==1){
           QStandardItem *groupItem = new QStandardItem(text);
           rootItem->appendRow(groupItem);
       }

   }

//删除朋友,需要数据库
void ClientMainInterface::Dropfriend(const QModelIndex &memberIndex,QStandardItemModel  *model){
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "确认", "确定删除好友？", QMessageBox::Yes | QMessageBox::No);
    if(reply==QMessageBox::Yes){
        QStandardItem *memberItem = model->itemFromIndex(memberIndex);
        QStandardItem *parentItem = memberItem->parent();

        if(model==m_pModel){
            QString drop_friend_send=QString("FRIEND_DELETE|%1").arg(memberItem->data().toString());
            qDebug()<<drop_friend_send;
            tcpSocket->write(drop_friend_send.toUtf8().data());
            qDebug()<<"send finished";
        }
        else{
            QString drop_friend_send=QString("GROUP_DELETE|%1").arg(memberItem->data().toString());
            qDebug()<<drop_friend_send;
            tcpSocket->write(drop_friend_send.toUtf8().data());
            qDebug()<<"send finished";
        }


        parentItem->removeRow(memberItem->row());

        //信息发送
    }
}

//组件移动，需要数据库
void ClientMainInterface::moveMemberToGroup(const QModelIndex &memberIndex, const QModelIndex &groupIndex,QStandardItemModel  *model)
    {
        QStandardItem *memberItem = model->itemFromIndex(memberIndex);
        QStandardItem *groupItem = model->itemFromIndex(groupIndex);

        QStandardItem *parentItem = memberItem->parent();
        if (parentItem) {
            QStandardItem *Item=new QStandardItem(memberItem->text());
            Item->setData(memberItem->data());
            if(model==m_pModel){
                QString move_friend_send=QString("FRIEND_GROUP|%1,%2").arg(memberItem->data().toString()).arg(groupItem->text());
                qDebug()<< move_friend_send;
                tcpSocket->write(move_friend_send.toUtf8().data());
                qDebug()<<"send finished";
            }
            else{
                QString move_friend_send=QString("GRUOP_GROUP|%1,%2").arg(memberItem->data().toString()).arg(groupItem->text());
                qDebug()<< move_friend_send;
                tcpSocket->write(move_friend_send.toUtf8().data());
                qDebug()<<"send finished";
            }



            parentItem->removeRow(memberItem->row());

            groupItem->appendRow(Item);



        }

    }
//删除组，需要数据库
void ClientMainInterface::DropGroup(const QModelIndex &groupIndex,QStandardItemModel  *model){
    QStandardItem *groupItem = model->itemFromIndex(groupIndex);
    if(groupItem->text()=="未分组"){
        QMessageBox errorMessage;
        errorMessage.setIcon(QMessageBox::Critical);
        errorMessage.setText("错误");
        errorMessage.setInformativeText("不可删除");
        errorMessage.setWindowTitle("Error");
        errorMessage.exec();
    }
    else{
        int row_0=0;//“未分组索引”
        for (row_0 = 0; row_0 < model->rowCount(); ++row_0) {
                QModelIndex index = model->index(row_0, 0);
                if (index.isValid()) {
                    QStandardItem *item = model->itemFromIndex(index);
                    if (item && item->text() == "未分组") {
                        break;
                    }
                }
            }
        QStandardItem *group_none = model->itemFromIndex(model->index(row_0,0));

        QStringList friendslist;
        for (int row = 0; row < groupItem->rowCount(); ++row) {
               QStandardItem *memberItem = groupItem->child(row);
               if (memberItem) {
                   QString memberText = memberItem->text();
                   QStandardItem *Item=new QStandardItem(memberText);
                   Item->setData(memberItem->data().toString());
                   friendslist.append(QString("%1,未分组").arg(memberItem->data().toString()));


                   groupItem->removeRow(memberItem->row());


                   group_none->appendRow(Item);
                   row--;
               }
           }
        if(model==m_pModel){
            QString move_friend_send="FRIEND_GROUP|" + friendslist.join("|");
            tcpSocket->write(move_friend_send.toUtf8().data());
            qDebug()<<"send message:"<<move_friend_send;
        }
        else{
            QString move_friend_send="GROUP_GROUP|" + friendslist.join("|");
            tcpSocket->write(move_friend_send.toUtf8().data());
            qDebug()<<"send message:"<<move_friend_send;
        }

        model->removeRow(groupItem->row());
    }
}


// 处理从服务器接收的数据


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

ClientMainInterface::~ClientMainInterface()
{
    delete ui;
}

void ClientMainInterface::on_pushButton_clicked()//添加好友
{
    bool ok;
    QString text = QInputDialog::getText(nullptr, "Input Dialog", "输入好友账号:", QLineEdit::Normal, "", &ok);//测试：ADD_FRIEND_SUCCESS:姓名|账户



    QString add_friend_send=QString("FRIEND_ADD|%1").arg(text);//此text为单纯账户信息
    tcpSocket->write(add_friend_send.toUtf8().data());
    qDebug()<<add_friend_send;
    qDebug()<<"send finished";
}

void ClientMainInterface::on_pushButton_2_clicked()
{
//    QString receiver=item_record->data().toString();


//    QString fileName = QFileDialog::getOpenFileName(this);
//    QFile* localFile = new QFile(fileName);
//    if(!localFile->open(QFile::ReadOnly))
//    {
//        qDebug() << "open file error!";
//        return;
//    }

//    //文件总大小
//    qint64 totalBytes = localFile->size();
//    QByteArray outBlock;  //数据缓冲区，即存放每次要发送的数据
//    qint64 bytesToWrite;   //剩余数据大小
//    qint64 numBytes; // 上次发送数据的大小
//    qint64 receiver_size=receiver.size();//--接收方用户名的大小

//    QDataStream sendOut(&outBlock,QIODevice::WriteOnly);
//    sendOut.setVersion(QDataStream::Qt_4_6);
//    // fileName = "d://input//a.txt";
//    QString currentFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);//--我不确这里是否要修改，文件名怎恶魔找到的

//    //依次写入总大小信息空间，文件名大小信息空间，文件名
//    sendOut << qint64(0) << qint64(0) << currentFileName<<receiver;//--传入接收方用户名

//    //这里的总大小是文件名大小等信息和实际文件大小的总和

//    totalBytes += outBlock.size();
//    totalBytes +=receiver_size;//--加上用户名大小
//    sendOut.device()->seek(0);
//    //返回outBolock的开始，用实际的大小信息代替两个qint64(0)空间
//    sendOut<<totalBytes<<qint64((outBlock.size() - sizeof(qint64)*2));

//    //发送完头数据后剩余数据的大小
//    bytesToWrite = totalBytes - tcpSocket->write(outBlock);

//    //ui->label->setText(tr("已连接"));
//    numBytes = outBlock.size();
//    outBlock.resize(0);
//    qDebug()<<QString(outBlock) ;
    QString filepath=QFileDialog::getOpenFileName(this,"open","../");
        if(false==filepath.isEmpty())//文件路径有效
        {
            filename.clear();
            filesize=0;
            send_size=0;
            //获取文件信息
            QFileInfo info(filepath);
            filename=info.fileName();
            filesize=info.size();

            //只读打开文件
            file.setFileName(filepath);
            if(!file.open(QIODevice::ReadOnly))
            {
                qDebug()<<"打开文件失败";
            }
            //------------添加信息
//            QListWidgetItem *pItem = new QListWidgetItem;

//            QDateTime datetime = QDateTime::currentDateTime();
//            QString date = datetime.toString("hh:mm:ss");
//            pItem->setText("[" + date + "]: 文件等待发送.." );
//            pItem->setTextAlignment(Qt::AlignRight);
//            ui->read_list->addItem(pItem);
//            //---------
            ui->pushButton_2->setEnabled(false);
            ui->pushButton_4->setEnabled(true);
        }
        else
        {
            qDebug()<<"文件路径出错";
        }
}



void ClientMainInterface::on_treeView_clicked(const QModelIndex &index)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
    QStandardItem *clickedItem = model->itemFromIndex(index);
    if(clickedItem->parent()){
        ui->pushButton_3->show();
        ui->pushButton_2->show();
        ui->plainTextEdit->show();
        ui->pushButton_4->show();

        if(ui->horizontalSpacer){
            ui->tab->layout()->removeItem(ui->horizontalSpacer);
            //delete ui->horizontalSpacer;
            ui->horizontalSpacer=nullptr;
        }
        if (!clickedItem)
            return;
        item_record=clickedItem;
        // 如果之前没有创建过对应的 QListWidget，则创建一个
        if (!itemToListWidget.contains(clickedItem))
        {
            QListWidget *newListWidget = new QListWidget(this);
            itemToListWidget.insert(clickedItem, newListWidget);
            // 设置大小和位置
            newListWidget->setGeometry(221,40,660,400);
            QString ask_message = QString("CHAT_REQUEST|%1").arg(clickedItem->data().toString());
            tcpSocket->write(ask_message.toUtf8());
            qDebug()<<"第一次";
        }
//        else{
//            itemToListWidget.value( clickedItem)->clear();
//        }
        // 隐藏之前的 QListWidget，显示当前点击项对应的 QListWidget
        foreach (QListWidget *widget, itemToListWidget)
        {
            widget->hide();
        }
        QLayout *layoutToDelete = ui->tab->layout();
        qDebug()<<"here"<< layoutToDelete->count();


        for (int i = 0; i < layoutToDelete->count(); ++i) {
               QLayoutItem *item = layoutToDelete->itemAt(i);
               QWidget *widget = item->widget();
                layoutToDelete->removeWidget(widget);
        }



        qDebug()<<"here"<< layoutToDelete->count();
        delete layoutToDelete;


        ui->tab->setLayout(nullptr);//清空所有layout;
        QHBoxLayout *layout = new QHBoxLayout;//水平布局
        layout->addWidget(ui->treeView);
        layout->addWidget(itemToListWidget[clickedItem]);
        layout->setStretch(0,1);//0:索引：treeview。1：伸展比例
        layout->setStretch(1,4);
        ui->tab->setLayout(layout);

        itemToListWidget[clickedItem]->show();
   }
}

void ClientMainInterface::on_pushButton_3_clicked()
{
    QString message=ui->plainTextEdit->toPlainText();
        //由于点击之前button不会出现，所以item_record一定非空
        QString sender=account;
        QString receiver=item_record->data().toString();
        QDateTime currentDateTime=QDateTime::currentDateTime();
        QString string_currentDateTime=currentDateTime.toString("yyyy-MM-dd HH:mm:ss");

        if(ui->tabWidget->currentIndex()==0){
            QString sent = QString("CHAT_PERSON|%1$%2$%3$%4").arg(sender).arg(receiver).arg(message).arg(string_currentDateTime);
            tcpSocket->write(sent.toUtf8());
            qDebug()<<"have sented:"<<sent;
            QString listItem = QString("[%1] [%2 -> %3]: %4").arg(string_currentDateTime).arg(sender).arg(receiver).arg(message);
            //qDebug() << "Loaded chat record:" << listItem;
             QListWidgetItem *listWidgetItem = new QListWidgetItem(listItem);
             if(sender==account)
                 listWidgetItem->setTextAlignment(Qt::AlignRight);
             QListWidget *associatedListWidget = itemToListWidget.value(item_record);
             associatedListWidget->addItem(listWidgetItem);
        }
        else{
            QString sent = QString("CHAT_GROUP|%1$%2$%3$%4").arg(sender).arg(receiver).arg(message).arg(string_currentDateTime);
            tcpSocket->write(sent.toUtf8());
            qDebug()<<"have sented:"<<sent;
          //  QString listItem = QString("[%1] [%2 -> %3]: %4").arg(string_currentDateTime).arg(sender).arg(receiver).arg(message);
            //qDebug() << "Loaded chat record:" << listItem;
           //  QListWidgetItem *listWidgetItem = new QListWidgetItem(listItem);
           //  QListWidget *associatedListWidget = itemToListWidget_group.value(item_record);
           //  associatedListWidget->addItem(listWidgetItem);
        }
        ui->plainTextEdit->clear();
}

void ClientMainInterface::on_pushButton_4_clicked()
{


     QString receiver=item_record->data().toString();
    //先发送文件头信息 文件名##文件大小
        QString head=QString("%1##%2").arg(filename).arg(filesize);
        qDebug()<<"send filesize"<<filesize;
        qint64 len=filesocket->write(head.toUtf8());//一定要注意write调用者是filesocket
        if(len>0)//t头部发送成功
        {
            //发送真正的文件信息
            //为了防止粘包现象，需要定时器延长时间20ms
            qDebug()<<"t头部发送成功";
            timer.start(100);
            //sendfile();
        }
        else
        {
            qDebug()<<"头部信息发送失败";
            file.close();
            ui->pushButton_2->setEnabled(true);
            ui->pushButton_4->setEnabled(false);
            //QListWidgetItem *pItem = new QListWidgetItem;

//            QDateTime datetime = QDateTime::currentDateTime();
//            QString date = datetime.toString("hh:mm:ss");
//            pItem->setText("[" + date + "]: 文件发送失败" );
//            pItem->setTextAlignment(Qt::AlignRight);

//            ui->read_list->addItem(pItem);
        }
}

void ClientMainInterface::on_pushButton_5_clicked()
{
    bool ok;
        QString text = QInputDialog::getText(nullptr, "Input Dialog", "输入群:", QLineEdit::Normal, "", &ok);//测试：ADD__SUCCESS:群名|id



        QString add_friend_send=QString("GROUP_ADD|%1").arg(text);//此text为单纯账户信息
        tcpSocket->write(add_friend_send.toUtf8().data());
        qDebug()<<add_friend_send;
        qDebug()<<"send finished";
}

void ClientMainInterface::on_treeView_2_clicked(const QModelIndex &index)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeView_2->model());
        QStandardItem *clickedItem = model->itemFromIndex(index);
        if(clickedItem->parent()){
            ui->pushButton_3->show();
            ui->pushButton_2->show();
            ui->plainTextEdit->show();
            ui->pushButton_4->show();

            if(ui->horizontalSpacer_2){
                ui->tab->layout()->removeItem(ui->horizontalSpacer_2);
                //delete ui->horizontalSpacer;
                ui->horizontalSpacer_2=nullptr;
            }

            if (!clickedItem)
                return;
            item_record=clickedItem;
            // 如果之前没有创建过对应的 QListWidget，则创建一个
            if (!itemToListWidget_group.contains(clickedItem))
            {

                QListWidgetItem *listWidgetItem = new QListWidgetItem(clickedItem->text());

                QListWidget *newListWidget = new QListWidget(this);

                newListWidget->addItem(listWidgetItem);

                itemToListWidget_group.insert(clickedItem, newListWidget);
                // 设置大小和位置
                newListWidget->setGeometry(221,40,660,400);
                newListWidget->setParent(ui->tab_2);


                QString ask_message = QString("GROUP_CHAT_REQUEST|%1").arg(clickedItem->data().toString());
                tcpSocket->write(ask_message.toUtf8());
                qDebug()<<"第一次";
               // delete ui->horizontalSpacer;
            }
    //        else{
    //            itemToListWidget.value( clickedItem)->clear();
    //        }
            // 隐藏之前的 QListWidget，显示当前点击项对应的 QListWidget
            foreach (QListWidget *widget, itemToListWidget_group)
            {
                widget->hide();
            }


            QLayout *layoutToDelete = ui->tab_2->layout();
            qDebug()<<"here"<< layoutToDelete->count();


            for (int i = 0; i < layoutToDelete->count(); ++i) {
                   QLayoutItem *item = layoutToDelete->itemAt(i);
                   QWidget *widget = item->widget();
                    layoutToDelete->removeWidget(widget);
            }



            qDebug()<<"here"<< layoutToDelete->count();
            delete layoutToDelete;


            ui->tab_2->setLayout(nullptr);//清空所有layout;
            QHBoxLayout *layout = new QHBoxLayout;//水平布局
            layout->addWidget(ui->treeView_2);
            layout->addWidget(itemToListWidget_group[clickedItem]);
            layout->setStretch(0,1);//0:索引：treeview。1：伸展比例
            layout->setStretch(1,4);
            ui->tab_2->setLayout(layout);


            itemToListWidget_group[clickedItem]->show();
       }
}

void ClientMainInterface::on_tabWidget_tabBarClicked(int index)
{
    ui->pushButton_3->hide();
    ui->pushButton_2->hide();
    ui->plainTextEdit->hide();
    ui->pushButton_4->hide();
}

void ClientMainInterface::on_pushButton_6_clicked()//创建群

{
    bool ok;
        QString gro = QInputDialog::getText(nullptr, "Input Dialog", "输入群名|群id:", QLineEdit::Normal, "", &ok);
        if(gro=="")
                return;
        QString id=gro.split("|")[1];
        QString g_name=gro.split("|")[0];
        int row_0=0;//组的索引
        for (row_0 = 0; row_0 < m_pModel_group->rowCount(); ++row_0) {
                QModelIndex index = m_pModel_group->index(row_0, 0);
                if (index.isValid()) {
                    QStandardItem *item = m_pModel_group->itemFromIndex(index);
                    if (item && item->text() == "未分组") {
                        break;
                    }
                }
            }
        QStandardItem *group_none = m_pModel_group->itemFromIndex(m_pModel_group->index(row_0,0));
        QStandardItem *Item=new QStandardItem(g_name);
        Item->setData(id);
        //qDebug()<<Item->data().toInt();
        group_none->appendRow(Item);
        Item->setEditable(false);
        QString ask_message = QString("GROUP_CREATE|%1|%2|%3").arg(id).arg(g_name).arg(account);
        tcpSocket->write(ask_message.toUtf8());
        qDebug()<<"done"<<ask_message;
}
