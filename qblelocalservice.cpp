#include "qblelocalservice.h"
#include "qblelocalcharacteristic.h"

#include <QDebug>

QBLELocalService::QBLELocalService(QDBusConnection bus, unsigned int index, const QString &uuid, const QString &appPath, QObject *parent)
    : QObject(parent), m_uuid(uuid)
{
    m_path = appPath + QStringLiteral("/service") + QString::number(index);
    bus.registerObject(m_path, this, QDBusConnection::ExportAdaptors | QDBusConnection::ExportAllProperties);
}

QDBusObjectPath QBLELocalService::path() const
{
    return QDBusObjectPath(m_path);
}

void QBLELocalService::addCharacteristic(QBLELocalCharacteristic *ch)
{
    m_characteristics.append(ch);
}

QList<QDBusObjectPath> QBLELocalService::characteristicPaths() const
{
    QList<QDBusObjectPath> result;
    for (QBLELocalCharacteristic *ch : m_characteristics) {
        result.append(ch->path());
    }
    return result;
}

QList<QBLELocalCharacteristic *> QBLELocalService::characteristics() const
{
    return m_characteristics;
}

QString QBLELocalService::uuid() const
{
    return m_uuid;
}

bool QBLELocalService::primary() const
{
    return true;
}
