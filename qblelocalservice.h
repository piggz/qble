#ifndef QBLELOCALSERVICE_H
#define QBLELOCALSERVICE_H

#include <QObject>
#include <QList>
#include <QString>
#include <QtDBus>

class QBLELocalCharacteristic;

/**
 * @brief Server-side GATT service exposed over D-Bus using the BlueZ GATT API.
 *
 * Implements org.bluez.GattService1. Add characteristics with addCharacteristic().
 * The service is registered at a path like /uk/co/piggz/amazfish/gatt/service<index>.
 *
 * BlueZ will discover this service when the application is registered via
 * QBLELocalApplication::registerWithAdapter().
 */
class QBLELocalService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattService1")
    Q_PROPERTY(QString UUID READ uuid)
    Q_PROPERTY(bool Primary READ primary)
    Q_PROPERTY(QList<QDBusObjectPath> Characteristics READ characteristicPaths)

public:
    explicit QBLELocalService(QDBusConnection bus, unsigned int index, const QString &uuid, const QString &appPath, QObject *parent = nullptr);

    QDBusObjectPath path() const;
    void addCharacteristic(QBLELocalCharacteristic *ch);
    QList<QDBusObjectPath> characteristicPaths() const;
    QList<QBLELocalCharacteristic *> characteristics() const;

    QString uuid() const;
    bool primary() const;

private:
    QString m_path;
    QString m_uuid;
    QList<QBLELocalCharacteristic *> m_characteristics;
};

#endif // QBLELOCALSERVICE_H
