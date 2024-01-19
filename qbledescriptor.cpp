#include "qbledescriptor.h"

QBLEDescriptor::QBLEDescriptor(const QString &path, QObject *parent) : QObject(parent)
{
    m_descriptorInterface = new QDBusInterface("org.bluez", path, "org.bluez.GattDescriptor1", QDBusConnection::systemBus());
    m_uuid = m_descriptorInterface->property("UUID").toString();

//    QStringList argumentMatch;
//    argumentMatch << "org.bluez.GattDescriptor1";
//    QDBusConnection::systemBus().connect("org.bluez", path, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
//                            this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
}

void QBLEDescriptor::writeValue(const QByteArray &val) const
{
    m_descriptorInterface->call("WriteValue", val, QVariantMap());
}

void QBLEDescriptor::writeAsync(const QByteArray &val) const
{
    m_descriptorInterface->asyncCall("WriteValue", val, QVariantMap());
}

