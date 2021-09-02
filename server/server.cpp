#include "server.h"
#include "ui_server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

int currentsize;

server::server(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::server)
{
    ui->setupUi(this);
    tcpServer = new QTcpServer();

    ui->listWidget->clear();
    ui->listWidget->insertItem(0,tr("当前无在线用户"));
    for(int i = 0; i < M; i++)
    {
        tcpSocket[i]=new QTcpSocket();
    }
    tcpServer->listen(QHostAddress::Any,8888);

    //init
    db = QSqlDatabase::addDatabase("QSQLITE");

    //判断是否建立了用户表
    db.setDatabaseName("./people.db");
    db.open();
    QSqlQuery sqlquery;
    sqlquery.exec("CREATE TABLE if not exists people (id INTEGER NOT NULL UNIQUE,name TEXT NOT NULL UNIQUE,password TEXT NOT NULL,ip TEXT,islogin INTEGER NOT NULL,PRIMARY KEY(id AUTOINCREMENT))");
    db.close();

    connect(tcpServer,&QTcpServer::newConnection,[=](){

        tcpSocket[0] = tcpServer->nextPendingConnection();
        currentsize++;
        QString ip = tcpSocket[0]->peerAddress().toString().section(":",3,3);
        int port = tcpSocket[0]->peerPort();
        //预留currentsize以用作多用户同时连接所用
        QString str = QString("[%1:%2]").arg(ip).arg(port);
        qDebug() << str ;

        connect(tcpSocket[0],&QTcpSocket::readyRead,[=](){
            //读取缓冲区数据
            QByteArray buffer = tcpSocket[0]->readAll();

            if("login" == QString(buffer).section("##",0,0))
            {//登陆
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare("select * from people where name = :name");
                sqlquery.bindValue(":name",QString(buffer).section("##",1,1));
                sqlquery.exec();
                if(!sqlquery.next())
                {//未查找到该用户
                    tcpSocket[0]->write(QString("login error##no_user").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
                else
                {//用户存在
                    int id = sqlquery.value(0).toInt();
                    QString pwd = sqlquery.value(2).toString();
                    if(pwd == QString(buffer).section("##",2,2))
                    {//登录成功
                        tcpSocket[0]->write(QString("login successed##%1").arg(id).toUtf8());
                        sqlquery.prepare("update people set ip=:ip, islogin=1 where name = :name");
                        sqlquery.bindValue(":ip",ip);
                        sqlquery.bindValue(":name",QString(buffer).section("##",1,1));
                        sqlquery.exec();
                        tcpSocket[0]->flush();

                        //更新服务器界面
                        ui->listWidget->clear();
                        sqlquery.prepare("select * from people where islogin = 1");
                        sqlquery.exec();
                        if(sqlquery.next())
                        {
                            QString userid = sqlquery.value(0).toString();
                            QString username = sqlquery.value(1).toString();
                            QString userip = sqlquery.value(3).toString();
                            //qDebug()<<userid;
                            ui->listWidget->insertItem(0,"用户ID："+userid+",用户昵称:"+username+",用户IP:"+userip);
                            int rownum = 1;
                            while (sqlquery.next())
                            {
                                QString userid = sqlquery.value(0).toString();
                                QString username = sqlquery.value(1).toString();
                                QString userip = sqlquery.value(3).toString();
                                ui->listWidget->insertItem(rownum,"用户ID："+userid+",用户昵称:"+username+",用户IP:"+userip);
                                rownum++;
                            }
                        }
                        else
                        {
                            ui->listWidget->clear();
                            ui->listWidget->insertItem(0,tr("当前无在线用户"));
                        }
                    }
                    else
                    {//密码错误
                        tcpSocket[0]->write(QString("login error##errpwd").toUtf8());
                        tcpSocket[0]->flush();
                        db.close();
                    }
                }
            }
            else if("register" == QString(buffer).section("##",0,0))
            {//注册环节
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                //注册用户的时候需要进行判重
                sqlquery.prepare("select * from people where name = :name");
                sqlquery.bindValue(":name",QString(buffer).section("##",1,1));
                sqlquery.exec();
                if(!sqlquery.next())
                {//可以新建
                    sqlquery.clear();
                    sqlquery.prepare("insert into people values (null,:name,:password,null,0)");
                    sqlquery.bindValue(":name",QString(buffer).section("##",1,1));
                    sqlquery.bindValue(":password",QString(buffer).section("##",2,2));
                    sqlquery.exec();

                    sqlquery.clear();
                    sqlquery.prepare("select * from people where name = :name");
                    sqlquery.bindValue(":name",QString(buffer).section("##",1,1));
                    sqlquery.exec();//获得新建的用户的id
                    sqlquery.next();
                    int newid = sqlquery.value(0).toInt();
                    sqlquery.exec("create table if not exists friend__" + QString::number(newid) +"(id INTEGER unique, name TEXT,sendmassage INTEGER,sendfile INTEGER)");

                    tcpSocket[0]->write(QString("register successed").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
                else
                {//有重名
                     tcpSocket[0]->write(QString("register error##same_name").toUtf8());
                     tcpSocket[0]->flush();
                     db.close();
                }
            }
            else if("wantsendmessage" == QString(buffer).section("##",0,0))
            {//想发信息，校验有没有这个人
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare("select * from people where name = :name");
                sqlquery.bindValue(":name",QString(buffer).section("##",2,2));
                sqlquery.exec();
                if(sqlquery.next())
                {//有这个人在，可以发消息
                    int otherid = sqlquery.value(0).toInt();
                    tcpSocket[0]->write(QString("wantsendmessage_ok##%1").arg(otherid).toUtf8());
                    tcpSocket[0]->flush();

                    //发消息前把数据库准备好
                    int thisid = QString(buffer).section("##",1,1).toInt();
                    if(thisid < otherid)
                    {
                        sqlquery.exec("CREATE TABLE if not exists chat__" + QString::number(thisid) + "__" + QString::number(otherid) + "(time datetime NOT NULL UNIQUE,id INTEGER,message TEXT, PRIMARY KEY(time))");
                    }
                    else
                    {
                        sqlquery.exec("CREATE TABLE if not exists chat__" + QString::number(otherid) + "__" + QString::number(thisid) + "(time datetime NOT NULL UNIQUE,id INTEGER,message TEXT, PRIMARY KEY(time))");
                    }


                    db.close();
                }
                else
                {//查无此人，无法对话
                    tcpSocket[0]->write(QString("wantsendmessage_error").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
            }
            else if("chat_history" == QString(buffer).section("##",0,0))
            {//idone是发信息的，idtwo是收信息的，所以one已经阅览了two的所有消息
                int idone = QString(buffer).section("##",1,1).toInt();
                int idtwo = QString(buffer).section("##",2,2).toInt();
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                QString sqlstring = "";
                if(idone < idtwo)
                {
                    sqlstring = "select * from chat__" + QString::number(idone) + "__" + QString::number(idtwo) + " order by time desc limit 20";
                }
                else
                {
                    sqlstring = "select * from chat__" + QString::number(idtwo) + "__" + QString::number(idone) + " order by time desc limit 20";
                }
                qDebug()<<sqlstring;
                sqlquery.exec(sqlstring);

                if(sqlquery.next())
                {
                    QString history = "##" + sqlquery.value(0).toDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")+ "##" + sqlquery.value(1).toString()+ "##" + sqlquery.value(2).toString();
                    int hisnum = 1;
                    while(sqlquery.next())
                    {
                        hisnum++;
                        QDateTime time = sqlquery.value(0).toDateTime();
                        QString timestr = time.toString("yyyy-MM-dd hh:mm:ss.zzz");
                        history = history + "##" + timestr;//时间
                        history = history + "##" + sqlquery.value(1).toString();//谁发的
                        history = history + "##" + sqlquery.value(2).toString();//内容
                    }
                    history = "chat_history_ok##" + QString::number(hisnum) + history;
                    tcpSocket[0]->write(history.toUtf8());
                    tcpSocket[0]->flush();

                    QString sqlstring = "update friend__" + QString::number(idone) + " set sendmassage = 0 where id = :id";
                    db.setDatabaseName("./people.db");
                    db.open();
                    QSqlQuery sqlquery;
                    sqlquery.prepare(sqlstring);
                    sqlquery.bindValue(":id", idtwo);
                    sqlquery.exec();
                    db.close();
                }
                else
                {//无历史记录
                    tcpSocket[0]->write(QString("chat_history_error").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
            }
            else if("chat_send" == QString(buffer).section("##",0,0))
            {//one发消息的，two收消息的
                QDateTime nowstr = QDateTime::fromString(QString(buffer).section("##",1,1), "yyyy-MM-dd hh:mm:ss.zzz");
                int idone = QString(buffer).section("##",2,2).toInt();
                int idtwo = QString(buffer).section("##",3,3).toInt();
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                QString sqlstring = "";
                if(idone < idtwo)
                {
                    sqlstring = "insert into chat__" + QString::number(idone) + "__" + QString::number(idtwo) + " values(:time,:id,:message)";
                }
                else
                {
                    sqlstring = "insert into chat__" + QString::number(idtwo) + "__" + QString::number(idone) + " values(:time,:id,:message)";
                }
                qDebug()<<sqlstring;
                sqlquery.prepare(sqlstring);
                sqlquery.bindValue(":time",nowstr);
                sqlquery.bindValue(":id",idone);
                sqlquery.bindValue(":message",QString(buffer).section("##",4,4));
                sqlquery.exec();

                sqlstring = "update friend__" + QString::number(idtwo) + " set sendmassage = 1 where id = :id";
                db.setDatabaseName("./people.db");
                db.open();
                sqlquery.clear();
                sqlquery.prepare(sqlstring);
                sqlquery.bindValue(":id", idone);
                sqlquery.exec();
                db.close();
            }
            else if("logout" == QString(buffer).section("##",0,0))
            {
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare("update people set islogin=0 where id = :id");
                sqlquery.bindValue(":id",QString(buffer).section("##",1,1));
                sqlquery.exec();

                //更新服务器界面
                ui->listWidget->clear();
                sqlquery.prepare("select * from people where islogin = 1");
                sqlquery.exec();
                if(sqlquery.next())
                {
                    QString userid = sqlquery.value(0).toString();
                    QString username = sqlquery.value(1).toString();
                    QString userip = sqlquery.value(3).toString();
                    //qDebug()<<userid;
                    ui->listWidget->insertItem(0,"用户ID："+userid+",用户昵称:"+username+",用户IP:"+userip);
                    int rownum = 1;
                    while (sqlquery.next())
                    {
                        QString userid = sqlquery.value(0).toString();
                        QString username = sqlquery.value(1).toString();
                        QString userip = sqlquery.value(3).toString();
                        ui->listWidget->insertItem(rownum,"用户ID："+userid+",用户昵称:"+username+",用户IP:"+userip);
                        rownum++;
                    }
                }
                else
                {
                    ui->listWidget->clear();
                    ui->listWidget->insertItem(0,tr("当前无在线用户"));
                }
            }
            else if("getfriendlist" == QString(buffer).section("##",0,0))
            {
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.exec("select * from friend__" + QString(buffer).section("##",1,1) + " desc");

                if(sqlquery.next())
                {
                    QList <QString> friendlist;
                    QList <QString> friendsendfilelist;
                    QList <QString> friendsendmassagelist;
                    friendlist.append(sqlquery.value(1).toString());

                    int sendmassagenum = 0;
                    if(sqlquery.value(2).toString() == '1')
                    {
                        sendmassagenum++;
                    }
                    friendsendmassagelist.append(sqlquery.value(2).toString());
                    int sendfilenum = 0;
                    if(sqlquery.value(3).toString() == '1')
                    {
                        sendfilenum++;
                    }
                    friendsendfilelist.append(sqlquery.value(3).toString());

                    QString friends = "";
                    while(sqlquery.next())
                    {
                        friendlist.append(sqlquery.value(1).toString());
                        if(sqlquery.value(2).toString() == '1')
                        {
                            sendmassagenum++;
                        }
                        friendsendmassagelist.append(sqlquery.value(2).toString());
                        if(sqlquery.value(3).toString() == '1')
                        {
                            sendfilenum++;
                        }
                        friendsendfilelist.append(sqlquery.value(3).toString());
                    }

                    int onlinefriendnum = 0;
                    for( int i = 0; i < friendlist.length(); i++)
                    {
                        sqlquery.prepare("select * from people where name = :name");
                        sqlquery.bindValue(":name",friendlist.at(i));
                        sqlquery.exec();
                        sqlquery.next();
                        QString peopleip = sqlquery.value(3).toString();
                        if(sqlquery.value(4).toInt() == 1)
                        {
                            onlinefriendnum++;
                            friends = "##" + friendlist.at(i) + "##1##" + peopleip +"##"+friendsendmassagelist.at(i)+"##"+ friendsendfilelist.at(i)+ friends;
                        }
                        else
                        {
                            friends = "##" + friendlist.at(i) + "##0##" + peopleip +"##"+friendsendmassagelist.at(i)+"##"+ friendsendfilelist.at(i)+ friends;
                        }
                    }
                    friends = "getfriendlist_ok##" + QString::number(friendlist.length()) +"##"+QString::number(onlinefriendnum)+"##"+QString::number(sendmassagenum)+"##"+QString::number(sendfilenum)+ friends;
                    qDebug()<<friends;
                    tcpSocket[0]->write(friends.toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
                else
                {//无朋友
                    tcpSocket[0]->write(QString("getfriendlist_error").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
            }
            else if("add_friend" == QString(buffer).section("##",0,0))
            {
                int whowantadd_id = QString(buffer).section("##",1,1).toInt();
                QString friend_name = QString(buffer).section("##",2,2);

                qDebug() << whowantadd_id << friend_name;

                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare("select * from people where name = :name");
                sqlquery.bindValue(":name",friend_name);
                sqlquery.exec();
                if(sqlquery.next())
                {
                    int friend_id = sqlquery.value(0).toInt();
                    qDebug() <<friend_id;
                    sqlquery.clear();
                    QString sqlstring = "insert into friend__" + QString::number(whowantadd_id) + " values(:id,:name,0,0)";
                    qDebug()<<sqlstring;
                    sqlquery.prepare(sqlstring);
                    sqlquery.bindValue(":id",friend_id);
                    sqlquery.bindValue(":name",friend_name);
                    sqlquery.exec();
                    qDebug()<<sqlquery.lastError();
                    tcpSocket[0]->write(QString("add_friend_ok").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
                else
                {//没这人
                    qDebug() <<"e";
                    tcpSocket[0]->write(QString("add_friend_error").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
            }
            else if("delete_friend" == QString(buffer).section("##",0,0))
            {
                int whowantdelete_id = QString(buffer).section("##",1,1).toInt();
                QString friend_name = QString(buffer).section("##",2,2);

                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare("delete from friend__"+QString::number(whowantdelete_id)+" where name = :name");
                sqlquery.bindValue(":name",friend_name);
                if(sqlquery.exec())
                {
                    tcpSocket[0]->write(QString("delete_friend_ok").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
                else
                {
                    tcpSocket[0]->write(QString("delete_friend_error").toUtf8());
                    tcpSocket[0]->flush();
                    db.close();
                }
            }
            else if("want_send_file" == QString(buffer).section("##",0,0))
            {//1给2发,1是id,2是name;在2的表中显示1的信息
                int userid = QString(buffer).section("##",1,1).toInt();
                qDebug()<<buffer;
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare("select * from people where name = :name");
                sqlquery.bindValue(":name",QString(buffer).section("##",2,2));
                sqlquery.exec();
                sqlquery.next();
                int otheruserid = sqlquery.value(0).toInt();

                QString sqlstring = "update friend__" + QString::number(otheruserid) + " set sendfile = 1 where id = :id";
                sqlquery.prepare(sqlstring);
                sqlquery.bindValue(":id",userid);
                sqlquery.exec();
            }
            else if("send_file_ok" == QString(buffer).section("##",0,0))
            {//1给2发,1是id,2是name;在2的表中显示1的信息
                int userid = QString(buffer).section("##",1,1).toInt();
                qDebug()<<buffer;
                QString sqlstring = "update friend__" + QString::number(userid) + " set sendfile = 0 where name = :name";
                qDebug()<<sqlstring;
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare(sqlstring);
                sqlquery.bindValue(":name", QString(buffer).section("##",2,2));
                sqlquery.exec();
            }
            else if("send_file_miss" == QString(buffer).section("##",0,0))
            {
                int userid = QString(buffer).section("##",1,1).toInt();
                qDebug()<<buffer;
                db.setDatabaseName("./people.db");
                db.open();
                QSqlQuery sqlquery;
                sqlquery.prepare("select * from people where name = :name");
                sqlquery.bindValue(":name",QString(buffer).section("##",2,2));
                sqlquery.exec();
                sqlquery.next();
                int otheruserid = sqlquery.value(0).toInt();

                QString sqlstring = "update friend__" + QString::number(otheruserid) + " set sendfile = 0 where id = :id";
                sqlquery.prepare(sqlstring);
                sqlquery.bindValue(":id",userid);
                sqlquery.exec();
            }
        });
    });
}

server::~server()
{
    tcpServer->close();
    tcpServer->deleteLater();
    delete ui;
}
