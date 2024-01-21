#include "qbledescriptor.h"

QBLEDescriptor::QBLEDescriptor(const QString &path, QObject *parent) : QObject(parent)
{
    m_descriptorInterface = new QDBusInterface("org.bluez", path, "org.bluez.GattDescriptor1", QDBusConnection::systemBus());
    m_uuid = m_descriptorInterface->property("UUID").toString();

    QStringList argumentMatch;
    argumentMatch << "org.bluez.GattDescriptor1";
    QDBusConnection::systemBus().connect("org.bluez", path, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
                            this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
}

void QBLEDescriptor::onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    Q_UNUSED(interface);
    Q_UNUSED(list);
    //qDebug() << "descriptor property changed" << interface << map;

    if (map.contains("Value")) {
        emit descriptorChanged(m_uuid, map["Value"].toByteArray());
    }
}

void QBLEDescriptor::writeValue(const QByteArray &val) const
{
    m_descriptorInterface->call("WriteValue", val, QVariantMap());
}

void QBLEDescriptor::writeAsync(const QByteArray &val) const
{
    m_descriptorInterface->asyncCall("WriteValue", val, QVariantMap());
}

void QBLEDescriptor::readAsync()
{
    QDBusPendingCall async = m_descriptorInterface->asyncCall("ReadValue", QVariantMap());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(readFinished(QDBusPendingCallWatcher*)));
}

void QBLEDescriptor::readFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QByteArray> reply = *call;
    if (reply.isError()) {
        qDebug() << "QBLEdescriptor::readFinished:" << reply.error().message();
    } else {
        m_value = reply.value();
        emit descriptorRead(m_uuid, m_value);
    }
    call->deleteLater();
}

void QBLEDescriptor::startNotify() const
{
    m_descriptorInterface->call("StartNotify");
}

void QBLEDescriptor::stopNotify() const
{
    m_descriptorInterface->call("StopNotify");
}

QByteArray QBLEDescriptor::value() const
{
    return m_value;
}

QByteArray QBLEDescriptor::readValue() const
{
    QDBusReply<QByteArray> reply = m_descriptorInterface->call("ReadValue", QVariantMap());
    //qDebug() << reply.error().message();
    return reply.value();
}
