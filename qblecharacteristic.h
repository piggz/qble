#ifndef QBLECHARACTERISTIC_H
#define QBLECHARACTERISTIC_H

#include <QObject>
#include <QVariantMap>
#include <QtDBus>
#include "qbledescriptor.h"

class QBLECharacteristic : public QObject
{
    Q_OBJECT
public:
    explicit QBLECharacteristic(const QString &path, QObject *parent = nullptr);

    void writeValue(const QByteArray &val) const;
    void writeAsync(const QByteArray &val) const;
    QByteArray readValue() const;
    void readAsync();
    QByteArray value() const; //Returns last buffered value without a read

    void startNotify() const;
    void stopNotify() const;

    Q_SIGNAL void characteristicChanged(const QString &characterisitc, const QByteArray &value);
    Q_SIGNAL void characteristicRead(const QString &characterisitc, const QByteArray &value);

    QBLEDescriptor *descriptor(const QString &c) const;

private:
    QString m_path;
    QString m_uuid;
    QByteArray m_value; //buffered value;
    QDBusInterface *m_characteristicInterface;

    Q_SLOT void onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list);
    Q_SLOT void readFinished(QDBusPendingCallWatcher *call);


    QMap<QString, QBLEDescriptor*> m_descriptorMap;

    void introspect();

};

#endif // QBLECHARACTERISTIC_H
