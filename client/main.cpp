#include <QtGui/QApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    client w;
    w.show();

    return a.exec();
}
