#include <QTime>
#include "NetworkSearch.h"

#define SET_SEQUENCE_NUMBER(request, seq) ({ \
                                            request[3] = ((quint8*)(&seq))[0]; \
                                            request[4] = ((quint8*)(&seq))[1];})

#define GET_SEQUENCE_NUMBER(request, seq) ({ \
                                            ((quint8*)(&seq))[0] = request[3]; \
                                            ((quint8*)(&seq))[1] = request[4];})
#define GET_NAME_LENGTH(data, length) ({ \
                                            ((quint8*)(&length))[0] = data[7]; \
                                            ((quint8*)(&length))[1] = data[8];})

NetworkSearch::NetworkSearch(QObject *parent)
    : QObject(parent)
{
    m_udpSocket = new QUdpSocket(this);
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkSearch::readData);
    m_udpSocket->open(QIODevice::ReadWrite);

    search4AllNodes();

}

NetworkSearch::~NetworkSearch()
{
    if (m_udpSocket && m_udpSocket->isOpen()) {
        m_udpSocket->close();
        delete m_udpSocket;
    }
}

void NetworkSearch::search4AllNodes()
{
    quint8 request[] = {0x48, 0x41, 0x50, 0x00, 0x00, 0xa5, 0x50, 0x01, 0x00, 0x05}; // 第3，4字节是sequence number小端，需回填
    // 生成一个随机数作为sequence number
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    m_seqNumber = quint32(qrand())%65536u;   //随机生成0到65535的随机数
    SET_SEQUENCE_NUMBER(request, m_seqNumber);
    m_seqNumber++; // 第一次随机生成，之后自增
    m_step = Broadcast;
    m_udpSocket->abort();
    m_udpSocket->writeDatagram(QByteArray((char*)request, sizeof(request)), QHostAddress::Broadcast, 28784); // 端口号28784固定
}

void NetworkSearch::readData()
{
    QByteArrayList data;
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray temp;
        temp.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(temp.data(), temp.size());
//        qDebug() << "0x" + temp.toHex().toUpper();
        data.append(temp);
    }

//    qDebug() << "call readData:" << m_step << data.size();
    if (m_step == Broadcast) {
        getNetworkAddress(data);
    } else {
        getModuleInfo(data);
    }
}

void NetworkSearch::getNetworkAddress(const QByteArrayList &list)
{
    m_step = QueryModule;
    QList<NetworkNodeInfo> infoList;
    quint16 index = 1;
    quint16 seq = m_seqNumber;
    foreach (auto ele, list) {
        NetworkNodeInfo info;
        auto mac = QString(ele.mid(11, 6).toHex().toUpper());
        for (int i = 1; i < 6; i++) {
            mac.insert(i*3-1, ":");
        }
        info.mac = mac;
        auto ip = ele.right(5);
        info.ip = QString("%1.%2.%3.%4")
                .arg(quint8(ip[0]))
                .arg(quint8(ip[1]))
                .arg(quint8(ip[2]))
                .arg(quint8(ip[3]));

        info.index = index;
        index++;
        infoList.append(info);

        m_nodeInfoMap.insert(seq, info); // 通过sequence确定应答对应的是哪个节点
        seq++;
    }
//    printNodeInfo();
    emit signal_nodeListFound(infoList);

    foreach (auto info, infoList) {
        // 发送获取ModuleID和name的请求
        for (int type = 0; type < 2; type++) {
            sendGetModuleInfoRequest(info.ip, type);
        }

        m_seqNumber++;
    }
}

void NetworkSearch::sendGetModuleInfoRequest(const QString &ip, int type)
{
    quint8 request[] = {0x48, 0x41, 0x50, 0x00, 0x00, 0xF9, 0x18, 0x04, 0x00, 0x0B, 0x00, 0x20, 0x00};// 第3，4字节是sequence number小端，需回填
    if (type == 0) {
        request[sizeof(request)-2] = 0x20; // for module ID
    } else {
        request[sizeof(request)-2] = 0x16; // for module name
        request[5] = 0xCA; // for crc
        request[6] = 0xB7; // for crc
    }
    auto host = QHostAddress(ip);
    SET_SEQUENCE_NUMBER(request, m_seqNumber);
    m_udpSocket->writeDatagram(QByteArray((char*)request, sizeof(request)), host, 28784);
}

void NetworkSearch::getModuleInfo(const QByteArrayList &list)
{
    foreach (auto info, list) {
        // 拿到应答带的sequence number
        quint16 seq = 0;
        GET_SEQUENCE_NUMBER(info, seq);
//        qDebug() << seq;
        if (!m_nodeInfoMap.contains(seq)) continue; // 收到sequence不合法的包，丢弃

//        printNodeInfo();
        if (info.size() == 15) {
            quint32 id = 0;
            auto idBytes = info.right(4);
            for (int i = 0; i < 4; i++) {
                ((quint8*)(&id))[i] = quint8(idBytes[i]);
            }
            m_nodeInfoMap[seq].moduleID = id;
        } else {
            quint16 length = 0;
            GET_NAME_LENGTH(info, length);
            m_nodeInfoMap[seq].name = info.right(length - 2);
        }

        emit signal_nodeInfoCatched(m_nodeInfoMap[seq]);
    }


}

void NetworkSearch::printNodeInfo()
{
    QHashIterator<quint16, NetworkNodeInfo> iter(m_nodeInfoMap);
    while (iter.hasNext()) {
        iter.next();
        qDebug() << "printNodeInfo, index" << iter.value().index;
    }
}








