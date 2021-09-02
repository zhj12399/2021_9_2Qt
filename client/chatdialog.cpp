#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <userinfo.h>
#include <home.h>
#include <client.h>

extern userinfo user;
extern bool is_open_chatdialog;
extern userinfo otheruser;
extern QString hostip;
extern int hosthost;

chatdialog::chatdialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::chatdialog)
{
    ui->setupUi(this);
    ui->label_title->setText("您正在与"+otheruser.name+"进行对话");
    is_open_chatdialog = true;
    tcpSocket = new QTcpSocket();
    timer = new QTimer();

    qDebug()<<otheruser.id<<otheruser.name;
    connect(timer,SIGNAL(timeout()),this,SLOT(getchathistory()));
    timer->start(500);
}

chatdialog::~chatdialog()
{
    is_open_chatdialog = false;
    timer->stop();
    delete ui;
}

void chatdialog::on_pushButton_send_clicked()
{//发送消息
    if(ui->lineEdit_sendmessage->text()!="")
    {
        tcpSocket->abort();//取消已有链接
        tcpSocket->connectToHost(hostip, hosthost);//链接服务器
        QString ip = tcpSocket->peerAddress().toString().section(":",3,3);
        int port = tcpSocket->peerPort();
        QString str = QString("[%1:%2]").arg(ip).arg(port);
        qDebug() << str ;
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
            //第一个是时间，第二个发送的ID，第三个是接受的ID，第四个是内容
            QString nowstr = QDateTime::currentDateTime().currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
            QString message = QString("chat_send##%1##%2##%3##%4").arg(nowstr).arg(user.id).arg(otheruser.id).arg(ui->lineEdit_sendmessage->text());
            tcpSocket->write(message.toUtf8());
            tcpSocket->flush();
            ui->lineEdit_sendmessage->clear();
        }
    }
    else
    {//空消息
        QMessageBox::warning(this, "Warning!", "不能发送空消息", QMessageBox::Yes);
    }
}

void chatdialog::on_pushButton_clicked()
{
    qDebug()<<is_open_chatdialog;

    is_open_chatdialog = false;
    qDebug()<<is_open_chatdialog;
    this->close();
    timer->stop();
}

void chatdialog::getchathistory()
{
    tcpSocket->abort();//取消已有链接
    tcpSocket->connectToHost(hostip, hosthost);//链接服务器

    if(!tcpSocket->waitForConnected(30000))
    {
        QMessageBox::warning(this, "Warning!", "网络错误", QMessageBox::Yes);
        this->close();
    }
    else
    {//服务器连接成功
        QString message = QString("chat_history##%1##%2").arg(user.id).arg(otheruser.id);
        tcpSocket->write(message.toUtf8());
        tcpSocket->flush();
        connect(tcpSocket,&QTcpSocket::readyRead,[=](){
            QByteArray buffer = tcpSocket->readAll();
            if(QString(buffer).section("##",0,0)==QString("chat_history_ok"))
            {
                QString chatshow = "";
                int num = QString(buffer).section("##",1,1).toInt();
                for(int rownum = 0;rownum < num ;rownum++)
                {
                    QDateTime time = QDateTime::fromString( QString(buffer).section("##",rownum*3+2,rownum*3+2),"yyyy-MM-dd hh:mm:ss.zzz");
                    qDebug()<<time.toString();
                    QString timeshow = time.toString("MM-dd hh:mm:ss");
                    qDebug()<<timeshow;
                    QString idshow = "";
                    if(QString(buffer).section("##",rownum*3+3,rownum*3+3).toInt()==user.id)
                    {//我自己发送的消息
                        idshow = " 我：";
                    }
                    else
                    {
                        idshow =" "+ otheruser.name + "：";
                    }
                    chatshow = "("+timeshow+")" + idshow + QString(buffer).section("##",rownum*3+4,rownum*3+4) +"\n" + chatshow;
                }
                ui->textBrowser_chat->setText(chatshow);
            }
            else if(QString(buffer).section("##",0,0)==QString("chat_history_error"))
            {
                ui->textBrowser_chat->setText("无消息记录");
            }
        });
    }
}

void chatdialog::closeEvent(QCloseEvent *event)
{
    is_open_chatdialog = false;
    timer->stop();
}


