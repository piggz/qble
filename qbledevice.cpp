#include "qbledevice.h"

#include <QDebug>
#include <QDBusPendingReply>

QBLEDevice::QBLEDevice(QObject *parent) : QObject(parent)
{
}

QBLEDevice::~QBLEDevice()
{
    if (m_deviceInterface) {
        delete m_deviceInterface;
    }
}

void QBLEDevice::setDevicePath(const QString& path)
{
    qDebug() << Q_FUNC_INFO << path;

    if (path.isEmpty()) {
        return;
    }

    QStringList argumentMatch;
    argumentMatch << "org.bluez.Device1";

    if (path != m_devicePath) {
        if (m_deviceInterface) {
            QDBusConnection::systemBus().disconnect("org.bluez", m_devicePath, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(), this, SLOT(onPropertiesChangedInt(QString, QVariantMap, QStringList)));
            delete m_deviceInterface;
        }

        m_devicePath = path;
        m_deviceInterface = new QDBusInterface("org.bluez", m_devicePath, "org.bluez.Device1", QDBusConnection::systemBus());
        QDBusConnection::systemBus().connect("org.bluez", m_devicePath, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
                                             this, SLOT(onPropertiesChangedInt(QString, QVariantMap, QStringList)));
    }

    qDebug() << Q_FUNC_INFO << path <<  m_deviceInterface->isValid();
}

void QBLEDevice::pair()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_deviceInterface) {
        return;
    }

    // A trusted device is stored by BlueZ: its object stays after a failed pairing and after a disconnect, so a
    // device that works without bonding can still be connected and reconnected without a scan
    trust(true);
    m_pairCallPending = true;
    // The user may confirm on both devices; the default D-Bus timeout of 25 s is too short for that
    QDBusMessage call = QDBusMessage::createMethodCall("org.bluez", m_devicePath, "org.bluez.Device1", "Pair");
    QDBusPendingCall pcall = QDBusConnection::systemBus().asyncCall(call, 60000);

    auto watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [&](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<void> reply(*w);
        w->deleteLater();
        m_pairCallPending = false;
        if (reply.error().name() == "org.bluez.Error.AlreadyExists") {
            // Bonded before, for example by another app: connect as after a pairing
            qDebug() << "Already paired, connecting";
            connectToDevice();
            return;
        }
        if (reply.error().name() == "org.bluez.Error.AuthenticationCanceled") {
            // The device ignored the pairing request (Bangle.js) or refused bonding: connect without bonding.
            // A rejection, a wrong passkey or no answer from the user ends with AuthenticationFailed instead.
            qDebug() << "Pairing not done by the device, connecting without bonding";
            connectToDevice();
            return;
        }
        if (reply.error().type() != QDBusError::NoError) {
            qDebug() << reply.error().name() << reply.error().message();
            Q_EMIT error(reply.error().message());
            // trust(false) changes Device1 while ServicesResolved may still be true; the device must not start
            m_holdUntilDisconnected = deviceProperty("Connected").toBool();
            trust(false);
            // Rejected, wrong passkey or timeout: the link is not encrypted, so the device must not start its setup
            disconnectFromDevice();
            return;
        }
        emitDeviceStateAfterPairing();
    });
}

void QBLEDevice::connectToDevice()
{
    qDebug() << "QBLEDevice::connectToDevice";
    m_holdUntilDisconnected = false;

    if (!m_deviceInterface) {
        return;
    }

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

    if (!m_deviceInterface) {
        return;
    }

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

void QBLEDevice::trust(bool arg)
{
    QDBusInterface dev("org.bluez", m_devicePath, "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QVariant qvar_arg = QVariant::fromValue(QDBusVariant(QVariant(arg)));
    dev.call("Set", "org.bluez.Device1", "Trusted", qvar_arg);
}

QBLEService* QBLEDevice::service(const QString &uuid) const
{
    return m_serviceMap.value(uuid, nullptr);
}

QString QBLEDevice::devicePath() const
{
    return m_devicePath;
}

void QBLEDevice::addService(const QString &uuid, QBLEService *s)
{
    m_serviceMap[uuid] = s;
}

void QBLEDevice::onPropertiesChangedInt(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    // On ServicesResolved the device starts GATT calls. A blocking call waits for the pairing, and the pairing waits
    // for the agent answer, which then cannot arrive. So the device sees no changes until Pair returns.
    if (m_pairCallPending) {
        qDebug() << Q_FUNC_INFO << "held back during pairing:" << map.keys();
        return;
    }
    if (m_holdUntilDisconnected) {
        if (map.value("Connected", true).toBool()) {
            qDebug() << Q_FUNC_INFO << "held back after failed pairing:" << map.keys();
            return;
        }
        m_holdUntilDisconnected = false;
    }
    emit propertiesChanged(interface, map, list);
}

void QBLEDevice::emitDeviceStateAfterPairing()
{
    QVariantMap map;
    map["Paired"] = deviceProperty("Paired");
    if (!deviceProperty("Connected").toBool()) {
        map["Connected"] = false;
    }
    qDebug() << Q_FUNC_INFO << map;
    Q_EMIT propertiesChanged("org.bluez.Device1", map, QStringList());
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
