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
        mIsThreaded = true;
        doThreadedJob();
    }
    else
    {
        qDebug() << "Repeated Attempt but Thread Is Already Running !!!";
        return;
    }
}

void SerialInterface::ReConfigSerialPort(const QString &PortName)
{
    mPortName = PortName;
}

void SerialInterface::doThreadedJob()
{
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
                emit IsConnected();
            else
            {
                qDebug() << "Error: Invalid Serial Port Information" << endl;
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
                    emit MessageRead(MsgRead);
                }
                else
                    emit ReadTimeOut();
            }
            else
                emit WriteTimeOut();
        }
    }
}

void SerialInterface::clearCommandList()
{
    mCommandList.clear();
    emit BufferIsEmpty();
}

void SerialInterface::addToCommandList(const QPair<quint8, QByteArray> &PriorityAndCommand)
{
    mCommandList.insert(PriorityAndCommand.first, PriorityAndCommand.second);
    emit BufferCount(mCommandList.size());
}

void SerialInterface::addToCommandList(const QMultiMap<quint8, QByteArray> &CommandList)
{
    mCommandList += CommandList;
    emit BufferCount(mCommandList.size());
}

void SerialInterface::disconnectSerialPort()
{
    clearCommandList();
    pauseSendReadLoop();
    mSerialPort.close();
    emit IsDisconnected();
}

void SerialInterface::stopThreadJob()
{
    disconnectSerialPort();
    mIsThreaded = false;
    emit ThreadJobTerminated();
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




