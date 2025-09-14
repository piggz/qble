#ifndef QBLEAGENT__H
#define QBLEAGENT__H
#include <QObject>
#include <QtDBus>
#include <QEventLoop>
#include <optional>

class QBLEAgent : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")
public:
    explicit QBLEAgent(QObject *parent = nullptr) : QObject(parent) { }

    void registerAgent();
    void unregisterAgent();


public slots:
    void Release() {}
    void Cancel() {}

    uint RequestPasskey(const QDBusObjectPath &device);
    QString RequestPinCode(const QDBusObjectPath &device);
    void RequestConfirmation(const QDBusObjectPath &device, uint passkey);
    void RequestAuthorization(const QDBusObjectPath &device)  {
        qDebug() << "TODO TODO TODO TODO TODO TODO TODO TODO TODO" << Q_FUNC_INFO << device.path() ;}
    void AuthorizeService(const QDBusObjectPath &device, const QString &in1) {
        qDebug() << "TODO TODO TODO TODO TODO TODO TODO TODO TODO" << Q_FUNC_INFO << device.path() << in1;}
    void DisplayPasskey(const QDBusObjectPath &device, uint in1, ushort in2) {
        qDebug() << "TODO TODO TODO TODO TODO TODO TODO TODO TODO" << Q_FUNC_INFO << device.path() << in1 << in2;}
    void DisplayPinCode(const QDBusObjectPath &device, const QString &in1) {
        qDebug() << "TODO TODO TODO TODO TODO TODO TODO TODO TODO" << Q_FUNC_INFO << device.path() << in1 ;}

    QString getDeviceName(const QDBusObjectPath& devicePath) ;

    Q_INVOKABLE void requestConfirmationResponse(const bool accept);
    Q_INVOKABLE void requestPasskeyResponse(const uint response);
    Q_INVOKABLE void requestPinCodeResponse(const QString& response);

signals:
    void UIRequestPasskey(const QString &deviceName, const QString &devicePath);
    void UIRequestPinCode(const QString &deviceName, const QString &devicePath);
    void UIRequestConfirmation(const QString &deviceName, const QString &devicePath, uint passkey);

    void pairingAccepted(const QString &devicePath);

private:

    std::optional<uint> m_request_passkey_response;
    QString m_request_pincode_response;
    std::optional<bool> m_request_confirmation_response;

    QEventLoop *m_wait_for_response_loop = nullptr;
};

#endif // QBLEAGENT__H
