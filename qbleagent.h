#ifndef QBLEAGENT__H
#define QBLEAGENT__H
#include <QObject>
#include <QtDBus>

class QBLEAgent : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")
public:
    explicit QBLEAgent(QObject *parent = nullptr) : QObject(parent) { }

    void registerAgent();
    void unregisterAgent();


public slots:
    void Release();
    void Cancel();

    uint RequestPasskey(const QDBusObjectPath &device);
    QString RequestPinCode(const QDBusObjectPath &device);
    void RequestConfirmation(const QDBusObjectPath &device, uint passkey);
    void RequestAuthorization(const QDBusObjectPath &device)  {
        qWarning() << "TODO: Not implemented" << Q_FUNC_INFO << device.path() ;}
    void AuthorizeService(const QDBusObjectPath &device, const QString &in1) {
        qWarning() << "TODO: Not implemented" << Q_FUNC_INFO << device.path() << in1;}
    void DisplayPasskey(const QDBusObjectPath& device, uint passkey, ushort entered);
    void DisplayPinCode(const QDBusObjectPath &device, const QString &in1) {
        qWarning() << "TODO: Not implemented" << Q_FUNC_INFO << device.path() << in1 ;}

    QString getDeviceName(const QDBusObjectPath& devicePath) ;

    Q_INVOKABLE void requestConfirmationResponse(const bool accept);
    Q_INVOKABLE void requestPasskeyResponse(const uint response);
    Q_INVOKABLE void requestPinCodeResponse(const QString& response);

signals:
    void UIRequestPasskey(const QString &deviceName, const QString &devicePath);
    void UIRequestPinCode(const QString &deviceName, const QString &devicePath);
    void UIRequestConfirmation(const QString &deviceName, const QString &devicePath, uint passkey);
    void UIDisplayPasskey(const QString& deviceName, const QString& devicePath, uint passkey);

    void pairingAccepted(const QString &devicePath);

private:
    void holdRequest(const QDBusObjectPath& device);
    QString deviceNameOrPath(const QDBusObjectPath& device);
    bool isPending(const QString& member) const;
    void sendReply(const QDBusMessage& reply, bool accepted);

    QDBusMessage m_pendingRequest;
    QString m_pendingDevicePath;
};

#endif // QBLEAGENT__H
