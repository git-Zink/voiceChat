#include "clientnode.h"

ClientNode::ClientNode()
{
    name="";
    client_socket=new QTcpSocket();
    state=true;

   /* connect(client_socket,SIGNAL(disconnected()),this,SLOT(remove()));
    connect(client_socket,SIGNAL(readyRead()),this,SLOT(create_channel()));*/
}
ClientNode::ClientNode(const ClientNode & source)
{
    name=source.name;
    client_socket=new QTcpSocket();

   /* connect(client_socket,SIGNAL(disconnected()),this,SLOT(remove()));
    connect(client_socket,SIGNAL(readyRead()),this,SLOT(create_channel()));*/

    client_socket=source.client_socket;
    state=source.state;
}

/*ClientNode::~ClientNode()
{
    //delete client_socket;
}*/
void ClientNode::create_channel()
{
    emit need_channel(name);
}
void ClientNode::remove()
{
    emit delete_user(name);
}
