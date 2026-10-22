#include "qbleagent.h"

void QBLEAgent::Release() {
    // BlueZ tells the agent to release resources (e.g., when unregistering).
    if (m_wait_for_response_loop) {
        m_wait_for_response_loop->quit();
        m_wait_for_response_loop = nullptr;
    }
}

void QBLEAgent::Cancel() {
    // BlueZ cancels an in-flight request; unblock any nested wait loop.
    if (m_wait_for_response_loop) {
        m_wait_for_response_loop->quit();
        m_wait_for_response_loop = nullptr;
    }
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

uint QBLEAgent::RequestPasskey(const QDBusObjectPath &device) {
    qDebug() << Q_FUNC_INFO << "RequestPasskey called!" << device.path();

    m_request_passkey_response = std::nullopt;
    QString deviceName = getDeviceName(device);
    if (deviceName.isEmpty()) {
        deviceName = device.path();
    }
    QEventLoop loop;
    m_wait_for_response_loop = &loop;
    emit UIRequestPasskey(deviceName, device.path());
    loop.exec();
    if (m_request_passkey_response.has_value()) {
        emit pairingAccepted(device.path());
    }
    qDebug() << Q_FUNC_INFO << "response" << m_request_passkey_response.value_or(0);
    return m_request_passkey_response.value_or(0);
}

QString QBLEAgent::RequestPinCode(const QDBusObjectPath &device) {

    m_request_pincode_response = "";
    QString deviceName = getDeviceName(device);
    if (deviceName.isEmpty()) {
        deviceName = device.path();
    }
    QEventLoop loop;
    m_wait_for_response_loop = &loop;
    emit UIRequestPinCode(deviceName, device.path());
    loop.exec();
    if (m_request_pincode_response != "") {
        emit pairingAccepted(device.path());
    }
    qDebug() << Q_FUNC_INFO << "response" << m_request_pincode_response;
    return m_request_pincode_response;

}

void QBLEAgent::RequestConfirmation(const QDBusObjectPath &device, uint passkey) {
    m_request_confirmation_response = std::nullopt;
    QString deviceName = getDeviceName(device);
    if (deviceName.isEmpty()) {
        deviceName = device.path();
    }
    QEventLoop loop;
    m_wait_for_response_loop = &loop;
    emit UIRequestConfirmation(deviceName, device.path(), passkey);
    loop.exec();

    qDebug() << Q_FUNC_INFO << "response" << (m_request_confirmation_response.has_value()
                                                 ? (m_request_confirmation_response.value() ? "accepted" : "rejected")
                                                 : "canceled/timeout");

    if (m_request_confirmation_response.has_value() && m_request_confirmation_response.value()) {
        emit pairingAccepted(device.path());
        return;
    }

    // IMPORTANT: Don't throw exceptions here — we must send a DBus reply (error) back to BlueZ.
    if (!m_request_confirmation_response.has_value()) {
        sendErrorReply(QStringLiteral("org.bluez.Error.Canceled"), QStringLiteral("Pairing canceled"));
        return;
    }
    sendErrorReply(QStringLiteral("org.bluez.Error.Rejected"), QStringLiteral("User rejected pairing"));
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

void QBLEAgent::requestPasskeyResponse(const uint response) {
    m_request_passkey_response = response;
    if (m_wait_for_response_loop) {
        m_wait_for_response_loop->quit();
        m_wait_for_response_loop = nullptr;
    }
}

void QBLEAgent::requestPinCodeResponse(const QString& response) {
    m_request_pincode_response = response;
    if (m_wait_for_response_loop) {
        m_wait_for_response_loop->quit();
        m_wait_for_response_loop = nullptr;
    }
}

void QBLEAgent::requestConfirmationResponse(const bool response) {
    m_request_confirmation_response = response;
        if (m_wait_for_response_loop) {
        m_wait_for_response_loop->quit();
        m_wait_for_response_loop = nullptr;
    }
}
