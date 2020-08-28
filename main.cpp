#include "NodeListTable.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NodeListTable w;
    w.show();

    return a.exec();
}
