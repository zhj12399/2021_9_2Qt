#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
namespace Ui {
class chatdialog;
}

class chatdialog : public QWidget
{
    Q_OBJECT

public:
    explicit chatdialog(QWidget *parent = nullptr);
    ~chatdialog();
    QTimer *timer;
private slots:
    void on_pushButton_clicked();
    void getchathistory();
    void on_pushButton_send_clicked();

protected:
    void closeEvent(QCloseEvent *event);
private:
    Ui::chatdialog *ui;
    QTcpSocket *tcpSocket;
};

#endif // CHATDIALOG_H
