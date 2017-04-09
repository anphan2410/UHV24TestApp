#include"binaryprotocol.h"

//Miscellany: Support Codes*********************************************/

QByteArray &operator <<(QByteArray &QBArr, const quint8 Dat)
{
    QBArr.append(Dat);
    return QBArr;
}

QByteArray &operator <<(QByteArray &QBArr, const quint16 Dat)
{
    return QBArr<<quint8(Dat>>8)<<quint8(Dat);
}

QByteArray &operator <<(QByteArray &QBArr, const quint32 Dat)
{
    return QBArr<<quint16(Dat>>16)<<quint16(Dat);
}

QByteArray &operator <<(QByteArray &QBArr, const quint64 Dat)
{
    return QBArr<<quint32(Dat>>32)<<quint32(Dat);
}

QByteArray &operator <<(QByteArray &QBArr, const QByteArray &Dat)
{
    QBArr.append(Dat);
    return QBArr;
}

quint8 XORofAllBytesInQByteArr(const QByteArray &QBArr)
{
    if (~QBArr.isNull() && ~QBArr.isEmpty())
    {
        QByteArray::const_iterator ConstItr = QBArr.constBegin();
        quint8 IntTmp = *(ConstItr++);
        for (;ConstItr!=QBArr.cend(); ConstItr++)
        {
            IntTmp ^=*ConstItr;
        }
        return IntTmp;
    }
    else
        return 0;
}

///
/// \brief IntStr2QBArr0Pad
/// \param Num
/// \param SizeInByte
/// \return QByteArray contains a hex number
///         representing an integer number encoded by Ascii code
///
const QByteArray IntStr2QBArr0Pad(const quint8 Num, const quint8 SizeInByte)
{
    QString QStrTmp = QString::number(Num);
    return QStrTmp.prepend(QString("").fill('0',SizeInByte-QStrTmp.size())).toLocal8Bit();
}



//___________________________________________________________________________________________

///
/// \brief BinaryProtocol::BinaryProtocol
/// \abstract This non-parameter constructor is used to create a new BinaryProtocol object
/// An unique mBPNo will be automatically assigned
/// After having created, this object will be added into BPObjSet to be tracked later
///
BinaryProtocol::BinaryProtocol()
    :mBPNo([]()
        {
            quint8 BPNum = 0;
            for (BPNum = 0; BPNum <= BinaryProtocol::BPObjSet.size(); BPNum++)
                if (!BinaryProtocol::BPObjSet.contains(BPNum))
                    break;
            return BPNum;
        }())
    ,mHdrCmd(quint8(mBPNo+_BaseHdrCmd))
    ,mHdrRsp(quint8(mBPNo+_BaseHdrRsp))
    ,mHdr(0)
    ,mDataLen(_StdDatLen)
    ,mCmd(0)
    ,mCh(0)
    ,mData(* new TypDat())
    ,mChkSum(0)
    ,mMsg(*(new QByteArray()))
{
    BinaryProtocol::BPObjSet.insert(mBPNo,this);
}

///
/// \brief BinaryProtocol::BinaryProtocol
/// \param BPNum
///
BinaryProtocol::BinaryProtocol(const quint8 BPNum)
    :mBPNo(BPNum)
    ,mHdrCmd(quint8(BPNum+_BaseHdrCmd))
    ,mHdrRsp(quint8(BPNum+_BaseHdrRsp))
    ,mHdr(0)
    ,mDataLen(_StdDatLen)
    ,mCmd(0)
    ,mCh()
    ,mData(* new TypDat())
    ,mChkSum(0)
    ,mMsg(*(new QByteArray()))
{
    //Check If There's Already A BP With That BPNum
    if (BinaryProtocol::BPObjSet.contains(BPNum))   //If There's An Object With That BPNum, Then Throw Exception
        throw std::invalid_argument(std::string("Pump #")+std::to_string(BPNum)+std::string(" Has Already Existed !!!"));
    else
        BinaryProtocol::BPObjSet.insert(BPNum,this);
}

///
/// \brief BinaryProtocol::~BinaryProtocol
///
BinaryProtocol::~BinaryProtocol()
{
    BinaryProtocol::BPObjSet.remove(mBPNo);
}

///
/// \brief BinaryProtocol::BP
/// \param BPNum
/// \return A Reference To The Existed Object
///         , here is an instance of Binary Protocol Class
///         , with unique number
///
BinaryProtocol &BinaryProtocol::BP(const quint8 BPNum)
{
    if (BinaryProtocol::BPObjSet.contains(BPNum))
        return *BinaryProtocol::BPObjSet.value(BPNum);
    else
        throw std::invalid_argument(std::string("Pump #")+std::to_string(BPNum)+std::string(" Does Not Exist !!!"));
}

///
/// \brief BinaryProtocol::BP
/// \param QBArr
/// \return A Reference To The Existed Object
///         , here is an instance of Binary Protocol Class
///         , corresponding with all protocol parts parsed
///         , using received data from its respective pump station
///
BinaryProtocol &BinaryProtocol::BP(const QByteArray &QBArr)
{
    TypHdr IntTmp1 = 0;
    TypHdr IntTmp2 = QBArr.left(_SzHdr).toHex().toInt(NULL,16);
    if (IntTmp2 < _BaseHdrCmd)
        IntTmp1 = IntTmp2 - _BaseHdrRsp;
    else
        IntTmp1 = IntTmp2 - _BaseHdrCmd;
    BinaryProtocol &BPTmp = BinaryProtocol::BP(IntTmp1);
    IntTmp1 = 0;
    BPTmp.mHdr = IntTmp2;
    IntTmp1 += _SzHdr;
    BPTmp.mDataLen = QBArr.mid(IntTmp1,_SzDatLen).toHex().toInt(NULL,16);
    IntTmp1 += _SzDatLen;
    BPTmp.mCmd = QBArr.mid(IntTmp1,_SzCmd).toHex().toInt(NULL,16);
    IntTmp1 += _SzCmd;
    BPTmp.mCh = QBArr.mid(IntTmp1,_SzCh).toHex().toInt(NULL,16);
    IntTmp1 += _SzCh;
    IntTmp2 = QString::fromLocal8Bit(* new QByteArray() << BPTmp.mDataLen).toInt()-_SzCmd-_SzCh;
    BPTmp.mData = QBArr.mid(IntTmp1,IntTmp2);
    IntTmp1 += IntTmp2;
    BPTmp.mChkSum = QBArr.mid(IntTmp1,_SzChkSum).toHex().toInt(NULL,16);
    //Vadility Of Response Message Should Be Added Here
    BPTmp.mMsg.clear();
    BPTmp.mMsg<<BPTmp.mHdr<<BPTmp.mDataLen<<BPTmp.mCmd<<BPTmp.mCh<<BPTmp.mData<<BPTmp.mChkSum;
    return BPTmp;
}

///
/// \brief BinaryProtocol::IsBP
/// \param BPNum
/// \return true if there's already an object
///             with given number, otherwise false.
///
bool BinaryProtocol::IsBP(const quint8 BPNum)
{
    return BinaryProtocol::BPObjSet.contains(BPNum);
}

///
/// \brief BinaryProtocol::BPCount
/// \return Quantity of Valid Instances
///         , here is the number of pump stations
///
quint8 BinaryProtocol::BPCount()
{
    return BinaryProtocol::BPObjSet.size();
}

///
/// \brief BinaryProtocol::DeleteBP
/// \param BPNum
/// \return true if there is an object with given number,
///                 and this object is deleted,
///         false otherwise.
///
bool BinaryProtocol::DeleteBP(const quint8 BPNum)
{
    if (BinaryProtocol::BPObjSet.contains(BPNum))
    {
        BinaryProtocol::BPObjSet.value(BPNum)->~BinaryProtocol();
        return true;
    }
    else
        return false;
}

quint8 BinaryProtocol::GetBPNo() const
{
    return mBPNo;
}

void BinaryProtocol::SetBPNo(const quint8 BPNum)
{
    //Check If There's Already A BP With That BPNum
    if (BinaryProtocol::BPObjSet.contains(BPNum))   //If There's An Object With That BPNum, Then Throw Exception
        throw std::invalid_argument(std::string("Pump #")+std::to_string(BPNum)+std::string(" Has Already Existed !!!"));
    else
    {
        BinaryProtocol::BPObjSet.remove(mBPNo);
        mBPNo = BPNum;
        mHdrCmd = quint8(BPNum+_BaseHdrCmd);
        mHdrRsp = quint8(BPNum+_BaseHdrRsp);
        mHdr = 0;
        mDataLen = _StdDatLen;
        mCmd = 0;
        mCh = 0;
        mData.clear();
        mChkSum = 0;
        mMsg.clear();
        BinaryProtocol::BPObjSet.insert(BPNum,this);
    }
}

BinaryProtocol::TypHdr BinaryProtocol::GetHdrCmd() const
{
    return mHdrCmd;
}

BinaryProtocol::TypHdr BinaryProtocol::GetHdrRsp() const
{
    return mHdrRsp;
}

BinaryProtocol::TypHdr BinaryProtocol::GetHdr() const
{
    return mHdr;
}

BinaryProtocol::TypCh BinaryProtocol::GetCh() const
{
    return mCh;
}

BinaryProtocol::TypCmd BinaryProtocol::GetCmd() const
{
    return mCmd;
}

const BinaryProtocol::TypDat BinaryProtocol::GetData() const
{
    return mData;
}

BinaryProtocol::TypChkSum BinaryProtocol::GetChkSum() const
{
    return mChkSum;
}

const QByteArray BinaryProtocol::GetMsg() const
{
    return mMsg;
}

///
/// \brief BinaryProtocol::GenMsg
/// \return New Message Generated From
///             Current Values Of Protocol Parts
///
const QByteArray BinaryProtocol::GenMsg()
{
    mMsg.clear();
    mHdr=mHdrCmd;
    quint8 int8tmp = _SzCmd + _SzCh + mData.size();
    if (int8tmp != 4)
        mDataLen = IntStr2QBArr0Pad(int8tmp, _SzDatLen).toHex().toInt(NULL,16);
    mMsg<<mHdr<<mDataLen<<mCmd<<mCh<<mData;
    mChkSum=XORofAllBytesInQByteArr(mMsg)&_MskChkSum;
    mMsg<<mChkSum;
    return mMsg;
}

const QString BinaryProtocol::GetMessageDirection() const
{
    if (mHdr<_BaseHdrCmd)
        return QString("From");
    else
        return QString("To");
}

quint8 BinaryProtocol::GetHeader() const
{
    return mBPNo;
}

quint8 BinaryProtocol::GetDataLength() const
{
    QByteArray QBTmp = "";
    QBTmp << mDataLen;
    return QBTmp.toInt();
}

const QString BinaryProtocol::GetCommand() const
{
    return BinaryProtocol::CmdCode2CmdMean.value(mCmd);
}

quint8 BinaryProtocol::GetChannel() const
{
    return (mCh-_BaseCh);
}

const QString BinaryProtocol::GetDataTranslation() const
{
    QString StrTmp = "";
    switch (mCmd) {
    case 0x5A30:
        switch (mData.toHex().toInt(NULL,16)) {
        case 0x30:
            StrTmp = "Local";
            break;
        case 0x31:
            StrTmp = "Remote I/O";
            break;
        case 0x32:
            StrTmp = "Serial";
            break;
        }
        break;
    case 0x4130:
        switch (mData.toHex().toInt(NULL,16)) {
        case 0x30:
            StrTmp = "Off";
            break;
        case 0x31:
            StrTmp = "On With RunMode:Start/Step";
            break;
        case 0x32:
            StrTmp = "On With RunMode:Start/Fixed";
            break;
        case 0x33:
            StrTmp = "On With RunMode:Protect/Step";
            break;
        case 0x34:
            StrTmp = "On With RunMode:Protect/Fixed";
            break;
        case 0x2D33:
            StrTmp = "Off Due To Interlock Cable/Panel";
            break;
        case 0x2D34:
            StrTmp = "Off Due To Remote I/O Interlock";
            break;
        case 0x2D36:
            StrTmp = "Off Due To HV Protect";
            break;
        case 0x2D37:
            StrTmp = "Off Due To HV Short Circuit/ Remote I/O Failure";
            break;
        case 0x2D38:
            StrTmp = "Off Due To HV Overtemperature";
            break;
        }
        break;
    case 0x4430:
        switch (mData.toHex().toInt(NULL,16)) {
        case 0x30:
            StrTmp = "Torr";
            break;
        case 0x31:
            StrTmp = "mBar";
            break;
        case 0x32:
            StrTmp = "Pascal";
            break;
        }
        break;
    case 0x5D30:
        switch (mData.toHex().toInt(NULL,16)) {
        case 0x01:
        case 0x10:
            StrTmp = "Reserved (always 0)";
            break;
        case 0x02:
        case 0x20:
            StrTmp = "Front Panel Interlock (equal to bit 20h)";
            break;
        case 0x04:
            StrTmp = "HV1 Remote I/O Interlock";
            break;
        case 0x08:
            StrTmp = "HV1 Cable interlock";
            break;
        case 0x40:
            StrTmp = "HV2 Remote I/O Interlock";
            break;
        case 0x80:
            StrTmp = "HV2 Cable interlock";
            break;
        }
        break;
    case 0x4630:
        StrTmp = QString::number(mData.toHex().toInt(NULL,16));
//*Uncomment Following Lines If Need**************************/
//        switch (mCh) {
//        case 0x31:
//        case 0x32:
//            switch (mData.toHex().toInt(NULL,16)) {
//            case 0x30:
//                StrTmp = "Spare";
//                break;
//            case 0x31:
//                StrTmp = "500 SC/Tr";
//                break;
//            case 0x32:
//                StrTmp = "300 SC/Tr";
//                break;
//            case 0x33:
//                StrTmp = "150 SC/Tr";
//                break;
//            case 0x34:
//                StrTmp = "75-55-40SC/T";
//                break;
//            case 0x35:
//                StrTmp = "20 SC/Tr";
//                break;
//            case 0x36:
//                StrTmp = "500 Diode/ND";
//                break;
//            case 0x37:
//                StrTmp = "300 Diode/ND";
//                break;
//            case 0x38:
//                StrTmp = "150 Diode/ND";
//                break;
//            case 0x39:
//                StrTmp = "75-55-40 D/ND";
//                break;
//            case 0x3A:
//                StrTmp = "20 -25 Diode/ND";
//                break;
//            }
//            break;
//        case 0x33:
//        case 0x34:
//            switch (mData.toHex().toInt(NULL,16)) {
//            case 0x30:
//                StrTmp = "Convectorr";
//                break;
//            case 0x31:
//                StrTmp = "Mini-B/A";
//                break;
//            case 0x32:
//                StrTmp = "Cold Cathode";
//                break;
//            }
//            break;
//        case 0x35:
//            switch (mData.toHex().toInt(NULL,16)) {
//            case 0x30:
//                StrTmp = "RS232/422";
//                break;
//            case 0x31:
//                StrTmp = "RS485";
//                break;
//            }
//            break;
//        }
        break;
    case 0x5430:
    case 0x5530:
        StrTmp = QString(mData);
        StrTmp =  QString::number((StrTmp.section('E',0,0,QString::SectionSkipEmpty).toDouble())
                                  *qPow(10,StrTmp.section('E',1,1,QString::SectionSkipEmpty).toDouble()));
        break;
    default:
        StrTmp = QString(mData);
        break;
    }
    if (mData.toHex().toInt(NULL,16) == 0x3F)
        StrTmp = "To Be Collected !";
    return StrTmp;
}

const QString BinaryProtocol::GetMessageTranslation() const
{
    QString StrTmp = "";
    if (GetChannel() == 0)
        StrTmp = "No Channel";
    else
        StrTmp = QString("Channel #").append(QString::number(GetChannel()));
    StrTmp = GetMessageDirection() + " - Pump #" + QString::number(GetHeader())
        + " - " + StrTmp + " - " + GetCommand() + " - Data " + GetDataTranslation();
    return StrTmp;
}

///
/// \brief BinaryProtocol::Cmd
/// \param Code
/// \return A Reference To Itself
///             After Update Its Protocol Command Part With Given Command Code
///
BinaryProtocol &BinaryProtocol::Cmd(const quint16 CmdCode)
{
    mCmd = CmdCode;
    return *this;
}

BinaryProtocol &BinaryProtocol::ModeLRS()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

BinaryProtocol &BinaryProtocol::HVSwitch()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

BinaryProtocol &BinaryProtocol::ProtectSwitch()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

BinaryProtocol &BinaryProtocol::UnitPres()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

BinaryProtocol &BinaryProtocol::ReaduCFWVer()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::ReadDSPFWVer()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::DevID()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

BinaryProtocol &BinaryProtocol::ReadDevType()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::ReadV()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::ReadI()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::ReadP()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::ReadErr()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::WriteSerialReset()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

BinaryProtocol &BinaryProtocol::ReadInterlock()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__)).Read();
}

BinaryProtocol &BinaryProtocol::SerialConfigMode()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

BinaryProtocol &BinaryProtocol::SerialProperty()
{
    return this->Cmd(CmdMean2CmdCode.value(__func__));
}

///
/// \brief BinaryProtocol::Ch
/// \param Channel
/// \return A Reference To Itself
///             After Update Its Protocol Channel Part With Given Channel
///
BinaryProtocol &BinaryProtocol::Ch(const quint8 Channel)
{
    mCh=Channel;
    return *this;
}

BinaryProtocol &BinaryProtocol::Ch1()
{
    return this->Ch(1+_BaseCh);
}

BinaryProtocol &BinaryProtocol::Ch2()
{
    return this->Ch(2+_BaseCh);
}

BinaryProtocol &BinaryProtocol::ChSerial()
{
    return this->Ch(5+_BaseCh);
}

BinaryProtocol &BinaryProtocol::noCh()
{
    return this->Ch(_BaseCh);
}

BinaryProtocol &BinaryProtocol::Read()
{
    return this->Data(_Read);
}

BinaryProtocol &BinaryProtocol::On()
{
    return this->Data(_On);
}

BinaryProtocol &BinaryProtocol::Off()
{
    return this->Data(_Off);
}


///
/// \brief BinaryProtocol::Data
/// \param Data
/// \return A Reference To Itself
///             After Update Its Protocol Data Part With Given Data
///
BinaryProtocol &BinaryProtocol::Data(const quint8 Data)
{
    mData.clear();
    mData<<Data;
    return *this;
}

BinaryProtocol::TypDatLen BinaryProtocol::GetDataLen() const
{
    return mDataLen;
}

///
/// \brief BinaryProtocol::BPObjSet
/// \abstract A Reference To A Class Variable Tracking All Instances
///
QHash<const quint8, BinaryProtocol *> &BinaryProtocol::BPObjSet = * new QHash<const quint8, BinaryProtocol *>();

///
/// \brief UHV2 Command Set
/// \abstract Fast Search For Code Indexed By Mean
///
const QHash<QString , BinaryProtocol::TypCmd> &BinaryProtocol::CmdMean2CmdCode = * new QHash<QString, BinaryProtocol::TypCmd>
({
     {"ModeLRS", 0x5A30},
     {"HVSwitch", 0x4130},
     {"UnitPres", 0x4430},
     {"ReaduCFWVer", 0x4530},
     {"ReadDSPFWVer", 0x4531},
     {"DevID", 0x4630},
     {"ReadDevType", 0x4631},
     {"ReadV", 0x5330},
     {"ReadI", 0x5430},
     {"ReadP", 0x5530},
     {"ReadErr", 0x7A30},
     {"WriteSerialReset", 0x5B30},
     {"ReadInterlock", 0x5D30},
     {"SerialConfigMode", 0x7861},
     {"SerialProperty", 0x7862},
     {"ProtectSwitch", 0x4330}
});

///
/// \brief UHV2 Command Set With Priority
/// \abstract Fast Search For Priority Indexed By Command Code
/// \note Please Consider Priority For Higher Interrupts
///
const QHash<BinaryProtocol::TypCmd, quint8> &BinaryProtocol::CmdCode2Priority = * new QHash<BinaryProtocol::TypCmd, quint8>
({
     {0x5A30, 15},
     {0x4130, 20},
     {0x4430, 1},
     {0x4530, 7},
     {0x4531, 8},
     {0x4630, 11},
     {0x4631, 11},
     {0x5330, 10},
     {0x5430, 10},
     {0x5530, 10},
     {0x7A30, 4},
     {0x5B30, 22},
     {0x5D30, 3},
     {0x7861, 8},
     {0x7862, 16},
     {0x4330, 19}
});

///
/// \brief UHV2 Command Set
/// \abstract Fast Search For Mean Indexed By Code
///
const QHash<BinaryProtocol::TypCmd, QString> &BinaryProtocol::CmdCode2CmdMean = BinaryProtocol::SwapKeyValQHash(BinaryProtocol::CmdMean2CmdCode);

///
/// \brief BinaryProtocol::SwapKeyValQHash
/// \return A Reference To A Same QHash with OldKey -> Value, OldValue -> Key,
///             This Is Used For Initialization of Class Static Set
///
template<typename TN1, typename TN2>
const QHash<TN1, TN2> &BinaryProtocol::SwapKeyValQHash(const QHash<TN2, TN1> &AQHashKeyValSet)
{
    QHash<TN1, TN2> &tmp = * new QHash<TN1, TN2>();
    TN2 tmp2 = "";
    auto KeyItr = AQHashKeyValSet.keyBegin();
    for (; KeyItr!=AQHashKeyValSet.keyEnd(); KeyItr++)
    {
        tmp2 = *KeyItr;
        tmp.insert(AQHashKeyValSet.value(tmp2), tmp2);
    }
    return tmp;
}
