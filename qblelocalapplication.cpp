#include "qblelocalapplication.h"
#include "qblelocalservice.h"
#include "qblelocalcharacteristic.h"

#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDebug>

static const char *BLUEZ_SERVICE       = "org.bluez";
static const char *GATT_MANAGER_IFACE  = "org.bluez.GattManager1";
static const char *GATT_SERVICE_IFACE  = "org.bluez.GattService1";
static const char *GATT_CHRC_IFACE     = "org.bluez.GattCharacteristic1";

QBLELocalApplication::QBLELocalApplication(const QString &path, QObject *parent)
    : QObject(parent), m_path(path), m_bus(QDBusConnection::systemBus())
{
    qDBusRegisterMetaType<InterfaceList>();
    qDBusRegisterMetaType<ManagedObjectList>();

    if (!m_bus.registerObject(m_path, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties)) {
        qCritical() << Q_FUNC_INFO << "Failed to register GATT application object at" << m_path << ":" << m_bus.lastError().message();
    } else {
        qDebug() << Q_FUNC_INFO << "GATT application registered at" << m_path;
    }
}

QBLELocalApplication::~QBLELocalApplication()
{
    m_bus.unregisterObject(m_path);
}

QDBusObjectPath QBLELocalApplication::path() const
{
    return QDBusObjectPath(m_path);
}

void QBLELocalApplication::addService(QBLELocalService *service)
{
    m_services.append(service);
}

bool QBLELocalApplication::registerWithAdapter(const QString &adapterPath)
{
    qDebug() << Q_FUNC_INFO << "Registering GATT application with adapter" << adapterPath;

    QDBusInterface gattManager(BLUEZ_SERVICE, adapterPath, GATT_MANAGER_IFACE, m_bus);
    if (!gattManager.isValid()) {
        qWarning() << Q_FUNC_INFO << "GattManager1 not available on" << adapterPath << ":" << gattManager.lastError().message();
        return false;
    }

    QDBusPendingCall reply = gattManager.asyncCall(
        QStringLiteral("RegisterApplication"),
        QVariant::fromValue(path()),
        QVariantMap());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [adapterPath](QDBusPendingCallWatcher *w) {
                QDBusPendingReply<> reply = *w;
                if (reply.isError()) {
                    qWarning() << "GATT RegisterApplication failed on" << adapterPath << ":" << reply.error().message();
                } else {
                    qInfo() << "GATT application successfully registered with" << adapterPath;
                }
                w->deleteLater();
            });

    return true;
}

void QBLELocalApplication::unregisterFromAdapter(const QString &adapterPath)
{
    qDebug() << Q_FUNC_INFO << "Unregistering GATT application from" << adapterPath;

    QDBusInterface gattManager(BLUEZ_SERVICE, adapterPath, GATT_MANAGER_IFACE, m_bus);
    if (gattManager.isValid()) {
        gattManager.asyncCall(QStringLiteral("UnregisterApplication"), QVariant::fromValue(path()));
    }
}

ManagedObjectList QBLELocalApplication::GetManagedObjects()
{
    ManagedObjectList result;

    for (QBLELocalService *svc : m_services) {
        // --- Service entry ---
        InterfaceList svcInterfaces;
        QVariantMap svcProps;
        svcProps.insert(QStringLiteral("UUID"), svc->uuid());
        svcProps.insert(QStringLiteral("Primary"), svc->primary());
        svcProps.insert(QStringLiteral("Characteristics"),
                        QVariant::fromValue(svc->characteristicPaths()));
        svcInterfaces.insert(QLatin1String(GATT_SERVICE_IFACE), svcProps);
        result.insert(svc->path(), svcInterfaces);

        // --- Characteristic entries ---
        for (QBLELocalCharacteristic *ch : svc->characteristics()) {
            InterfaceList chInterfaces;
            QVariantMap chProps;
            chProps.insert(QStringLiteral("Service"),  QVariant::fromValue(ch->servicePath()));
            chProps.insert(QStringLiteral("UUID"),     ch->uuid());
            chProps.insert(QStringLiteral("Flags"),    ch->flags());
            chProps.insert(QStringLiteral("Descriptors"),
                           QVariant::fromValue(ch->descriptorPaths()));
            chInterfaces.insert(QLatin1String(GATT_CHRC_IFACE), chProps);
            result.insert(ch->path(), chInterfaces);
        }
    }

    return result;
}
