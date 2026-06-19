#ifndef QBLELOCALCHARACTERISTIC_H
#define QBLELOCALCHARACTERISTIC_H

#include <QObject>
#include <QtDBus>

// Forward declaration
class QBLELocalService;

/**
 * @brief Server-side GATT characteristic exposed over D-Bus using the BlueZ GATT API.
 *
 * Implements org.bluez.GattCharacteristic1. Subclass this and override
 * ReadValue() and/or WriteValue() to respond to requests from the remote
 * BLE device (e.g. an InfiniTime watch acting as GATT client).
 */
class QBLELocalCharacteristic : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattCharacteristic1")
    Q_PROPERTY(QDBusObjectPath Service READ servicePath)
    Q_PROPERTY(QString UUID READ uuid)
    Q_PROPERTY(QStringList Flags READ flags)
    Q_PROPERTY(QList<QDBusObjectPath> Descriptors READ descriptorPaths)

public:
    explicit QBLELocalCharacteristic(QDBusConnection bus, unsigned int index, const QString &uuid, const QStringList &flags, QBLELocalService *service, QObject *parent = nullptr);

    QDBusObjectPath path() const;
    QDBusObjectPath servicePath() const;
    QString uuid() const;
    QStringList flags() const;
    QList<QDBusObjectPath> descriptorPaths() const; // empty for now

Q_SIGNALS:
    void valueWritten(const QByteArray &value);

public Q_SLOTS:
    // BlueZ GATT server callbacks — override in subclass as needed
    virtual QByteArray ReadValue(const QVariantMap &options);
    virtual void WriteValue(const QByteArray &value, const QVariantMap &options);
    virtual void StartNotify();
    virtual void StopNotify();

private:
    QString m_path;
    QString m_uuid;
    QStringList m_flags;
    QBLELocalService *m_service;
};

#endif // QBLELOCALCHARACTERISTIC_H
