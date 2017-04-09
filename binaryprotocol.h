#ifndef BINARYPROTOCOL_H
#define BINARYPROTOCOL_H

#include<QDebug>
#include<QtMath>

///
/// \brief The BinaryProtocol class
///
class BinaryProtocol
{
    /*Define Types Used By Protocol Parameters*/
    typedef quint8 TypHdr;  //Type of Header
    typedef quint16 TypDatLen;   //Type of Data Length
    typedef quint16 TypCmd;  //Type of Command
    typedef quint8 TypCh;   //Type of Channel
    typedef QByteArray TypDat;  //Type of Data
    typedef quint8 TypChkSum;   //Type of Check Sum
    //__________________________________________________

    /*Define Class Constants***************************************************************************/
    //Fixed Size in Byte of Protocol Parameters (if provided)
    static constexpr const quint8 _SzHdr = sizeof(TypHdr);
    static constexpr const quint8 _SzDatLen = sizeof(TypDatLen);
    static constexpr const quint8 _SzCmd = sizeof(TypCmd);
    static constexpr const quint8 _SzCh = sizeof(TypCh);
    //Data Is A Dynamic Parameter => Size of Data Is Not A Constant
    static constexpr const quint8 _SzChkSum = sizeof(TypChkSum);

    //Protocol Parameter Offset (if provided)
    static constexpr const TypHdr _BaseHdrCmd = 0x81;
    static constexpr const TypHdr _BaseHdrRsp = 0x01;
    static constexpr const TypCh _BaseCh = 0x30;

    //Checksum Mask
    static constexpr const TypChkSum _MskChkSum = 0x7F;


    //Supported Function For Purpose of Initialization Data Set Below
    template <typename TN1, typename TN2>
    static const QHash<TN1, TN2> &SwapKeyValQHash(const QHash<TN2, TN1> &AQHashKeyValSet);

    //Command Set Indexed In Two Fields: Command Meaning & Command Code, Optimizing For Fast Performance
    //All Details About Commands Defined In Source File
    static const QHash<QString, TypCmd> &CmdMean2CmdCode;
    static const QHash<TypCmd, QString> &CmdCode2CmdMean;

    //Prioritize Command Set Indexed By Command Code
    //The higher is the priority value, the more priority is the command
    static const QHash<TypCmd, quint8> &CmdCode2Priority;

    //Miscellany (Just A Bunch Of Other Constants)
    static constexpr const TypDatLen _StdDatLen = 0x3034;   //Standard Data Length, Here "04" => 0x3034
    static constexpr const quint8 _Read = 0x3F;
    static constexpr const quint8 _On = 0x31;
    static constexpr const quint8 _Off = 0x30;

    //__________________________________________________________________________________________________

    /*Dynamic Class Property Tracking of All Class Instance************/
    static QHash<const quint8, BinaryProtocol *> &BPObjSet;
    //__________________________________________________________________

    /*Declare Object Property******************************************************************************/
    quint8 mBPNo;     //Unique Object Number Representing For Binary Protocol Of A Unique Pump
    TypHdr mHdrCmd;   //Unique Object => Unique Command Header
    TypHdr mHdrRsp;   //Unique Object => Unique Command Response

    //Protocol Parts : Header + DataPacketLength + Command + Channel + Data + CheckSum
    //DataPacketLength = ASCII Encode (SizeInByte(Command) + SizeInByte(Channel) + SizeInByte(Data))
    TypHdr mHdr;
    TypDatLen mDataLen;
    TypCmd mCmd;
    TypCh  mCh;
    TypDat &mData;
    TypChkSum mChkSum;
    //Protocol
    QByteArray &mMsg;
    //______________________________________________________________________________________________________

public:
    BinaryProtocol();
    BinaryProtocol(const quint8 BPNum);
    ~BinaryProtocol();
    static BinaryProtocol &BP(const quint8 BPNum);
    static BinaryProtocol &BP(const QByteArray &QBArr);
    static bool IsBP(const quint8 BPNum);
    static quint8 BPCount();
    static bool DeleteBP(const quint8 BPNum);

    //Class Getter/Property Constants Out In Primitive Data Types
    //For A Sake Of Universal Communication
    static const int ChannelBase = _BaseCh;

    quint8 GetBPNo() const;
    void SetBPNo(const quint8 BPNum);

    //Get Protocol Parts
    TypHdr GetHdrCmd() const;
    TypHdr GetHdrRsp() const;
    TypHdr GetHdr() const;
    TypCh GetCh() const;
    TypDatLen GetDataLen() const;
    TypCmd GetCmd() const;
    const TypDat GetData() const;
    TypChkSum GetChkSum() const;
    const QByteArray GetMsg() const;

    const QByteArray GenMsg();

    //Get Protocol Part Meanings
    const QString GetMessageDirection() const;
    quint8 GetHeader() const;
    quint8 GetDataLength() const;
    const QString GetCommand() const;
    quint8 GetChannel() const;
    const QString GetDataTranslation() const;
    const QString GetMessageTranslation() const;

    BinaryProtocol &Cmd(const quint16 CmdCode);
    BinaryProtocol &ModeLRS();
    BinaryProtocol &HVSwitch();
    BinaryProtocol &ProtectSwitch();
    BinaryProtocol &UnitPres();
    BinaryProtocol &ReaduCFWVer();
    BinaryProtocol &ReadDSPFWVer();
    BinaryProtocol &DevID();
    BinaryProtocol &ReadDevType();
    BinaryProtocol &ReadV();
    BinaryProtocol &ReadI();
    BinaryProtocol &ReadP();
    BinaryProtocol &ReadErr();
    BinaryProtocol &WriteSerialReset();
    BinaryProtocol &ReadInterlock();
    BinaryProtocol &SerialConfigMode();
    BinaryProtocol &SerialProperty();

    BinaryProtocol &Ch(const quint8 Channel);
    BinaryProtocol &Ch1();
    BinaryProtocol &Ch2();
    BinaryProtocol &ChSerial();
    BinaryProtocol &noCh();

    BinaryProtocol &Data(const quint8 Data);
    BinaryProtocol &Read();
    BinaryProtocol &On();
    BinaryProtocol &OnProtectVStep();
    BinaryProtocol &OnProtectVFixed();
    BinaryProtocol &Off();
};

#endif // BINARYPROTOCOL_H
