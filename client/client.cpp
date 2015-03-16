 #include "client.h"
#include "ui_client.h"
#include<QMessageBox>
#include<QString>
#include<QAbstractItemView>

client::client(QWidget *parent):QMainWindow(parent), ui(new Ui::client)
{

    ui->setupUi(this);
    this->setGeometry(600,200,230,470);
    ui->clientList->setGeometry(0,0,230,400);
    ui->clientList->setSelectionMode(QAbstractItemView::SingleSelection);

    welcome();
}

void client::servSendData()
{
    //одна большая функция, которая обрабывает все данные получаемые от сервера
    char what_c[]={0,0,0};
    server_socket->read(what_c,3);
    QString what;
    for(int i=0;i<3;++i)
        what+=what_c[i];

    if(what=="add")
    {
        //к серверу подключился новый пользователь, нужно внести его в локальный список
        QString user="";
        while(server_socket->bytesAvailable())
        {
            char username[]={0};
            server_socket->read(username,1);
            if(username[0]!=';')
                user+=username[0];
            else
            {
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setText(user);
                ui->clientList->insertItem(ui->clientList->count()+1,newItem);
                user="";
            }

        }
        return;
    }
    if(what=="del")
    {
        //от сервера кто-то отсоеденился, нужно екго удалить из локального списка
        char body[20];
        for(int i=0;i<20;++i)
            body[i]=0;
        server_socket->read(body,20);

        for(int i=0;i<ui->clientList->count();++i)
            if(ui->clientList->item(i)->text()==body)
            {
                ui->clientList->takeItem(i);
                return;
            }
        QMessageBox::warning(this,"Warning","Somebody has disconnected from server, but we cant remove him from your user-list","ok");
        return;
    }
    if(what=="adr")
    {
        //ответ на ранее посланный запрос "получить адрес пользователя, которому зотим позвонить"
        char body[20];
        for(int i=0;i<20;++i)
            body[i]=0;
        server_socket->read(body,20);

        QHostAddress ip(body);

        client_socket_mic=new QTcpSocket();
        client_socket_phone=new QTcpSocket();

        tcpServer_mic->close();
        tcpServer_phone->close();

        connect(client_socket_mic,SIGNAL(disconnected()),this,SLOT(dropChannel()));

        /* Звонящий пользователь играет роль "клиента",
           пользователь, который принимает звонок - "сервера" */
        /* Устанавливается прямое соеденение между клиентами,
           к сетевым потокам, затем будут присоеденены потоки микрофона и "наушников" */

        client_socket_mic->connectToHost(ip,27015);
        client_socket_phone->connectToHost(ip,27016);

        if(!client_socket_mic->waitForConnected(2000))
        {
            QMessageBox::warning(this,"Error","User doesnt answer","ok");
            disconnect(client_socket_mic,SIGNAL(disconnected()),this,SLOT(dropCall()));
            client_socket_mic->disconnect();
            client_socket_phone->disconnect();
            delete client_socket_mic;
            delete client_socket_phone;
            tcpServer_mic->listen(QHostAddress::Any,27016);
            tcpServer_phone->listen(QHostAddress::Any,27015);
            return;
        }
        speak();
        return;
    }
    QMessageBox::warning(this,"Warning","The server has sent not understandable data");
    return;
}

void client::dropChannel()
{
    input->stop();
    output->stop();

    delete input;
    delete output;

    QMessageBox::warning(this,"Warning","Disconect from user","ok");

    tcpServer_mic->listen(QHostAddress::Any,27016);
    tcpServer_phone->listen(QHostAddress::Any,27015);

    ui->callButton->setEnabled(true);
    ui->dropButton->setEnabled(false);
}

void client::dropCall()
{
    //"бросить трубку"
    client_socket_phone->close();
    client_socket_mic->close();
    delete client_socket_phone;
    delete client_socket_mic;

}

void client::speak()
{
    //момент разговора
    //связываем аудио потоки с сетевыми
    QAudioFormat format;
    format.setFrequency(8000);
    format.setChannels(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    input=new QAudioInput(format,this);
    output=new QAudioOutput(format,this);

    input->start(client_socket_mic);
    output->start(client_socket_phone);

    ui->dropButton->setEnabled(true);
    ui->callButton->setEnabled(false);
}

void client::wantCall()
{
    //хотим кому-то позвонить, запрашиваем у сервера адреса пользователя
    if(ui->clientList->currentRow()<0)
    {
        QMessageBox::warning(this,"Error","You should select a user","ok");
        return;
    }
    QString name=ui->clientList->currentItem()->text();
    server_socket->write(name.toStdString().c_str());
}

void client::userCall()
{
    //до нас кто-то дозвонился, устанавливаем соеденение
    client_socket_mic=new QTcpSocket();
    client_socket_phone=new QTcpSocket();

    client_socket_mic=tcpServer_mic->nextPendingConnection();
    client_socket_phone=tcpServer_phone->nextPendingConnection();

    connect(client_socket_mic,SIGNAL(disconnected()),this,SLOT(dropChannel()));

    tcpServer_mic->close();
    tcpServer_phone->close();

    speak();
}

void client::send_nick()
{

    //посылаем серверу свой никнейм
    //под.ключаемся к серверу

    //connectFrame->setEnabled(true);
    QMessageBox::about(connectFrame,"Server found!","You have connected");
    connectFrame->close();

    //уничтожаем элементы формы-подключения
    delete connectButton;
    delete cancelButton;
    delete ipAddressLabel;
    delete nickLabel;
    delete inputAddress;
    delete inputNick;

    delete connectFrame;

    //связываем функции с событиями кнопок - "позвонить", "бросить трубку"
    connect(ui->callButton,SIGNAL(clicked()),this,SLOT(wantCall()));
    connect(ui->dropButton,SIGNAL(clicked()),this,SLOT(dropCall()));
    ui->dropButton->setEnabled(false);

    server_socket->write(nick->toStdString().c_str());


    tcpServer_mic = new QTcpServer();
    tcpServer_phone = new QTcpServer();

    connect(tcpServer_mic,SIGNAL(newConnection()),this,SLOT(userCall()));

    tcpServer_mic->listen(QHostAddress::Any,27016);
    tcpServer_phone->listen(QHostAddress::Any,27015);

}

void client::discServ()
{        
    //Внезапное отключение сервера
    QMessageBox::warning(this,"Disconnect","You disconnect form server, sry","ok");
    for(int i=0;i<ui->clientList->count();++i)
        ui->clientList->takeItem(i);
    welcome();

}

bool rightNick(QString name)
{
    //Никнейм - от 1 до 10 симолов, без символа ';'
    if (name.length()>=10 || name.length()==0)
        return false;
    for(int i=0;i<name.length();++i)
        if(name[i]==';')
            return false;
    return true;
}

void client::connectServer()
{
    //Попытка подсоедениться к серверу

    //Корректность никнейма
    if(!rightNick(inputNick->text()))
    {
        QMessageBox::warning(connectFrame,"Error","Your nick must be less 10, doesnt include ';' and not be empty","ok");
        return;
    }

    QString ip_text=inputAddress->text();
    QHostAddress ip;
    if(!ip.setAddress(ip_text))
    {
        //Если сервер по данному адресу не найден
        QMessageBox::warning(connectFrame,"Error","Wrong IP address","ok");
        return;
    }

    this->setWindowTitle(inputNick->text());

    nick=new QString(inputNick->text());

    server_socket=new QTcpSocket();



    //connect(server_socket,SIGNAL(connected()),this,SLOT(send_nick()));

    connect(server_socket,SIGNAL(disconnected()),this,SLOT(discServ()));
    connect(server_socket,SIGNAL(readyRead()),this,SLOT(servSendData()));

    server_socket->connectToHost(ip,27014);
    if(server_socket->waitForConnected(3000))
    {
        send_nick();
        return;
    }
    QMessageBox::warning(connectFrame,"Error","server not found","ok");
    delete server_socket;
    delete nick;
    //connectFrame->setEnabled(false);
}
void client::noHost()
{
    //При подключение хост не найден
    QMessageBox::warning(connectFrame,"Error","Server not found","ok");
    server_socket->disconnect();
}

void client::cancel()
{
    //Кнопка "отмена" на встпутиельной форме
    connectFrame->close();

    delete connectButton;
    delete cancelButton;
    delete ipAddressLabel;
    delete nickLabel;
    delete inputAddress;
    delete inputNick;

    delete connectFrame;

    this->close();
}

void client::welcome()
{
    //Вывод приветствующей формы
    connectFrame=new QMainWindow(ui->centralWidget);
    connectFrame->setWindowModality(Qt::WindowModal);
    connectFrame->setWindowTitle("Please, connect to Server");
    connectFrame->setGeometry(585,350,260,160);

    ipAddressLabel=new QLabel(connectFrame);
    ipAddressLabel->setText("IP's server:");
    ipAddressLabel->setGeometry(10,10,70,23);

    inputAddress=new QLineEdit(connectFrame);
    inputAddress->setGeometry(90,10,100,23);

    nickLabel=new QLabel(connectFrame);
    nickLabel->setText("Your nickname:");
    nickLabel->setGeometry(10,60,70,23);

    inputNick=new QLineEdit(connectFrame);
    inputNick->setGeometry(90,60,100,23);

    connectButton=new QPushButton(connectFrame);
    connectButton->setGeometry(30,110,70,30);
    connectButton->setText("connect");
    connect(connectButton,SIGNAL(clicked()),this,SLOT(connectServer()));

    cancelButton=new QPushButton(connectFrame);
    cancelButton->setGeometry(150,110,70,30);
    cancelButton->setText("cancel");
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(cancel()));


    connectFrame->show();
}

client::~client()
{
    delete ui;
}
