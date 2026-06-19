#ifndef QBLELOCALAPPLICATION_H
#define QBLELOCALAPPLICATION_H

#include <QObject>
#include <QList>
#include <QString>
#include <QtDBus>

class QBLELocalService;
class QBLELocalCharacteristic;

// D-Bus type aliases required by the BlueZ ObjectManager interface
typedef QMap<QString, QVariantMap>              InterfaceList;
typedef QMap<QDBusObjectPath, InterfaceList>    ManagedObjectList;

Q_DECLARE_METATYPE(InterfaceList)
Q_DECLARE_METATYPE(ManagedObjectList)

/**
 * @brief GATT application root that implements org.freedesktop.DBus.ObjectManager.
 *
 * BlueZ calls GetManagedObjects() on this object to enumerate all GATT services
 * and characteristics offered by the application.
 *
 * Usage:
 *   1. Create a QBLELocalApplication.
 *   2. Add QBLELocalService instances with addService().
 *   3. Call registerWithAdapter() with the BlueZ adapter D-Bus path.
 *   4. BlueZ will then advertise and handle the registered services.
 *
 * The application is registered on the system D-Bus at the path provided in the
 * constructor (default: "/uk/co/piggz/amazfish/gatt").
 */
class QBLELocalApplication : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.ObjectManager")

public:
    explicit QBLELocalApplication(const QString &path = QStringLiteral("/uk/co/piggz/amazfish/gatt"), QObject *parent = nullptr);
    ~QBLELocalApplication() override;

    QDBusObjectPath path() const;
    void addService(QBLELocalService *service);

    /**
     * Register this application with the BlueZ GattManager1 on the given adapter.
     * @param adapterPath  e.g. "/org/bluez/hci0" from AmazfishConfig::localAdapter()
     * @return true on success
     */
    bool registerWithAdapter(const QString &adapterPath);

    /**
     * Unregister this application from the BlueZ GattManager1.
     * Call this on shutdown or when the adapter disappears.
     */
    void unregisterFromAdapter(const QString &adapterPath);

public Q_SLOTS:
    // Called by BlueZ to enumerate all GATT objects managed by this application
    ManagedObjectList GetManagedObjects();

private:
    QString m_path;
    QList<QBLELocalService *> m_services;
    QDBusConnection m_bus;
};

#endif // QBLELOCALAPPLICATION_H
