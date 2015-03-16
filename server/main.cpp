#include <QtGui/QApplication>
#include "server.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    server w;
    w.show();
    return a.exec();
}
