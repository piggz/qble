#ifndef QBLECHARACTERISTIC_H
#define QBLECHARACTERISTIC_H

#include "qbledescriptor.h"

#include <QObject>
#include <QVariantMap>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>

#include <QSocketNotifier>

class QBLECharacteristic : public QObject
{
    Q_OBJECT
public:
    explicit QBLECharacteristic(const QString &path, QObject *parent = nullptr);
    ~QBLECharacteristic() override;

    bool writeValue(const QByteArray &val, QString *errorMessage = nullptr) const;
    void writeAsync(const QByteArray &val) const;
    bool writeAsyncChecked(const QByteArray &val);
    QByteArray readValue() const;
    void readAsync();
    QByteArray value() const; //Returns last buffered value without a read

    void startNotify() const;
    void stopNotify() const;
    bool acquireNotify();
    void releaseNotify();

    Q_SIGNAL void characteristicChanged(const QString &characterisitc, const QByteArray &value);
    Q_SIGNAL void characteristicRead(const QString &characterisitc, const QByteArray &value);
    Q_SIGNAL void characteristicWritten(const QString &characteristic, const QByteArray &value);
    Q_SIGNAL void characteristicWriteFailed(const QString &characteristic, const QString &errorMessage);

    QBLEDescriptor *descriptor(const QString &c) const;

private:
    QString m_path;
    QString m_uuid;
    QByteArray m_value; //buffered value;
    QDBusInterface *m_characteristicInterface;
    int m_notifyFd = -1;
    int m_notifyMtu = 0;
    QSocketNotifier* m_notifyNotifier = nullptr;

    Q_SLOT void onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list);
    Q_SLOT void readFinished(QDBusPendingCallWatcher *call);
    Q_SLOT void writeFinished(QDBusPendingCallWatcher *call);
    Q_SLOT void readNotifySocket();

    QMap<QString, QBLEDescriptor*> m_descriptorMap;

    void introspect();

};

#endif // QBLECHARACTERISTIC_H
