#ifndef CLIENTNODE_H
#define CLIENTNODE_H
#include<QTcpSocket>
#include<QString>
#include<QObject>
class ClientNode:public QObject
{
    Q_OBJECT
private:
    //Q_DISABLE_COPY(ClientNode);
public:
    QString name;
    QTcpSocket* client_socket;
    bool state;

    ClientNode();
    ClientNode(const ClientNode &);
    //~ClientNode();
public slots:
    void remove();
    void create_channel();
signals:
    void delete_user(QString);
    void need_channel(QString);
};
#endif // CLIENTNODE_H
