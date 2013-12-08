#include "eventmachinethread.h"

#include <QDomElement>
#include <QDomDocument>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QDateTime>

const int PORT = 2525;

const QString CONNECTIONS_CONF_FILE = "../connections-conf.xml";

EventMachineThread::EventMachineThread(QObject *parent) :
    QThread(parent)
{
    m_timeToReadData = new QTimer;
    m_server = new QTcpServer(this);

    if (!m_server->listen(QHostAddress::Any, PORT)) {
        qDebug() << "Fail open port " << PORT;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(clientConnect()));
}

void EventMachineThread::run()
{
    // open file with connections
    // start timer for read file and make actions with events
    createConections(CONNECTIONS_CONF_FILE);

    exec();
}

void EventMachineThread::clientConnect()
{
    qDebug() << "New connection!";

    QTcpSocket* client = m_server->nextPendingConnection();

    connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnect()));
    connect(client, SIGNAL(readyRead()), this, SLOT(readEvents()));

    m_sockets.push_back(client);
}

void EventMachineThread::clientDisconnect()
{
    qDebug() << "client disconnect";

    QTcpSocket* client = (QTcpSocket*) sender();

    for (int i = 0; i < m_sockets.size(); i++) {
        if (client == m_sockets[i]) {
            m_sockets.remove(i);
        }
    }

    client->deleteLater();
}

void EventMachineThread::readEvents()
{
    QTcpSocket* client = (QTcpSocket*) sender();

    qDebug() << "input xml file";

    QDomDocument doc("module");

    QTextStream inSocket(client);
    QString xmlData;

    xmlData = inSocket.readAll();

    int errorLine;
    QString errorParse;
    if (!doc.setContent(xmlData, &errorParse, &errorLine)) {
        qDebug() << "Error: " << errorParse;
        qDebug() << "in line: " << errorLine << endl;
        return;
    }

    QDomElement docElem = doc.documentElement();

    QVector<Connector*>::iterator connect = m_connections.begin();
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // пробуем преобразовать узел в элемент.
        if(!e.isNull()) {
            int idModule = e.attribute("id").toInt();
            if (idModule) {
                 // удобнее это вынести в функцию
                for (connect = m_connections.begin(); connect != m_connections.end(); ++connect) {
                    if ((*connect)->idModule == idModule) {
                        (*connect)->parseXml(e);
                    }
                }
            }
        }
        n = n.nextSibling();
    }

    // now it there, maybe this mast be in other place
    generateSockets();
}

void EventMachineThread::createConections(QString fileName)
{
    QDomDocument doc("connections");
    QFile inFile(fileName);
    QString errorParse;
    QString tagName;
    int errorLine;

    QString event;
    QString socket;
    QString nameSender;
    QString nameReceiver;
    int receiver;
    int sender;

    if (!inFile.open(QIODevice::ReadOnly)) {
        qDebug() << "couldn't open file: " << fileName;
        return;
    }
    if (!doc.setContent(&inFile, &errorParse, &errorLine)) {
        qDebug() << "Error: " << errorParse;
        qDebug() << "in line: " << errorLine << endl;
        inFile.close();
        return;
    }
    inFile.close();

    // печатает имена всех непосредственных потомков
    // внешнего элемента.
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    QDomNode childNod;
    QDomElement childElem;
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // пробуем преобразовать узел в элемент.
        if(!e.isNull()) {
            //qDebug() << e.tagName() << '\n'; // узел действительно является элементом.
            //qDebug() << "attr: " << e.attribute("id") << '\n';
            if (e.tagName() == "connect") {
                childNod = e.firstChild();
                while(!childNod.isNull()) {
                    childElem = childNod.toElement();
                    if(!childElem.isNull()) {
                        tagName = childElem.tagName(); // узел действительно является элементом.
                        //qDebug() << tagName;
                        if (tagName == "nameSender") {
                            nameSender = childElem.text();
                        }
                        if (tagName == "idSender") {
                            sender = childElem.text().toInt();
                        }
                        if (tagName == "event") {
                            event = childElem.text();
                        }
                        if (tagName == "nameReceiver") {
                            nameReceiver = childElem.text();
                        }
                        if (tagName == "idReceiver") {
                            receiver = childElem.text().toInt();
                        }
                        if (tagName == "socket") {
                            socket = childElem.text();
                        }
                    }
                    childNod = childNod.nextSibling();
                }

                // непосредственное создание connection
//                eswitchDev = new eSwitch(0, name, addr);
//                eswitchDev->idModule = id;
//                createNewHardwareModule(eswitchDev);
                Connector* connect = new Connector(nameSender, nameReceiver);
                connect->addConnection(sender, event, receiver, socket);
                m_connections.push_back(connect);

                sender = 0;
                event.clear();
                receiver = 0;
                socket.clear();
            }

        }
        n = n.nextSibling();
    }
}

/*
void EventMachineThread::eventGuiMachine(int idModule, QString eventName, QString eventData)
{
    QVector<ModuleGui*>::Iterator module;


    for (module = moduleVector.begin(); module != moduleVector.end(); ++module) {
        if ((*module)->idModule == idModule) {
            (*module)->socket(eventName, eventData);
        }
    }

    generateXmlModuleGui();
}
*/

void EventMachineThread::generateSockets()
{
    QVector<Connector*>::Iterator connect;

    QString xmlData;

    QTextStream out(&xmlData);

    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<module>\n";

    for (connect = m_connections.begin(); connect != m_connections.end(); ++connect) {
        if ((*connect)->isEvent()) {
            (*connect)->generateXml(out);
            qDebug() << "Event in " << (*connect)->nameSender << " to " << (*connect)->nameReceiver;
        }
    }

    out << "</module>\n";

    // send to all clients
    for (int i = 0; i < m_sockets.size(); i++) {
        m_sockets[i]->write(xmlData.toUtf8());
    }
}
