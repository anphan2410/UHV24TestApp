#include "serialinterface.h"

SerialInterface::SerialInterface(const QString &PortName):
    mPortName(PortName)
{
}

SerialInterface::~SerialInterface()
{
}

const QByteArray &SerialInterface::GetMsgSent()
{
    return MsgSent;
}

const QByteArray &SerialInterface::GetMsgRead()
{
    return MsgRead;
}

void SerialInterface::StartThreadJob()
{
    if (!mIsThreaded)
    {
#ifdef qqdebug
        qDebug() << "Start doThreadedJob()" << endl;
#endif
        mIsThreaded = true;
        doThreadedJob();
    }
    else
    {
#ifdef qqdebug
        qDebug() << "Repeated Attempt but Thread Is Already Running !!!" << endl;
#endif
        return;
    }
}

void SerialInterface::ReConfigSerialPort(const QString &PortName)
{
    mPortName = PortName;
#ifdef qqdebug
    qDebug() << "Serial Portname Is Changed !!!" << endl;
#endif
}

void SerialInterface::doThreadedJob()
{
    QSerialPort mSerialPort;
#ifdef qqdebug
    qDebug() << "New Serial Port Object Has Been Created !!!" << endl;
#endif
    while (mIsThreaded)
    {
        qApp->processEvents();        
        if (mSerialPort.portName() != mPortName)
        {            
            mutex.lock();
            //Initialize Serial Port
            mSerialPort.close();
            mSerialPort.setReadBufferSize(64);
            mSerialPort.setDataBits(QSerialPort::Data8);
            mSerialPort.setBaudRate(QSerialPort::Baud9600);
            mSerialPort.setStopBits(QSerialPort::OneStop);
            mSerialPort.setParity(QSerialPort::NoParity);
            mSerialPort.setFlowControl(QSerialPort::NoFlowControl);
            mSerialPort.setPortName(mPortName);
            mutex.unlock();
            //Connect Serial Port
            if (mSerialPort.open(QIODevice::ReadWrite))
            {
#ifdef qqdebug
                qDebug() << "Serial Port " << mPortName << " Is Connected !!!" << endl;
#endif
                emit IsConnected();
            }
            else
            {
#ifdef qqdebug
                qDebug() << "Error: Invalid Serial Port Information" << endl;
#endif
                emit InvalidConnection();
                return;
            }            
        }       
        while ((mCommandList.size() != 0) && mIsContinuous)
        {
            //Fetch the most priority message and its corresponding priority value
            mutex.lock();
            MsgSent = mCommandList.constEnd().value();
            mCommandList.remove(mCommandList.constEnd().key(), MsgSent);
            emit BufferCount(mCommandList.size());
            mutex.unlock();
            //Send Message
            mSerialPort.write(MsgSent);
            if (mSerialPort.waitForBytesWritten(mWriteTimeOut)) {
#ifdef qqdebug
                qDebug() << "Success Send Command Message !!!" << endl;
#endif
                emit MessageSent(MsgSent);
                //Give Signal-Slot mechanism a chance to do things
                qApp->processEvents();
                //Read Message
                if(mSerialPort.waitForReadyRead(mReadTimeOut))
                {
                    MsgRead.clear();
                    MsgRead = mSerialPort.readAll();
                    while(mSerialPort.waitForReadyRead(mReadTimeOut))
                    {
                        MsgRead+= mSerialPort.readAll();
                    }
#ifdef qqdebug
                    qDebug() << "Success Read Command Message !!!" << endl;
#endif
                    emit MessageRead(MsgRead);
                }
                else
                {
#ifdef qqdebug
                qDebug() << "Error: Reading Message Timed Out !!!" << endl;
#endif
                emit ReadTimeOut();
                }
            }
            else
            {
#ifdef qqdebug
                qDebug() << "Error: Sending Message Timed Out !!!" << endl;
#endif
            emit WriteTimeOut();
            }
        }
    }
    mSerialPort.close();
}

void SerialInterface::clearCommandList()
{
    mCommandList.clear();
    emit BufferIsEmpty();
}

void SerialInterface::addToCommandList(const APairOfPrioAndCommand &PriorityAndCommand)
{
    mCommandList.insert(PriorityAndCommand.first, PriorityAndCommand.second);
    emit BufferCount(mCommandList.size());
}

void SerialInterface::addToCommandList(const AListOfPrioAndCommand &CommandList)
{
    mCommandList += CommandList;
    emit BufferCount(mCommandList.size());
}

void SerialInterface::disconnectSerialPort()
{
#ifdef qqdebug
    qDebug() << "inside disconnectSerialPort()" << endl;
#endif
    clearCommandList();
    pauseSendReadLoop();
    emit IsDisconnected();
#ifdef qqdebug
    qDebug() << "emit IsDisconnected()" << endl;
#endif
}

void SerialInterface::stopThreadJob()
{
#ifdef qqdebug
    qDebug() << "inside stopThreadJob()" << endl;
#endif
    disconnectSerialPort();
    mIsThreaded = false;
    emit ThreadJobTerminated();
#ifdef qqdebug
    qDebug() << "emit ThreadJobTerminated()" << endl;
#endif
}

void SerialInterface::startSendReadLoop()
{
    mIsContinuous = true;
    emit IsContinuous();
}

void SerialInterface::pauseSendReadLoop()
{
    mIsContinuous = false;
    emit IsPaused();
}




