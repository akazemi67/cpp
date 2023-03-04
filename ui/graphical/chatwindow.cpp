#include "chatwindow.h"
#include "./ui_chatwindow.h"
#include "logging.h"
#include "ImageWidget.h"
#include <QFileDialog>

ChatWindow::ChatWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::ChatWindow) {
    ui->setupUi(this);
    ui->lstAllPeers->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &ChatWindow::bindSignal, this, &ChatWindow::bindSlot);
    connect(this, &ChatWindow::updateChatSignal, this, &ChatWindow::updateChatSlot);
    connect(this, &ChatWindow::imageRecvSignal, this, &ChatWindow::imageRecvSlot);
    connect(ui->lstAllPeers, &QListWidget::currentItemChanged, this, &ChatWindow::onCurrentItemChanged);
    connect(ui->edtChat, &QLineEdit::returnPressed, this, &ChatWindow::on_btnSend_clicked);

    readPeersInfo();
    ui->btnSend->setEnabled(false);
    ui->btnImage->setEnabled(false);
    ui->edtPort->setText("1337");
    ui->edtName->setText("EliteQt");
}

void ChatWindow::showContextMenu(const QPoint &pos) {
    QPoint globalPos = ui->lstAllPeers->mapToGlobal(pos);
    QMenu connMenu;
    connMenu.addAction("Connect", this, SLOT(connectPeer()));
    connMenu.exec(globalPos);
}

void ChatWindow::connectPeer() {
    auto currentItem = ui->lstAllPeers->currentItem();
    const QString &name = currentItem->text();
    auto peer = peersInfo->find(name.toStdString());

    if(networking->connectPeer(peer->second)) {
        AuthMessage auth(ui->edtName->text().toStdString());
        try {
            networking->sendMessage(peer->second.name, auth);
        }catch (...){}

        currentItem->setForeground(QBrush(QColor(Qt::green)));
        ui->btnSend->setEnabled(true);
        ui->btnImage->setEnabled(true);

        emit updateChatSignal(QString(peer->second.name.c_str()),
                              "------ Chat Started ------");
    }
}

ChatWindow::~ChatWindow() {
    delete ui;
}

void ChatWindow::on_btnImage_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "",
                                                    tr("All files (*.*);;JPEG (*.jpg *.jpeg);;TIFF (*.tif)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            getLogger()->error("Error opening image file to read.");
            return;
        }

        QByteArray byteArray = file.readAll();
        auto imgBytes = std::make_shared<std::vector<uint8_t>>(byteArray.begin(), byteArray.end());
        ImageMessage imageMessage(imgBytes);
        auto peerName = ui->lstAllPeers->currentItem()->text();
        networking->sendMessage(peerName.toStdString(), imageMessage);
        file.close();
    }

}

void ChatWindow::on_btnSend_clicked() {
    auto msgText = ui->edtChat->text();
    auto peerName = ui->lstAllPeers->currentItem()->text();
    TextMessage txtMsg(msgText.toStdString());
    networking->sendMessage(peerName.toStdString(), txtMsg);
    ui->edtChat->setText("");
    emit updateChatSignal(peerName,"[SND]: "+ msgText);
}

void ChatWindow::on_btnListen_clicked() {
    try {
        int port = ui->edtPort->text().toInt();
        networking = createNetworking(port, this);
    }
    catch(...){
        getLogger()->error("Cannot listen for incoming connections!");
    }
}

void ChatWindow::readPeersInfo() {
    DataReaderFactory factory(DEFAULT_CONFIG_FILE_PATH);
    std::vector<std::unique_ptr<PeersDataReader>> readers = std::move(factory.createDataReadersFromConfig());
    peersInfo = std::make_shared<std::map<std::string, Peer>>();

    for(auto& reader : readers){
        auto data = reader->readData();
        if(data== nullptr)
            continue;
        for(auto &info : *data){
            peersInfo->insert({info.name, info});
            ui->lstAllPeers->addItem(QString(info.name.c_str()));
        }
    }
}

void ChatWindow::bindSucceeded() {
    emit bindSignal();
}

void ChatWindow::bindSlot() {
    ui->edtPort->setEnabled(false);
    ui->edtName->setEnabled(false);
    ui->btnListen->setEnabled(false);
    connect(ui->lstAllPeers, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    QMessageBox::information(this, "BindOK", "Listening Succeeded. Waiting for connections.");
}

void ChatWindow::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    ui->btnSend->setEnabled(false);
    ui->btnImage->setEnabled(false);
    if(current->foreground() == QBrush(QColor(Qt::green))){
        ui->btnSend->setEnabled(true);
        ui->btnImage->setEnabled(true);
    }
    updateCurrentPeerChat();
}

void ChatWindow::updateChatSlot(QString peer, QString msg) {
    auto chatItem = std::make_shared<QListWidgetItem>();
    chatHistory[peer].push_back(msg);
    updateCurrentPeerChat();
}

void ChatWindow::newAuthMessage(std::string peerName, std::unique_ptr<AuthMessage> authMsg) {
    QListWidgetItem *peerItem = nullptr;
    for(auto i=0; i<ui->lstAllPeers->count(); i++){
        auto item = ui->lstAllPeers->item(i);
        if(peerName == item->text().toStdString()){
            peerItem = item;
            break;
        }
    }
    if(!peerItem){
        peerItem = new QListWidgetItem(QString(peerName.c_str()));
        ui->lstAllPeers->addItem(peerItem);
    }

    peerItem->setForeground(QBrush(QColor(Qt::green)));
    emit updateChatSignal(peerItem->text(),"------ Chat Started ------");
}

void ChatWindow::newTextMessage(std::string peerName, std::unique_ptr<TextMessage> txtMsg) {
    emit updateChatSignal(QString(peerName.c_str()), "[RCV]: " + QString(txtMsg->text.c_str()));
}

void ChatWindow::newImageMessage(std::string peerName, std::unique_ptr<ImageMessage> imgMsg) {
    QString title = "IMG from:: ";
    title += peerName.c_str();
    emit imageRecvSignal(title, *imgMsg->image);
}

void ChatWindow::peerDisconnect(const std::string peerName) {

}

void ChatWindow::updateCurrentPeerChat() {
    ui->lstChat->clear();

    auto name = ui->lstAllPeers->currentItem()->text();
    auto chatIt = chatHistory.find(name);
    if(chatIt!=chatHistory.end()){
        for(auto item : chatHistory[name]){
            ui->lstChat->addItem(item);
        }
    }
}

void ChatWindow::imageRecvSlot(QString title, const std::vector<uint8_t> &imageData) {
    auto imgWidget = new ImageWidget(imageData);
    imgWidget->setGeometry(this->geometry());
    imgWidget->setWindowTitle(title);
    imgWidget->show();
}

