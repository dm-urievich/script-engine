#include "eventmachinethread.h"

#include <QDomElement>
#include <QDomDocument>
#include <QVector>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

const QString CONNECTIONS_CONF_FILE = "../connections-conf.xml";
const QString MODULE_EVENTS_FILE = "../../moduleEvents.xml";
const QString MODULE_SOCKETS_FILE = "../../moduleSockets.xml";

EventMachineThread::EventMachineThread(QObject *parent) :
    QThread(parent)
{
    m_timeToReadData = new QTimer(this);
    m_timeToReadData->setInterval(1000);

    connect(m_timeToReadData, SIGNAL(timeout()), this, SLOT(readEvents()));
}

void EventMachineThread::run()
{
    // open file with connections
    // start timer for read file and make actions with events
    createConections(CONNECTIONS_CONF_FILE);


//    m_timeToReadData->start();

    for(;;) {
        readEvents();       // it's not right
        QThread::msleep(1000);
    }
    exec();
}

void EventMachineThread::readEvents()
{
    QDomDocument doc("module");
    QFile inFile(MODULE_EVENTS_FILE);
    QString errorParse;
    QVector<Connector*>::iterator connect = m_connections.begin();
    int errorLine;
    int idModule = 0;

    static QDateTime lastModif;
    QDateTime curModif;
    QFileInfo fileDate(MODULE_EVENTS_FILE);
    curModif = fileDate.lastModified();
    if (curModif <= lastModif) {
        return;
    }
    lastModif = curModif;

    if (!inFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: read file";
        return;
    }
    if (!doc.setContent(&inFile, &errorParse, &errorLine)) {
        qDebug() << "Error: " << errorParse;
        qDebug() << "in line: " << errorLine << endl;
        inFile.close();
        return;
    }
    inFile.close();

//    qDebug() << "Read file " << MODULE_EVENTS_FILE;

    // печатает имена всех непосредственных потомков
    // внешнего элемента.
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // пробуем преобразовать узел в элемент.
        if(!e.isNull()) {
            //qDebug() << e.tagName() << '\n'; // узел действительно является элементом.
            //qDebug() << "attr: " << e.attribute("id") << '\n';
            idModule = e.attribute("id").toInt();
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

    QFile outFile(MODULE_SOCKETS_FILE);
    if (outFile.open(QIODevice::WriteOnly)) {
        QTextStream out(&outFile);
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
        outFile.close();
    }
}
