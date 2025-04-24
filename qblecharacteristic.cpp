#include "qblecharacteristic.h"
#include <QtXml/QtXml>


QBLECharacteristic::QBLECharacteristic(const QString &path, QObject *parent) : QObject(parent)
{
    m_characteristicInterface = new QDBusInterface("org.bluez", path, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus());

    m_path = path;
    m_uuid = m_characteristicInterface->property("UUID").toString();

    QStringList argumentMatch;
    argumentMatch << "org.bluez.GattCharacteristic1";
    QDBusConnection::systemBus().connect("org.bluez", path, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
                            this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
    introspect();
}

void QBLECharacteristic::onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    Q_UNUSED(interface);
    Q_UNUSED(list);
    //qDebug() << "Characteristic property changed" << interface << map;

    if (map.contains("Value")) {
        emit characteristicChanged(m_uuid, map["Value"].toByteArray());
    }
}

void QBLECharacteristic::writeValue(const QByteArray &val) const
{
    qDebug() << Q_FUNC_INFO << val.toHex();
    m_characteristicInterface->call("WriteValue", val, QVariantMap());
}

void QBLECharacteristic::writeAsync(const QByteArray &val) const
{
    m_characteristicInterface->asyncCall("WriteValue", val, QVariantMap());
}

QByteArray QBLECharacteristic::readValue() const
{
    QDBusReply<QByteArray> reply = m_characteristicInterface->call("ReadValue", QVariantMap());
    //qDebug() << reply.error().message();
    return reply.value();
}

void QBLECharacteristic::readAsync()
{
    QDBusPendingCall async = m_characteristicInterface->asyncCall("ReadValue", QVariantMap());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(readFinished(QDBusPendingCallWatcher*)));
}

void QBLECharacteristic::readFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QByteArray> reply = *call;
    if (reply.isError()) {
        qDebug() << "QBLECharacteristic::readFinished:" << reply.error().message();
    } else {
        m_value = reply.value();
        emit characteristicRead(m_uuid, m_value);
    }
    call->deleteLater();
}

void QBLECharacteristic::startNotify() const
{
    m_characteristicInterface->call("StartNotify");
}

void QBLECharacteristic::stopNotify() const
{
    m_characteristicInterface->call("StopNotify");
}

QByteArray QBLECharacteristic::value() const
{
    return m_value;
}


void QBLECharacteristic::introspect()
{
    QDBusInterface miIntro("org.bluez", m_path, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = miIntro.call("Introspect");
    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");


    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("desc")) {
            QString path = m_path + "/" + nodeName;
            QDBusInterface descInterface("org.bluez", path, "org.bluez.GattDescriptor1", QDBusConnection::systemBus(), 0);
                m_descriptorMap[descInterface.property("UUID").toString()] = new QBLEDescriptor(path, this);
        }
    }

    qDebug() << Q_FUNC_INFO << "descriptors" << m_descriptorMap.keys();

}

QBLEDescriptor *QBLECharacteristic::descriptor(const QString &c) const
{
    return m_descriptorMap.value(c, nullptr);
}
