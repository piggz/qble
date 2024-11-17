#include "adaptermodel.h"

typedef QMap<QString, QVariantMap> InterfaceList;
typedef QMap<QDBusObjectPath, InterfaceList> ManagedObjectList;

Q_DECLARE_METATYPE(InterfaceList)
Q_DECLARE_METATYPE(ManagedObjectList)

AdapterModel::AdapterModel()
{
    qDBusRegisterMetaType<InterfaceList>();
    qDBusRegisterMetaType<ManagedObjectList>();

    m_objectManager = new QDBusInterface("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());

    QDBusPendingReply<ManagedObjectList> reply = m_objectManager->call("GetManagedObjects");
    reply.waitForFinished();
    if (reply.isError())
        return;

    for (const QDBusObjectPath &path: static_cast<const QList<QDBusObjectPath>>(reply.value().keys())) {
        const InterfaceList ifaceList = reply.value().value(path);
        for (const QString &iface: static_cast<const QList<QString>>(ifaceList.keys())) {
            if (iface == QStringLiteral("org.bluez.Adapter1")) {
               m_devices << path.path();
               m_deviceNames << ifaceList.value(iface).value(QStringLiteral("Name")).toString();
               m_deviceAliases << ifaceList.value(iface).value(QStringLiteral("Alias")).toString();
               m_deviceAddresses << ifaceList.value(iface).value(QStringLiteral("Address")).toString();
            }
        }
    }
}

int AdapterModel::rowCount(const QModelIndex &parent) const
{
    return m_devices.count();
}

QVariant AdapterModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() < m_devices.length()) {
        if (role == AdapterPath || role == ItemText) {
            return m_devices.at(index.row());
        } else if (role == AdapterName) {
            return m_deviceNames.at(index.row());
        } else if (role == AdapterAlias) {
            return m_deviceAliases.at(index.row());
        } else if (role == AdapterAddress) {
            return m_deviceAddresses.at(index.row());
        }
    }
    return QVariant();
}

QHash<int, QByteArray> AdapterModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[AdapterPath] = "path";
    names[AdapterName] = "name";
    names[AdapterAlias] = "alias";
    names[AdapterAddress] = "address";
    names[ItemText] = "itemText";

    return names;
}

QVariantMap AdapterModel::get(int row) const {
    if (row < 0 || row >= m_devices.size()) {
        return QVariantMap();
    }

    QVariantMap data;
    data["path"] = m_devices.at(row);
    data["itemText"] = data["path"];
    data["name"] = m_deviceNames.at(row);
    data["alias"] = m_deviceAliases.at(row);
    data["address"] = m_deviceAddresses.at(row);

    return data;
}
