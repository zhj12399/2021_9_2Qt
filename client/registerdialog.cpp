#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <userinfo.h>
#include <clientserve.h>

extern userinfo user;
extern QString hostip;
extern int hosthost;
registerdialog::registerdialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::registerdialog)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket();
}

registerdialog::~registerdialog()
{
    delete ui;
}

void registerdialog::on_pushButton_register_clicked()
{
    if(ui->lineEdit_name->text()!=""&& ui->lineEdit_passwordone->text()!=""&& ui->lineEdit_passwordtwo->text()!="")
    {
        if(ui->lineEdit_passwordone->text()==ui->lineEdit_passwordtwo->text())
        {
            tcpSocket->abort();//取消已有链接
            tcpSocket->connectToHost(hostip, hosthost);//链接服务器

            if(!tcpSocket->waitForConnected(30000))
            {
                QMessageBox::warning(this, "Warning!", "网络错误", QMessageBox::Yes);
                this->close();
                user.islogin = false;
                client *cli = new client();
                cli->show();
            }
            else
            {//服务器连接成功
                QString loginmessage = QString("register##%1##%2").arg(ui->lineEdit_name->text()).arg(ui->lineEdit_passwordone->text());
                tcpSocket->write(loginmessage.toUtf8());
                tcpSocket->flush();
                QString ip = tcpSocket->peerAddress().toString().section(":",3,3);
                int port = tcpSocket->peerPort();
                QString str = QString("[%1:%2]").arg(ip).arg(port);
                qDebug() << str ;
                connect(tcpSocket,&QTcpSocket::readyRead,[=](){
                    QByteArray buffer = tcpSocket->readAll();
                    if(QString(buffer).section("##",0,0)==QString("register successed"))
                    {//注册成功
                        this->close();
                        client *cli = new client();
                        cli->show();
                    }
                    else if(QString(buffer).section("##",0,0)==QString("register error"))
                    {
                        if(QString(buffer).section("##",1,1)==QString("same_name"))
                        {
                            QMessageBox::warning(this, "Warning!", "昵称有重复", QMessageBox::Yes);
                            ui->lineEdit_name->clear();
                            ui->lineEdit_name->setFocus();
                        }
                    }
                });
            }
        }
        else
        {//两次密码不一样
            QMessageBox::warning(this, "Warning!", "两次密码不一样", QMessageBox::Yes);
            ui->lineEdit_passwordone->clear();
            ui->lineEdit_passwordtwo->clear();
            ui->lineEdit_passwordone->setFocus();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning!", "用户名或密码不为空", QMessageBox::Yes);
        ui->lineEdit_name->clear();
        ui->lineEdit_passwordone->clear();
        ui->lineEdit_passwordtwo->clear();
        ui->lineEdit_name->setFocus();
    }
}


void registerdialog::on_pushButton_back_clicked()
{
    this->close();
    client *cli = new client();
    cli->show();
}



void registerdialog::on_lineEdit_name_editingFinished()
{
     ui->label_name->setStyleSheet("color: rgb(0, 0, 0);font: 75 12pt Comic Sans MS;");
}


void registerdialog::on_lineEdit_name_textEdited(const QString &arg1)
{
     ui->label_name->setStyleSheet("color: rgb(255, 158, 2);font: 75 12pt Comic Sans MS;");
}


void registerdialog::on_lineEdit_passwordone_editingFinished()
{
    ui->label_passwordone->setStyleSheet("color: rgb(0, 0, 0);font: 75 12pt Comic Sans MS;");
}


void registerdialog::on_lineEdit_passwordone_textEdited(const QString &arg1)
{
    ui->label_passwordone->setStyleSheet("color: rgb(255, 158, 2);font: 75 12pt Comic Sans MS;");
}


void registerdialog::on_lineEdit_passwordtwo_editingFinished()
{
    ui->label_passwordtwo->setStyleSheet("color: rgb(0, 0, 0);font: 75 12pt Comic Sans MS;");
}


void registerdialog::on_lineEdit_passwordtwo_textEdited(const QString &arg1)
{
    ui->label_passwordtwo->setStyleSheet("color: rgb(255, 158, 2);font: 75 12pt Comic Sans MS;");
}

