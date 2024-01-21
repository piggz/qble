#ifndef QBLEDESCRIPTOR_H
#define QBLEDESCRIPTOR_H

#include <QObject>
#include <QVariantMap>
#include <QtDBus>

class QBLEDescriptor : public QObject
{
    Q_OBJECT
public:
    explicit QBLEDescriptor(const QString &path, QObject *parent = nullptr);

    void writeValue(const QByteArray &val) const;
    void writeAsync(const QByteArray &val) const;
    QByteArray readValue() const;
    void readAsync();
    QByteArray value() const; //Returns last buffered value without a read

    void startNotify() const;
    void stopNotify() const;

    Q_SIGNAL void descriptorChanged(const QString &characterisitc, const QByteArray &value);
    Q_SIGNAL void descriptorRead(const QString &characterisitc, const QByteArray &value);


private:

    QDBusInterface *m_descriptorInterface;

    Q_SLOT void onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list);
    Q_SLOT void readFinished(QDBusPendingCallWatcher *call);


    // https://doc.qt.io/qt-6/qbluetoothuuid.html#DescriptorType-enum
    // for example
    //   QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration is 0x2902
    //   but UUID is "00002902-0000-1000-8000-00805f9b34fb"
    QString m_uuid;

    QByteArray m_value; //buffered value;

};

#endif // QBLEDESCRIPTOR_H
