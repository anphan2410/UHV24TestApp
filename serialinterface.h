#ifndef SERIALINTERFACE_H
#define SERIALINTERFACE_H
#define qqdebug
#include <QMultiMap>
#include <QPair>
#include <QMutex>
#include <QtSerialPort>
#include <QSerialPortInfo>

typedef QPair<quint8, QByteArray> APairOfPrioAndCommand;
typedef QMultiMap<quint8, QByteArray> AListOfPrioAndCommand;

class SerialInterface: public QObject
{
    Q_OBJECT
    bool mIsThreaded = false;
    bool mIsContinuous = false;
    QMutex mutex;
    QString mPortName;    
    quint8 mWriteTimeOut = 100;
    quint16 mReadTimeOut = 300;
    AListOfPrioAndCommand mCommandList;
    AListOfPrioAndCommand::const_iterator mCommandListCItr;
    QByteArray MsgSent;
    QByteArray MsgRead;

    void doThreadedJob();
    void disconnectSerialPort();
public:
    explicit SerialInterface(const QString &PortName);
    ~SerialInterface();    
    const QByteArray &GetMsgSent();
    const QByteArray &GetMsgRead();
public slots:
    void StartThreadJob();
    void ReConfigSerialPort(const QString &PortName);
    void clearCommandList();
    void addToCommandList(const APairOfPrioAndCommand &PriorityAndCommand);
    void addToCommandList(const AListOfPrioAndCommand &CommandList);
    void stopThreadJob();
    void startSendReadLoop();
    void pauseSendReadLoop();
signals:
    void ThreadJobStarted();
    void ThreadJobTerminated();
    void InvalidConnection();
    void IsContinuous();
    void IsPaused();
    void IsConnected();
    void IsDisconnected();
    void BufferCount(int);
    void BufferIsEmpty();
    void MessageSent(QByteArray);
    void MessageRead(QByteArray);
    void ReadTimeOut();
    void WriteTimeOut();
};

#endif // SERIALINTERFACE_H
