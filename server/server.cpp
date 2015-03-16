#include "server.h"
#include "ui_server.h"
#include<QMessageBox>
#include<string>

server::server(QWidget *parent):QMainWindow(parent), ui(new Ui::server)
{
    ui->setupUi(this);
    this->setWindowTitle("Server");

    onButton=new QPushButton(this);
    onButton->setGeometry(75,410,70,30);
    onButton->setText("On");

    offButton=new QPushButton(this);
    offButton->setGeometry(205,410,70,30);
    offButton->setText("Off");

    console=new QTextEdit(this);
    console->setGeometry(0,0,350,380);
    console->setReadOnly(true);


    isRun=false;

    connect(onButton,SIGNAL(clicked()),this,SLOT(turnOn()));
    connect(offButton,SIGNAL(clicked()),this,SLOT(turnOff()));
    connect(this,SIGNAL(destroyed()),this,SLOT(quit()));
}

void server::removeClient(QString source)
{
    //удаление клиента (отключанение)
    if(isRun)
    {
        QLinkedList<ClientNode>::iterator it;
        bool fl=false;

        for(it=clients_paper->begin();it!=clients_paper->end() && !fl;++it)
            if((*it).name==source)
                fl=true;
        if(!fl)
        {
            console->append("Error while deleting user");
            return;
        }
        --it;

        QString info=(*it).name+" disconnected";
        console->append(info);

        QString nick="del"+(*it).name;
        clients_paper->erase(it);


        for(it=clients_paper->begin();it!=clients_paper->end();++it)
            if((*it).client_socket->isWritable())
                (*it).client_socket->write(nick.toStdString().c_str());
    }

}

void server::phone_channel(QString source)
{
    //отправка адреса клиенту, который хочет позвонить

    console->append("try to create chanel");
    QLinkedList<ClientNode>::iterator it;
    bool fl=false;

    for(it=clients_paper->begin();it!=clients_paper->end() && !fl;++it)
        if((*it).name==source)
            fl=true;
    if(!fl)
    {
        console->append("Cant detect caller-user");
        return;
    }
    --it;
    char nick_c[10];
    for(int i=0;i<10;++i)
        nick_c[i]=0;
    (*it).client_socket->read(nick_c,10);
    QString nick=nick_c;

    QString info=(*it).name+" wants call to "+nick;
    console->append(info);

    QLinkedList<ClientNode>::iterator iter;
    fl=false;
    for(iter=clients_paper->begin();iter!=clients_paper->end() && !fl;++iter)
        if((*iter).name==nick)
            fl=true;
    if(!fl)
    {
        console->append("Cant detect users goal");
        return;
    }
    --iter;
    QString data="adr"+(*iter).client_socket->peerAddress().toString();
    (*it).client_socket->write(data.toStdString().c_str());

    info="data for "+(*it).name+" has sent";
    console->append(info);
}
void server::addClient()
{
    //подключаен нового клиента

    ClientNode* temp=new ClientNode();
    //temp->client_socket=tcpServer->nextPendingConnection();
    clients_paper->push_back(*temp);

    clients_paper->back().client_socket=tcpServer->nextPendingConnection();

    char recv_name[10];
    for(int i=0;i<10;++i)
        recv_name[i]=0;
    console->append("Somebody tries to connect, wait for his nickname...");
    clients_paper->back().client_socket->waitForReadyRead(30000);
    if(!clients_paper->back().client_socket->bytesAvailable())
    {
        console->append("Cant reiceve user's nickname. User will be disconnect");
        clients_paper->back().client_socket->close();
        return;
    }
    clients_paper->back().client_socket->read(recv_name,10);
    clients_paper->back().name=recv_name;   

    for(QLinkedList<ClientNode>::iterator it=clients_paper->begin();it!=(clients_paper->end()-1);++it)
        if((*it).name==clients_paper->back().name)
        {
            console->append("This nick has already used, user will be disconnect");
            clients_paper->back().client_socket->disconnect();
            clients_paper->pop_back();
            return;
        }


    QString nick="add"+clients_paper->back().name+";";

    QString adduser="add";

    for(QLinkedList<ClientNode>::iterator it=clients_paper->begin();it!=(clients_paper->end()-1);++it)
    {
        (*it).client_socket->write(nick.toStdString().c_str());
        adduser=adduser+(*it).name+";";
    }
    clients_paper->back().client_socket->write(adduser.toStdString().c_str());

    connect((&clients_paper->back())->client_socket,SIGNAL(disconnected()),&clients_paper->back(),SLOT(remove()));
    connect((&clients_paper->back())->client_socket,SIGNAL(readyRead()),&clients_paper->back(),SLOT(create_channel()));

    connect(&clients_paper->back(),SIGNAL(delete_user(QString)),this,SLOT(removeClient(QString)));
    connect(&clients_paper->back(),SIGNAL(need_channel(QString)),this,SLOT(phone_channel(QString)));

    QString join=clients_paper->back().name+" joins! His (her) addres is "+
                                                            clients_paper->back().client_socket->peerAddress().toString();
    console->append(join);
}

void server::quit()
{
//полное удаление (отсоеденение) пользователей
   QLinkedList<ClientNode>::iterator it=clients_paper->begin();
   while(it!=clients_paper->end())
   {
       QLinkedList<ClientNode>::iterator temp=it;
       it++;
       QString my="close the "+(*temp).name;
       console->append(my);

       (*temp).client_socket->close();
   }

   delete tcpServer;
   delete clients_paper;
   console->append("Server is off now");



}

void server::turnOff()
{
    //выключение сервера
    if(!isRun)
    {
        console->append("Server has already off");
        return;
    }
    console->append("Shut down the server, all users will be disconnect...");
    isRun=false;

    quit();
}

void server::turnOn()
{
    //включаем сервер, если он еще не включен
    if(isRun)
    {
        console->append("Server has already run");
        return;
    }
    tcpServer=new QTcpServer(this);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(addClient()));

    if(!tcpServer->listen(QHostAddress::Any,27014))
    {
        console->append("Can't run the server");
        delete tcpServer;
        return;
    }
    isRun=true;
    //выводим на экран адрес сервера
    QList<QHostAddress> addr = QNetworkInterface::allAddresses();

    QString ipAddress = (*(addr.begin()+1)).toString();

    clients_paper=new QLinkedList<ClientNode>;
    QString run="Server has run\nIP Adress is " + ipAddress + " Port is 27014\nWaiting for people...";

    console->append(run);
}

server::~server()
{
    delete ui;
}
