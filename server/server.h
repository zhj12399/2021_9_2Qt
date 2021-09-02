#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QDateTime>
#include <QSqlQuery>

const int M = 20;//设置20个通信口
QT_BEGIN_NAMESPACE
namespace Ui { class server; }
QT_END_NAMESPACE

class server : public QMainWindow
{
    Q_OBJECT

public:
    server(QWidget *parent = nullptr);
    ~server();
private:
    Ui::server *ui;
    QTcpServer* tcpServer;
    QTcpSocket* tcpSocket[M];
    QSqlDatabase db;
    QSqlQuery sqlquery;
};
#endif // SERVER_H
