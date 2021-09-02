#include "client.h"
#include "ui_client.h"
#include <QtNetwork>
#include <QMessageBox>
#include <userinfo.h>
#include <home.h>

extern userinfo user;
extern QString hostip;
extern int hosthost;

client::client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::client)
{
    ui->setupUi(this);
    tcpSocket = new QTcpSocket();
}

client::~client()
{
    delete tcpSocket;
    delete ui;
}


void client::on_pushButton_login_clicked()
{
    if(ui->lineEdit_username->text()!=""&& ui->lineEdit_pwd->text()!= "")
    {//账号密码不能为空
        tcpSocket->abort();//取消已有链接
        tcpSocket->connectToHost(hostip, hosthost);//链接服务器
        QString ip = tcpSocket->peerAddress().toString().section(":",3,3);
        int port = tcpSocket->peerPort();
        QString str = QString("[%1:%2]").arg(ip).arg(port);
        qDebug() << str ;
        if(!tcpSocket->waitForConnected(30000))
        {
            QMessageBox::warning(this, "Warning!", "网络错误", QMessageBox::Yes);
        }
        else
        {//服务器连接成功
            QString loginmessage = QString("login##%1##%2").arg(ui->lineEdit_username->text()).arg(ui->lineEdit_pwd->text());
            tcpSocket->write(loginmessage.toUtf8());
            tcpSocket->flush();
            connect(tcpSocket,&QTcpSocket::readyRead,[=](){
                QByteArray buffer = tcpSocket->readAll();
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
            });
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning!", "用户名或密码不为空", QMessageBox::Yes);
        ui->lineEdit_username->clear();
        ui->lineEdit_pwd->clear();
        ui->lineEdit_username->setFocus();
    }
}


void client::on_pushButton_register_clicked()
{
    this->close();
    registerdialog *reg = new registerdialog();
    reg->show();
}


void client::on_lineEdit_username_editingFinished()
{
    ui->label_username->setStyleSheet("color: rgb(0, 0, 0);font: 75 12pt Comic Sans MS;");
}


void client::on_lineEdit_username_textEdited(const QString &arg1)
{
    ui->label_username->setStyleSheet("color: rgb(255, 85, 255);font: 75 12pt Comic Sans MS;");
}


void client::on_lineEdit_pwd_editingFinished()
{
    ui->label_pwd->setStyleSheet("color: rgb(0, 0, 0);font: 75 12pt Comic Sans MS;");
}


void client::on_lineEdit_pwd_textEdited(const QString &arg1)
{
    ui->label_pwd->setStyleSheet("color: rgb(255, 85, 255);font: 75 12pt Comic Sans MS;");
}

