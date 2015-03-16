#ifndef SERVER_H
#define SERVER_H

#include<QMainWindow>
#include<QTcpServer>
#include<QTcpSocket>
#include<QHostAddress>
#include<QString>
#include<QLinkedList>
#include<QNetworkInterface>
#include<QPushButton>
#include<QLabel>
#include<QTextEdit>
#include<QMutex>
#include<iterator>
#include"clientnode.h"
using namespace std;

namespace Ui {
    class server;
}

class server : public QMainWindow
{
    Q_OBJECT

public:
    explicit server(QWidget *parent = 0);
    ~server();

private:
    Ui::server *ui;
    //все клиенты хранятся в листе на сервере, тип - ClientNode
    QLinkedList<ClientNode>* clients_paper;

    QTcpServer* tcpServer;
    QPushButton* onButton;
    QPushButton* offButton;
    QTextEdit* console;
    bool isRun;


private slots:
    void turnOn();
    void turnOff();
    void addClient();
    void phone_channel(QString);
    void removeClient(QString);
    void quit();
};

#endif // SERVER_H
