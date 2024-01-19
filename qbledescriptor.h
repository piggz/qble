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

private:

    QDBusInterface *m_descriptorInterface;

    // https://doc.qt.io/qt-6/qbluetoothuuid.html#DescriptorType-enum
    // for example
    //   QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration is 0x2902
    //   but UUID is "00002902-0000-1000-8000-00805f9b34fb"
    QString m_uuid;

};

#endif // QBLEDESCRIPTOR_H
