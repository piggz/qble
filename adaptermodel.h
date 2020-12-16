#ifndef ADAPTERMODEL_H
#define ADAPTERMODEL_H

#include <QAbstractListModel>
#include <QtDBus>

class AdapterModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AdapterModel();
    enum Roles {
        AdapterPath = Qt::UserRole + 1,
        AdapterName,
        ItemText
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> 	roleNames() const override;

private:
     QDBusInterface *m_objectManager = nullptr;
     QStringList m_devices;
     QStringList m_deviceNames;

};

#endif // ADAPTERMODEL_H
