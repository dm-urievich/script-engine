#include <QCoreApplication>
#include "eventmachinethread.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    EventMachineThread *core = new EventMachineThread(&a);

    // This will cause the application to exit when
    // the task signals finished.
    QObject::connect(core, SIGNAL(finish()), &a, SLOT(quit()));

    // This will run the task from the application event loop.
    core->start();

    return a.exec();
}
