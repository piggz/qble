#include "qbleagent.h"

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
    if (deviceName.isEmpty())
        deviceName = device.path();
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
    if (deviceName.isEmpty())
        deviceName = device.path();
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
    qDebug() << Q_FUNC_INFO << "RequestConfirmation called!" << device.path() << passkey;
    QString deviceName = getDeviceName(device);
    if (deviceName.isEmpty()) {
        deviceName = device.path();
    }

    m_pending_confirmation_call = message();
    qDebug() << m_pending_confirmation_call.type();

    emit UIRequestConfirmation(deviceName, device.path(), passkey);

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

void QBLEAgent::requestConfirmationResponse(const bool accepted) {

    if (m_pending_confirmation_call.type() == QDBusMessage::InvalidMessage) {
        qWarning() << Q_FUNC_INFO << "No pending DBus call!";
        return;
    }

    QDBusConnection bus = QDBusConnection::systemBus();

    if (accepted) {
        qDebug() << Q_FUNC_INFO << "User accepted pairing";

        QDBusMessage reply = m_pending_confirmation_call.createReply();
        if (!bus.send(reply)) {
            qWarning() << Q_FUNC_INFO << "Failed to send reply:" << bus.lastError().name() << bus.lastError().message();
        } else {
            qDebug() << Q_FUNC_INFO << "Reply sent successfully";
        }

        const QString device = m_pending_confirmation_call.arguments().at(0).toString();
        qDebug() << Q_FUNC_INFO << device;
        emit pairingAccepted(device);

    } else {
        qDebug() << Q_FUNC_INFO << "User rejected pairing";

        QDBusMessage err =
            m_pending_confirmation_call.createErrorReply(
                "org.bluez.Error.Rejected",
                "User rejected pairing"
                );

        if (!bus.send(err)) {
            qWarning() << Q_FUNC_INFO << "Failed to send error reply:" << bus.lastError().name() << bus.lastError().message();
        } else {
            qDebug() << Q_FUNC_INFO << "Error reply sent successfully";
        }
    }

    m_pending_confirmation_call = QDBusMessage();

}
