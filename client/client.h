#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <registerdialog.h>
#include <home.h>

QT_BEGIN_NAMESPACE
namespace Ui { class client; }
QT_END_NAMESPACE

class client : public QMainWindow
{
    Q_OBJECT

public:
    client(QWidget *parent = nullptr);
    ~client();

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_register_clicked();

    void on_lineEdit_username_editingFinished();

    void on_lineEdit_username_textEdited(const QString &arg1);

    void on_lineEdit_pwd_editingFinished();

    void on_lineEdit_pwd_textEdited(const QString &arg1);

private:
    Ui::client *ui;
    QTcpSocket *tcpSocket;
};
#endif // CLIENT_H
