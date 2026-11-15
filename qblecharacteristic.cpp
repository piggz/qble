#include "qblecharacteristic.h"

#include <QDomDocument>
#include <QDBusReply>
#include <QDebug>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>

#include <QDBusUnixFileDescriptor>
#include <unistd.h>

QBLECharacteristic::QBLECharacteristic(const QString &path, QObject *parent) : QObject(parent)
{
    m_characteristicInterface = new QDBusInterface("org.bluez", path, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus());

    m_path = path;
    m_uuid = m_characteristicInterface->property("UUID").toString();

    QStringList argumentMatch;
    argumentMatch << "org.bluez.GattCharacteristic1";
    QDBusConnection::systemBus().connect("org.bluez", path, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
                            this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
    introspect();
}

void QBLECharacteristic::onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    Q_UNUSED(interface);
    Q_UNUSED(list);
    //qDebug() << "Characteristic property changed" << interface << map;

    if (map.contains("Value")) {
        emit characteristicChanged(m_uuid, map["Value"].toByteArray());
    }
}

bool QBLECharacteristic::writeValue(const QByteArray &val, QString *errorMessage) const
{
    //qDebug() << Q_FUNC_INFO << val.toHex();
    QDBusReply<void> reply = m_characteristicInterface->call("WriteValue", val, QVariantMap());
    if (!reply.isValid()) {
        const QString message = reply.error().message();
        qWarning() << Q_FUNC_INFO << message;
        if (errorMessage != nullptr) {
            *errorMessage = message;
        }
        return false;
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    return true;
}

void QBLECharacteristic::writeAsync(const QByteArray &val) const
{
    m_characteristicInterface->asyncCall("WriteValue", val, QVariantMap());
}

bool QBLECharacteristic::writeAsyncChecked(const QByteArray &val)
{
    QDBusPendingCall async = m_characteristicInterface->asyncCall("WriteValue", val, QVariantMap());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    watcher->setProperty("value", val);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(writeFinished(QDBusPendingCallWatcher*)));

    return true;
}

QByteArray QBLECharacteristic::readValue() const
{
    QDBusReply<QByteArray> reply = m_characteristicInterface->call("ReadValue", QVariantMap());
    //qDebug() << reply.error().message();
    return reply.value();
}

void QBLECharacteristic::readAsync()
{
    QDBusPendingCall async = m_characteristicInterface->asyncCall("ReadValue", QVariantMap());
    const QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(readFinished(QDBusPendingCallWatcher*)));
}

void QBLECharacteristic::readFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QByteArray> reply = *call;
    if (reply.isError()) {
        qDebug() << "QBLECharacteristic::readFinished:" << reply.error().message();
    } else {
        m_value = reply.value();
        emit characteristicRead(m_uuid, m_value);
    }
    call->deleteLater();
}

void QBLECharacteristic::writeFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<void> reply = *call;
    const QByteArray v = call->property("value").toByteArray();

    if (reply.isError()) {
        emit characteristicWriteFailed(m_uuid, reply.error().message());
    } else {
        emit characteristicWritten(m_uuid, v);
    }

    call->deleteLater();
}

void QBLECharacteristic::startNotify() const
{
    m_characteristicInterface->call("StartNotify");
}

void QBLECharacteristic::stopNotify() const
{
    m_characteristicInterface->call("StopNotify");
}

// BlueZ sends every notification as one packet on the socket. With StartNotify, notifications that come
// close together can be merged into one PropertiesChanged signal, and the earlier values are lost.
bool QBLECharacteristic::acquireNotify()
{
    if (m_notifyFd >= 0) {
        return true;
    }

    QDBusMessage reply = m_characteristicInterface->call("AcquireNotify", QVariantMap());
    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().count() < 2) {
        qWarning() << Q_FUNC_INFO << m_uuid << reply.errorMessage();
        return false;
    }

    QDBusUnixFileDescriptor fd = reply.arguments().at(0).value<QDBusUnixFileDescriptor>();
    if (!fd.isValid()) {
        qWarning() << Q_FUNC_INFO << m_uuid << "Invalid file descriptor";
        return false;
    }

    // QDBusUnixFileDescriptor closes its descriptor when it goes away, so keep a copy.
    m_notifyFd = ::dup(fd.fileDescriptor());
    m_notifyMtu = reply.arguments().at(1).toInt();
    m_notifyNotifier = new QSocketNotifier(m_notifyFd, QSocketNotifier::Read, this);
    connect(m_notifyNotifier, &QSocketNotifier::activated, this, &QBLECharacteristic::readNotifySocket);
    qDebug() << Q_FUNC_INFO << m_uuid << "MTU" << m_notifyMtu;
    return true;
}

QBLECharacteristic::~QBLECharacteristic()
{
    releaseNotify();
}

void QBLECharacteristic::releaseNotify()
{
    if (m_notifyFd < 0) {
        return;
    }
    delete m_notifyNotifier;
    m_notifyNotifier = nullptr;
    ::close(m_notifyFd);
    m_notifyFd = -1;
}

void QBLECharacteristic::readNotifySocket()
{
    QByteArray buffer(qMax(m_notifyMtu, 512), 0);
    ssize_t length = ::read(m_notifyFd, buffer.data(), buffer.size());
    if (length <= 0) {
        // The device disconnected or BlueZ released the socket.
        qDebug() << Q_FUNC_INFO << m_uuid << "Notify socket closed";
        releaseNotify();
        return;
    }
    buffer.resize(length);
    m_value = buffer;
    emit characteristicChanged(m_uuid, m_value);
}

QByteArray QBLECharacteristic::value() const
{
    return m_value;
}


void QBLECharacteristic::introspect()
{
    QDBusInterface miIntro("org.bluez", m_path, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = miIntro.call("Introspect");
    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");


    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("desc")) {
            QString path = m_path + "/" + nodeName;
            QDBusInterface descInterface("org.bluez", path, "org.bluez.GattDescriptor1", QDBusConnection::systemBus(), 0);
                m_descriptorMap[descInterface.property("UUID").toString()] = new QBLEDescriptor(path, this);
        }
    }

    qDebug() << Q_FUNC_INFO << "descriptors" << m_descriptorMap.keys();

}

QBLEDescriptor *QBLECharacteristic::descriptor(const QString &c) const
{
    return m_descriptorMap.value(c, nullptr);
}
