#include "connector.h"

Connector::Connector(QString nameSender, QString nameReceiver) :
    nameSender(nameSender),
    nameReceiver(nameReceiver)
{
}

bool Connector::isEvent()
{
    return m_eventModule;
}

void Connector::addConnection(int idSender, QString event, int idReceiver, QString socket)
{
    idModule = idSender;
    m_idReceiver = idReceiver;
    m_event = event;
    m_socket = socket;
}

void Connector::generateXml(QTextStream &out)
{
    out << "<connector id=\""   << m_idReceiver    <<"\">\n";
    out << "<idModule>"         << idModule       << "</idModule>\n";
    out << "<" << m_socket << ">" << m_dataEvent    << "</" << m_socket << ">\n";
    out << "</connector>\n";

    m_eventModule = false;
}

void Connector::parseXml(QDomElement &domElement)
{
    QString tagName;

    QDomNode n = domElement.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // пробуем преобразовать узел в элемент.
        if(!e.isNull()) {
            tagName = e.tagName(); // узел действительно является элементом.
            if (tagName == m_event) {
                m_dataEvent = e.text();
                if (m_dataEvent.toInt() != 0) {
                    m_eventModule = true;
                }
            }
        }
        n = n.nextSibling();
    }
}
