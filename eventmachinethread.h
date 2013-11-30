#ifndef EVENTMACHINETHREAD_H
#define EVENTMACHINETHREAD_H

#include <QThread>
#include <QTimer>
#include <QVector>

#include "connector.h"

class EventMachineThread : public QThread
{
    Q_OBJECT
public:
    explicit EventMachineThread(QObject *parent = 0);
    void createConections(QString fileName);

protected:
    void run();

private:
    void generateSockets(void);

    QTimer* m_timeToReadData;
    QVector<Connector*> m_connections;

signals:

public slots:
    void readEvents();
};

#endif // EVENTMACHINETHREAD_H
