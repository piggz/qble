#include "qbleagent.h"

void QBLEAgent::Release()
{
    // BlueZ tells the agent to release resources (e.g., when unregistering).
    m_pendingRequest = QDBusMessage();
}

void QBLEAgent::Cancel()
{
    // BlueZ cancels the request (timeout or pairing failed); a late answer from the UI is ignored.
    qDebug() << Q_FUNC_INFO << m_pendingRequest.member();
    m_pendingRequest = QDBusMessage();
}

void QBLEAgent::registerAgent() {
    qDebug() << Q_FUNC_INFO;
    bool ok = QDBusConnection::systemBus().registerObject("/qble/agent", this, QDBusConnection::ExportAllSlots);
    if (!ok) {
        qWarning() << "Could not register agent object on DBus!" << QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1", QDBusConnection::systemBus());
    QDBusReply<void> reply = agentManager.call(QStringLiteral("RegisterAgent"), QVariant::fromValue(QDBusObjectPath("/qble/agent")), QVariant::fromValue(QString(""))); // "KeyboardDisplay" | "NoInputNoOutput" | "DisplayYesNo"
    if (!reply.isValid()) {
        qWarning() << "Failed to register agent:" << reply.error().message();
        return;
    }

    QDBusReply<void> reply2 = agentManager.call(QStringLiteral("RequestDefaultAgent"), QVariant::fromValue(QDBusObjectPath("/qble/agent")));
    if (!reply2.isValid()) {
        qWarning() << "Failed to register default agent:" << reply2.error().message();
        return;
    }

}

void QBLEAgent::unregisterAgent() {

    QDBusInterface agentManager("org.bluez", "/org/bluez", "org.bluez.AgentManager1", QDBusConnection::systemBus());
    agentManager.call("UnregisterAgent",  QVariant::fromValue(QDBusObjectPath("/qble/agent")));

    QDBusConnection::systemBus().unregisterObject("/qble/agent");
}

uint QBLEAgent::RequestPasskey(const QDBusObjectPath& device)
{
    qDebug() << Q_FUNC_INFO << device.path();
    holdRequest(device);
    emit UIRequestPasskey(deviceNameOrPath(device), device.path());
    return 0; // not sent, the reply goes from requestPasskeyResponse()
}

QString QBLEAgent::RequestPinCode(const QDBusObjectPath& device)
{
    qDebug() << Q_FUNC_INFO << device.path();
    holdRequest(device);
    emit UIRequestPinCode(deviceNameOrPath(device), device.path());
    return QString(); // not sent, the reply goes from requestPinCodeResponse()
}

void QBLEAgent::RequestConfirmation(const QDBusObjectPath& device, uint passkey)
{
    qDebug() << Q_FUNC_INFO << device.path() << passkey;
    holdRequest(device);
    emit UIRequestConfirmation(deviceNameOrPath(device), device.path(), passkey);
}

void QBLEAgent::DisplayPasskey(const QDBusObjectPath& device, uint passkey, ushort entered)
{
    qDebug() << Q_FUNC_INFO << device.path() << passkey << entered;
    // BlueZ calls this again for each key typed on the device; the UI shows the passkey once
    if (entered == 0) {
        emit UIDisplayPasskey(deviceNameOrPath(device), device.path(), passkey);
    }
}

// The reply to BlueZ is sent later, when the UI answers. The D-Bus slot returns at once, so nothing blocks meanwhile.
void QBLEAgent::holdRequest(const QDBusObjectPath& device)
{
    setDelayedReply(true);
    m_pendingRequest = message();
    m_pendingDevicePath = device.path();
}

QString QBLEAgent::deviceNameOrPath(const QDBusObjectPath& device)
{
    QString deviceName = getDeviceName(device);
    if (deviceName.isEmpty()) {
        deviceName = device.path();
    }
    return deviceName;
}

// Returns false when no request with this method waits, for example after Cancel()
bool QBLEAgent::isPending(const QString& member) const
{
    if (m_pendingRequest.type() != QDBusMessage::MethodCallMessage || m_pendingRequest.member() != member) {
        qWarning() << Q_FUNC_INFO << "no pending" << member;
        return false;
    }
    return true;
}

void QBLEAgent::sendReply(const QDBusMessage& reply, bool accepted)
{
    QDBusConnection::systemBus().send(reply);
    m_pendingRequest = QDBusMessage();
    if (accepted) {
        emit pairingAccepted(m_pendingDevicePath);
    }
}

QString QBLEAgent::getDeviceName(const QDBusObjectPath& devicePath) {
    QDBusInterface deviceIface(
        "org.bluez", devicePath.path(),
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus()
        );

    QDBusReply<QVariant> reply = deviceIface.call(
        "Get", "org.bluez.Device1", "Name"
        );
    if (reply.isValid()) {
        return reply.value().toString();
    }
    return QString();
}

void QBLEAgent::requestPasskeyResponse(const uint response)
{
    qDebug() << Q_FUNC_INFO << response;
    if (isPending("RequestPasskey")) {
        sendReply(m_pendingRequest.createReply(QVariant::fromValue(response)), true);
    }
}

void QBLEAgent::requestPinCodeResponse(const QString& response)
{
    qDebug() << Q_FUNC_INFO << response;
    if (!isPending("RequestPinCode")) {
        return;
    }
    if (response.isEmpty()) {
        sendReply(m_pendingRequest.createErrorReply("org.bluez.Error.Rejected", "No pin code entered"), false);
        return;
    }
    sendReply(m_pendingRequest.createReply(response), true);
}

void QBLEAgent::requestConfirmationResponse(const bool response)
{
    qDebug() << Q_FUNC_INFO << response;
    if (!isPending("RequestConfirmation")) {
        return;
    }
    if (!response) {
        sendReply(m_pendingRequest.createErrorReply("org.bluez.Error.Rejected", "User rejected pairing"), false);
        return;
    }
    sendReply(m_pendingRequest.createReply(), true);
}
