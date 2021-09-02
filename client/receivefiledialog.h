#ifndef RECEIVEFILEDIALOG_H
#define RECEIVEFILEDIALOG_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>
#include <QCloseEvent>

namespace Ui {
class receivefiledialog;
}

class receivefiledialog : public QWidget
{
    Q_OBJECT

public:
    explicit receivefiledialog(QWidget *parent = nullptr);
    ~receivefiledialog();

private slots:

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::receivefiledialog *ui;
    QTcpSocket *tcpSocket;
    QFile file;
    QString fileName;
    quint64 fileSize;
    quint64 receiveSize;
    bool isStart;

    QString ip;
    qint16 port;
};

#endif // RECEIVEFILEDIALOG_H
