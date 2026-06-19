#include "qblelocalcharacteristic.h"
#include "qblelocalservice.h"

#include <QDebug>

QBLELocalCharacteristic::QBLELocalCharacteristic(QDBusConnection bus, unsigned int index, const QString &uuid, const QStringList &flags, QBLELocalService *service, QObject *parent)
    : QObject(parent), m_uuid(uuid), m_flags(flags), m_service(service)
{
    m_path = service->path().path() + QStringLiteral("/char") + QString::number(index);
    bus.registerObject(m_path, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties);
}

QDBusObjectPath QBLELocalCharacteristic::path() const
{
    return QDBusObjectPath(m_path);
}

QDBusObjectPath QBLELocalCharacteristic::servicePath() const
{
    return m_service->path();
}

QString QBLELocalCharacteristic::uuid() const
{
    return m_uuid;
}

QStringList QBLELocalCharacteristic::flags() const
{
    return m_flags;
}

QList<QDBusObjectPath> QBLELocalCharacteristic::descriptorPaths() const
{
    return {};
}

// Default implementations — subclasses override what they need

QByteArray QBLELocalCharacteristic::ReadValue(const QVariantMap &)
{
    qWarning() << Q_FUNC_INFO << "Default ReadValue called — override in subclass";
    return QByteArray();
}

void QBLELocalCharacteristic::WriteValue(const QByteArray &value, const QVariantMap &)
{
    qDebug() << Q_FUNC_INFO << "Default WriteValue — emitting valueWritten";
    emit valueWritten(value);
}

void QBLELocalCharacteristic::StartNotify()
{
    qDebug() << Q_FUNC_INFO << "StartNotify called";
}

void QBLELocalCharacteristic::StopNotify()
{
    qDebug() << Q_FUNC_INFO << "StopNotify called";
}
