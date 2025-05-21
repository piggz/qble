#include "qbledevice.h"

QBLEDevice::QBLEDevice(QObject *parent) : QObject(parent)
{
}

void QBLEDevice::setDevicePath(const QString &path)
{

    m_devicePath = path;
    m_deviceInterface = new QDBusInterface("org.bluez", m_devicePath, "org.bluez.Device1", QDBusConnection::systemBus());

    qDebug() << Q_FUNC_INFO << path <<  m_deviceInterface->isValid();
    QStringList argumentMatch;
    argumentMatch << "org.bluez.Device1";

    QDBusConnection::systemBus().connect("org.bluez", m_devicePath, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
                                         this, SLOT(onPropertiesChangedInt(QString, QVariantMap, QStringList)));
}

void QBLEDevice::pair()
{
    qDebug() << "QBLEDevice::pair";

    QDBusPendingCall pcall = m_deviceInterface->asyncCall("Pair");

    auto watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [&](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<void> reply(*w);
        if (reply.error().type() != QDBusError::NoError) {
            qDebug() << reply.error().message();
            Q_EMIT error(reply.error().message());
        }
        w->deleteLater();
    });
}

void QBLEDevice::connectToDevice()
{
    qDebug() << "QBLEDevice::connectToDevice";

    QDBusPendingCall pcall = m_deviceInterface->asyncCall("Connect");

    auto watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [&](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<void> reply(*w);
        if (reply.error().type() != QDBusError::NoError) {
            qDebug() << reply.error().message();
            Q_EMIT error(reply.error().message());
        }
        w->deleteLater();
    });
}

void QBLEDevice::disconnectFromDevice()
{
    qDebug() << "QBLEDevice::disconnectFromDevice";

    QDBusPendingCall pcall = m_deviceInterface->asyncCall("Disconnect");

    auto watcher = new QDBusPendingCallWatcher(pcall, this);
        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                         [&](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<void> reply(*w);
            if (reply.error().type() != QDBusError::NoError) {
                qDebug() << reply.error().message();
                Q_EMIT error(reply.error().message());
            }
            w->deleteLater();
        });
}

QBLEService* QBLEDevice::service(const QString &uuid) const
{
    return m_serviceMap.value(uuid, nullptr);
}

QString QBLEDevice::devicePath() const
{
    return m_devicePath;
}

void QBLEDevice::addService(const QString &uuid, QBLEService *service)
{
    m_serviceMap[uuid] = service;
}

void QBLEDevice::onPropertiesChangedInt(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    emit propertiesChanged(interface, map, list);
}

QVariant QBLEDevice::deviceProperty(const char *name) const
{
    if (m_deviceInterface) {
        return m_deviceInterface->property(name);
    }
    return QVariant();
}

QMap<QString, QBLEService *> QBLEDevice::services()
{
    return m_serviceMap;
}
