#include "receivefiledialog.h"
#include "ui_receivefiledialog.h"
#include <QMessageBox>
#include <userinfo.h>
#include <QFileDialog>
#include <qdir.h>

extern userinfo otheruser;
extern userinfo user;
extern int hosthost;
extern QString hostip;
extern bool is_open_chatdialog;
receivefiledialog::receivefiledialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::receivefiledialog)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    is_open_chatdialog = true;
    ui->label_message->setText("等待与" + otheruser.name + "连接");
    ip = otheruser.ip;
    qDebug() << ip;
    port = 9988;
    isStart = true;
    tcpSocket = new QTcpSocket(this);
    qDebug()<<ip<<port;
    tcpSocket->connectToHost(ip, port);
    connect(tcpSocket,&QTcpSocket::connected,[=](){
        ui->label_message->setText("连接成功，等待"+otheruser.name+"发送");
    });

    connect(tcpSocket,&QTcpSocket::readyRead,[=](){
        QByteArray buf = tcpSocket->readAll();
        if(isStart == true)
        {
            isStart = false;//识别头部信息
            fileName = QString(buf).section("##",0,0);
            fileSize = QString(buf).section("##",1,1).toInt();
            ui->label_message->setText("发现文件"+fileName);
            receiveSize = 0;

            //打开文件
            file.setFileName("./"+fileName);
            bool isOK = file.open(QIODevice::WriteOnly|QIODevice::Append);
            if(isOK == false)
            {
                tcpSocket->disconnectFromHost();
                tcpSocket->close();
                QMessageBox::information(this,"Error:","打开文件错误");
                return;
            }

            //初始化进度条
            ui->progressBar->setMinimum(0);
            ui->progressBar->setMaximum(fileSize);
            ui->progressBar->setValue(0);
        }
        else
        {
            //文件信息
            ui->label_message->setText("正在接收文件");
            quint64 len = file.write(buf);
            if(len > 0)
            {
                receiveSize += len;
            }
            //更新进度条
            ui->progressBar->setValue(receiveSize);
            if(receiveSize == fileSize)
            {//传输文件完毕
                file.close();
                ui->label_message->setText("成功接收文件");
                QString filename;
                QWidget *qwidget = new QWidget();
                filename = QFileDialog::getSaveFileName(qwidget,"choose file","./"+fileName,nullptr);
                QDir d;
                d.rename("./"+fileName,filename);
                this->close();
                ui->progressBar->setValue(0);
            }
        }
    });
}

receivefiledialog::~receivefiledialog()
{
    delete ui;
}

void receivefiledialog::closeEvent(QCloseEvent *event)
{
    tcpSocket = new QTcpSocket();
    tcpSocket->abort();
    tcpSocket->connectToHost(ip, port);
    QString message = QString("send_ok");
    tcpSocket->write(message.toUtf8());
    tcpSocket->flush();
    tcpSocket->disconnectFromHost();
    tcpSocket->close();

    tcpSocket = new QTcpSocket();
    tcpSocket->abort();//取消已有链接
    tcpSocket->connectToHost(hostip, hosthost);//链接服务器
    if(!tcpSocket->waitForConnected(30000))
    {
        this->close();
        user.islogin = false;
    }
    else
    {//服务器连接成功
        QString loginmessage = QString("send_file_ok##%1##%2").arg(user.id).arg(otheruser.name);
        qDebug()<<loginmessage;
        tcpSocket->write(loginmessage.toUtf8());
        tcpSocket->flush();
        is_open_chatdialog = false;
    }
    tcpSocket->disconnectFromHost();
    tcpSocket->close();
}
