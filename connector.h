#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QDomElement>
#include <QTextStream>
#include <QString>

/*
 * Специальный класс для соединения
 * событий и сокетов модулей
 * все генериться через XML
 */

class Connector
{
public:
    Connector(QString nameSender = "sender", QString nameReceiver = "receiver");

    bool isEvent(void);
    void addConnection(int idSender, QString event, int idReceiver, QString socket);
    void parseXml(QDomElement& domElement);
    void generateXml(QTextStream& out);

    int idModule;

    QString nameSender;
    QString nameReceiver;

private:
    int m_idReceiver;
    QString m_name;
    QString m_event;
    QString m_socket;
    QString m_dataEvent;
    bool m_eventModule;
};

#endif // CONNECTOR_H
