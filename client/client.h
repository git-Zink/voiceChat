#ifndef CLIENT_H
#define CLIENT_H

#include<QMainWindow>
#include<QPushButton>
#include<QLabel>
#include<QLineEdit>
#include<QTcpServer>
#include<QTcpSocket>
#include<QHostAddress>
#include<QString>
#include<QAudioInput>
#include<QAudioOutput>

namespace Ui {
    class client;
}

class client : public QMainWindow
{
    Q_OBJECT

public:
    explicit client(QWidget *parent = 0);
    ~client();

private:
    Ui::client *ui;
    void welcome();
    QMainWindow* connectFrame;
    QPushButton* connectButton;
    QPushButton* cancelButton;
    QLabel* ipAddressLabel;
    QLabel* nickLabel;
    QLineEdit* inputAddress;
    QLineEdit* inputNick;
    QTcpSocket* server_socket;

    QTcpServer* tcpServer_mic;
    QTcpServer* tcpServer_phone;

    QTcpSocket* client_socket_mic;
    QTcpSocket* client_socket_phone;

    QString* nick;
    QAudioInput* input;
    QAudioOutput* output;

private slots:
    void connectServer();
    void cancel();
    void send_nick();
    void discServ();
    void servSendData();
    void userCall();
    void wantCall();
    void dropCall();
    void speak();
    void dropChannel();
    void noHost();

};

#endif // CLIENT_H
