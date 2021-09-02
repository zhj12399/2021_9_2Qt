#ifndef SENDFILEDIALOG_H
#define SENDFILEDIALOG_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QTimer>
#include <QCloseEvent>

namespace Ui {
class sendfiledialog;
}

class sendfiledialog : public QWidget
{
    Q_OBJECT

public:
    explicit sendfiledialog(QWidget *parent = nullptr);
    ~sendfiledialog();

    //发送数据
    void sendData();

private slots:
    void on_pushButton_selectfile_clicked();
    void on_pushButton_sendfile_clicked();
protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::sendfiledialog *ui;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    QFile file;
    QTimer timer;
    QString fileName;
    quint64 fileSize;
    quint64 sendSize;
};

#endif // SENDFILEDIALOG_H
