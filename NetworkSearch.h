#ifndef NETWORKSEARCH_H
#define NETWORKSEARCH_H

#include <QObject>
#include <QUdpSocket>
#include <QHash>

struct NetworkNodeInfo {
    quint16 index = 0;
    quint16 moduleID = 0;
    QString name = "";
    QString ip = "";
    QString mac = "";
};
Q_DECLARE_METATYPE(NetworkNodeInfo)

class NetworkSearch : public QObject
{
    Q_OBJECT

    enum Step {
        None,
        Broadcast,
        QueryModule
    } m_step = None;

public:
    explicit NetworkSearch(QObject *parent = 0);
    ~NetworkSearch();

    void search4AllNodes();


signals:
    void signal_nodeListFound(const QList<NetworkNodeInfo> &nodeList);
    void signal_nodeInfoCatched(const NetworkNodeInfo &nodeinfo); // 最大支持65536个节点

public slots:

private slots:
    void readData();

private:
    void getNetworkAddress(const QByteArrayList &list);
    void sendGetModuleInfoRequest(const QString &ip, int type);
    void getModuleInfo(const QByteArrayList &list);

    void printNodeInfo();

private:
    QUdpSocket *m_udpSocket;
    quint16 m_seqNumber = 0; // 0-65535循环，累加溢出自动回0
    QHash<quint16, NetworkNodeInfo> m_nodeInfoMap;
};

#endif // NETWORKSEARCH_H
