#ifndef CLIENTSERVE_H
#define CLIENTSERVE_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class clientserve;
}

class clientserve : public QWidget
{
    Q_OBJECT

public:
    explicit clientserve(QWidget *parent = nullptr);
    ~clientserve();

private:
    Ui::clientserve *ui;
    QTcpServer * tcpServer; //监听
    QTcpSocket * tcpSocket; //通信
};

#endif // CLIENTSERVE_H
