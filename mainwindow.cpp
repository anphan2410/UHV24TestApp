#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    NowUHV2(new BinaryProtocol())
{
    ui->setupUi(this);
    listSerialPortsToComboBox();
    QThread *UHV2workerThread = new QThread;
    SerialInterface *UHV2workerSI = new SerialInterface("");
    UHV2workerSI->moveToThread(UHV2workerThread);
    qRegisterMetaType<APairOfPrioAndCommand>("APairOfPrioAndCommand");
    qRegisterMetaType<AListOfPrioAndCommand>("AListOfPrioAndCommand");

    connect(this, SIGNAL(sigReConfiguretheSerialInterface(QString)),UHV2workerSI, SLOT(ReConfigSerialPort(QString)));
    connect(this, SIGNAL(sigStartSerialInterfaceThreadJob()),UHV2workerSI, SLOT(StartThreadJob()));
    connect(this, SIGNAL(sigSerialInterfaceClosureRequest()),UHV2workerSI, SLOT(stopThreadJob()));

    connect(this, SIGNAL(sigSerialInterfaceBufferClear()), UHV2workerSI, SLOT(clearCommandList()));

    connect(this, SIGNAL(sigSerialInterfaceStartSendReadLoop()), UHV2workerSI, SLOT(startSendReadLoop()));
    connect(this, SIGNAL(sigSerialInterfacePauseSendReadLoop()), UHV2workerSI, SLOT(pauseSendReadLoop()));
    connect(this, SIGNAL(sigSerialInterfaceAddAPairToCommandList(APairOfPrioAndCommand))
            , UHV2workerSI, SLOT(addToCommandList(APairOfPrioAndCommand)));
    connect(this, SIGNAL(sigSerialInterfaceAddAListToCommandList(AListOfPrioAndCommand))
            , UHV2workerSI, SLOT(addToCommandList(AListOfPrioAndCommand)));

    connect(UHV2workerSI, SIGNAL(IsConnected()), this, SLOT(serialInterfaceIsConnected()));
    //connect(UHV2workerSI, SIGNAL(IsDisconnected()), this, SLOT(serialInterfaceIsDisconnected()));
    connect(UHV2workerSI, SIGNAL(InvalidConnection()), this, SLOT(serialInterfaceIsDisconnected()));

    connect(UHV2workerSI, SIGNAL(BufferCount(int)), this, SLOT(displayBufferCount(int)));
    connect(UHV2workerSI, SIGNAL(MessageSent(QByteArray)), this, SLOT(displayMessageSent(QByteArray)));
    connect(UHV2workerSI, SIGNAL(MessageRead(QByteArray)), this, SLOT(displayMessageRead(QByteArray)));

    /**
              <Add More Here If Needed>
    **/

    UHV2workerThread->start();
    quint8 tmpQInt8 = ui->spinBox_9->value();
    if (NowUHV2->GetBPNo() != tmpQInt8)
    {
        if (!BinaryProtocol::IsBP(tmpQInt8))
            NowUHV2->SetBPNo(tmpQInt8);
        else
            NowUHV2 = &BinaryProtocol::BP(tmpQInt8);
    }
    NowUHV2->Ch(quint8(ui->spinBox_10->value()+BinaryProtocol::ChannelBase));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::serialInterfaceIsRequested()
{
#ifdef qqdebug
    qDebug() << "inside void serialInterfaceIsRequested()" << endl;
#endif
    ui->pushButton_SerialConnect->setText("Connecting");
    ui->connectionStatus->setText("- - -");
    ui->pushButton_SerialConnect->setDisabled(true);    
    emit sigReConfiguretheSerialInterface(ui->comboBoxSerial->currentText());
    emit sigStartSerialInterfaceThreadJob();
#ifdef qqdebug
    qDebug() << "emit sigReConfiguretheSerialInterface & sigStartSerialInterfaceThreadJob" << endl;
#endif
}

void MainWindow::HiddenOutUi(bool IsDisabled)
{
    ui->frameInterrupt->setDisabled(IsDisabled);
    ui->frame_Monitor->setDisabled(IsDisabled);
    ui->label_21->setDisabled(IsDisabled);
    ui->spinBox_9->setDisabled(IsDisabled);
    ui->spinBox_10->setDisabled(IsDisabled);
}

void MainWindow::AutoSendnRead()
{
    ui->pushButton_7->setText("Pause Test");
    emit sigSerialInterfaceStartSendReadLoop();
}

void MainWindow::PauseAutoSendnRead()
{
    ui->pushButton_7->setText("Run Test");
    emit sigSerialInterfacePauseSendReadLoop();
}

void MainWindow::serialInterfaceIsConnected()
{
#ifdef qqdebug
    qDebug() << "inside void serialInterfaceIsConnected()" << endl;
#endif
    ui->pushButton_SerialConnect->setText("Disconnect");
    ui->connectionStatus->setText("Connected");
    ui->pushButton_SerialConnect->setEnabled(true);
    HiddenOutUi(false);
}


void MainWindow::serialInterfaceIsDisconnected()
{
#ifdef qqdebug
    qDebug() << "inside void serialInterfaceIsDisconnected()" << endl;
#endif
    ui->connectionStatus->setText("- X -");
    ui->pushButton_SerialConnect->setText("Connect");
    ui->pushButton_SerialConnect->setEnabled(true);
    HiddenOutUi(true);
    emit sigSerialInterfaceClosureRequest();
#ifdef qqdebug
    qDebug() << "emit sigSerialInterfaceClosureRequest()" << endl;
#endif
}

void MainWindow::serialInterfaceInvalid()
{
    QMessageBox::critical(this, "Error", "Serial Interface is not working !!!");
    serialInterfaceIsDisconnected();
}

void MainWindow::displayMessageSent(const QByteArray &MessageSent)
{
    ToggleDisplayHighlightSendRead(1);
    NowUHV2 = &BinaryProtocol::BP(MessageSent);
    ui->label_MsgSnt->setText(QString(MessageSent.toHex()));
    ui->label_MessageSent->setText(NowUHV2->GetMessageTranslation());
}

void MainWindow::displayMessageRead(const QByteArray &MessageRead)
{
    ToggleDisplayHighlightSendRead(2);
    NowUHV2 = &BinaryProtocol::BP(MessageRead);
    ui->label_MsgRd->setText(QString(MessageRead.toHex()));
    ui->label_MessageRead->setText(NowUHV2->GetMessageTranslation());
}

void MainWindow::displayBufferCount(int Count)
{
    ui->label_20->setText(QString::number(Count));
}

void MainWindow::listSerialPortsToComboBox()
{
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->comboBoxSerial->addItem(info.portName());
}

void MainWindow::on_pushButton_SerialConnect_clicked()
{
    if (ui->pushButton_SerialConnect->text() == "Connect")
    {
#ifdef qqdebug
        qDebug() << "Button Connect Is Clicked !" << endl;
#endif
        serialInterfaceIsRequested();
    }
    else if (ui->pushButton_SerialConnect->text() == "Disconnect")
    {
#ifdef qqdebug
        qDebug() << "Button Disconnect Is Clicked !" << endl;
#endif
        serialInterfaceIsDisconnected();
    }
}

void MainWindow::on_pushButton_clicked()
{
    emit sigSerialInterfaceBufferClear();
}

void MainWindow::on_pushButton_7_clicked()
{
    if (ui->pushButton_7->text() == "Run Test")
        AutoSendnRead();
    else if (ui->pushButton_7->text() == "Pause Test")
        PauseAutoSendnRead();
}

void MainWindow::on_comboBoxSerial_currentIndexChanged(const QString &arg1)
{
    if (arg1 != "")
        serialInterfaceIsDisconnected();
}

void MainWindow::on_pushButton_2_clicked()
{
    if (ui->pushButton_2->text() == "HV ON")
    {
        ui->pushButton_2->setText("HV OFF");
        emit sigSerialInterfaceAddAPairToCommandList(APairOfPrioAndCommand(quint8(ui->spinBox->value()),NowUHV2->HVSwitch().On().GenMsg()));
    }
    else
    {
        ui->pushButton_2->setText("HV ON");
        emit sigSerialInterfaceAddAPairToCommandList(APairOfPrioAndCommand(quint8(ui->spinBox->value()),NowUHV2->HVSwitch().Off().GenMsg()));
    }
}

void MainWindow::on_spinBox_9_valueChanged(int arg1)
{
    quint8 tmpQInt8 = arg1;
    if (NowUHV2->GetBPNo() != tmpQInt8)
    {
        if (!BinaryProtocol::IsBP(tmpQInt8))
            NowUHV2->SetBPNo(tmpQInt8);
        else
            NowUHV2 = &BinaryProtocol::BP(tmpQInt8);
    }
}

void MainWindow::on_spinBox_10_valueChanged(int arg1)
{
    NowUHV2->Ch(quint8(arg1+BinaryProtocol::ChannelBase));
}

void MainWindow::on_pushButton_6_clicked()
{
    if (ui->pushButton_6->text() == "PROT ON")
    {
        ui->pushButton_6->setText("PROT OFF");
        emit sigSerialInterfaceAddAPairToCommandList(APairOfPrioAndCommand(quint8(ui->spinBox_8->value()),NowUHV2->ProtectSwitch().On().GenMsg()));
    }
    else
    {
        ui->pushButton_6->setText("PROT ON");
        emit sigSerialInterfaceAddAPairToCommandList(APairOfPrioAndCommand(quint8(ui->spinBox_8->value()),NowUHV2->ProtectSwitch().Off().GenMsg()));
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    AListOfPrioAndCommand tmpQMulMap;
    quint8 tmpKey = ui->spinBox_3->value();
    QByteArray tmpVal = NowUHV2->ReadP().GenMsg();
    for (int i = 1; i <= ui->spinBox_2->value(); i++)
        tmpQMulMap.insert(tmpKey, tmpVal);
    emit sigSerialInterfaceAddAListToCommandList(tmpQMulMap);
}

void MainWindow::on_pushButton_4_clicked()
{
    AListOfPrioAndCommand tmpQMulMap;
    quint8 tmpKey = ui->spinBox_5->value();
    QByteArray tmpVal = NowUHV2->ReadV().GenMsg();
    for (int i = 1; i <= ui->spinBox_4->value(); i++)
        tmpQMulMap.insert(tmpKey, tmpVal);
    emit sigSerialInterfaceAddAListToCommandList(tmpQMulMap);
}

void MainWindow::on_pushButton_5_clicked()
{
    AListOfPrioAndCommand tmpQMulMap;
    quint8 tmpKey = ui->spinBox_7->value();
    QByteArray tmpVal = NowUHV2->ReadI().GenMsg();
    for (int i = 1; i <= ui->spinBox_6->value(); i++)
        tmpQMulMap.insert(tmpKey, tmpVal);
    emit sigSerialInterfaceAddAListToCommandList(tmpQMulMap);
}

void MainWindow::ToggleDisplayHighlightSendRead(quint8 option)
{
    switch (option) {
    case 0:
        ui->label_MessageSent->setStyleSheet(QStringLiteral("QLabel{color: rgb(0, 0, 0);}"));
        ui->label_MessageRead->setStyleSheet(QStringLiteral("QLabel{color: rgb(0, 0, 0);}"));
        break;
    case 1:
        ui->label_MessageSent->setStyleSheet(QStringLiteral("QLabel{color: rgb(0, 0, 255);}"));
        ui->label_MessageRead->setStyleSheet(QStringLiteral("QLabel{color: rgb(0, 0, 0);}"));
        break;
    case 2:
        ui->label_MessageSent->setStyleSheet(QStringLiteral("QLabel{color: rgb(0, 0, 0);}"));
        ui->label_MessageRead->setStyleSheet(QStringLiteral("QLabel{color: rgb(0, 0, 255);}"));
        break;
    default:
        break;
    }
}

