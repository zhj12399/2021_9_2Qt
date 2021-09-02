#include "clientserve.h"
#include "ui_clientserve.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <userinfo.h>
#include <QMessageBox>
#include <client.h>

userinfo user;
QString clientmessage = "";
bool sendmessage = false;

clientserve::clientserve(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::clientserve)
{
    ui->setupUi(this);

    client *c;
    c->show();

    tcpSocket = new QTcpSocket(this); //与服务器交流
    tcpServer = new QTcpServer(this); //服务器也是一直打开的
    tcpSocket->abort();//取消已有链接
    tcpSocket->connectToHost(user.ip,user.port);//链接服务器
    connect(tcpSocket,&QTcpSocket::readyRead,
            [=]()
    {
         QByteArray buffer = tcpSocket->readAll(); //接收消息种类
         if(!tcpSocket->waitForConnected(30000))
         {
             QMessageBox::warning(this, "Warning!", "网络错误", QMessageBox::Yes);
         }
         else
         {//服务器连接成功
            if(sendmessage==true)
            {//需要发消息
                tcpSocket->write(clientmessage.toUtf8());
                tcpSocket->flush();
                sendmessage = false;
            }
            else
            {
                if(QString(buffer).section("##",0,0)==QString("login successed"))
                {//登陆成功
                    user.id=QString(buffer).section("##",1,1).toInt();
                    user.name = ui->lineEdit_username->text();
                    user.islogin = true;
                    this->close();
                    home *hom = new home();
                    hom->show();
                }
                else if(QString(buffer).section("##",0,0)==QString("login error"))
                {
                    if(QString(buffer).section("##",1,1)==QString("no_user"))
                    {//用户不存在
                        QMessageBox::warning(this, "Warning!", "用户不存在", QMessageBox::Yes);
                        ui->lineEdit_username->clear();
                        ui->lineEdit_pwd->clear();
                        ui->lineEdit_username->setFocus();
                    }
                    else if(QString(buffer).section("##",1,1)==QString("errpwd"))
                    {
                        QMessageBox::warning(this, "Warning!", "密码错误", QMessageBox::Yes);
                        ui->lineEdit_pwd->clear();
                        ui->lineEdit_pwd->setFocus();
                    }
                }
            }
         }
    });
}

clientserve::~clientserve()
{
    delete ui;
}
