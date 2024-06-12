#include "vjoyclient.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    vjoyclient w;
    w.show();
    return a.exec();
}
