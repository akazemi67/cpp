#include "chatwindow.h"
#include "./ui_chatwindow.h"
#include "logging.h"

ChatWindow::ChatWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    ui->lstAllPeers->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &ChatWindow::bindSignal, this, &ChatWindow::bindSlot);
    connect(this, &ChatWindow::updateChatSignal, this, &ChatWindow::updateChatSlot);
    connect(ui->lstAllPeers, &QListWidget::currentItemChanged, this, &ChatWindow::onCurrentItemChanged);

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
            networking->sendMessage(peer->second, auth);
        }catch (...){}
        currentItem->setForeground(QBrush(QColor(Qt::green)));

        emit updateChatSignal(QString(peer->second.name.c_str()),
                              "------ Chat Started ------",
                              QBrush(QColor(Qt::white)),
                              QBrush(QColor(Qt::black)));
    }
}

ChatWindow::~ChatWindow() {
    delete ui;
}

void ChatWindow::on_btnImage_clicked() {

}

void ChatWindow::on_btnSend_clicked() {

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

void ChatWindow::newAuthMessage(std::unique_ptr<AuthMessage> ptr) {

}

void ChatWindow::newTextMessage(std::unique_ptr<TextMessage> ptr) {

}

void ChatWindow::newImageMessage(std::unique_ptr<ImageMessage> ptr) {

}

void ChatWindow::peerDisconnect(const Peer &peer) {

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
    auto name = current->text();
    auto chatIt = chatHistory.find(name);
    for(auto i=0; i<ui->lstChat->count(); i++)
        ui->lstChat->takeItem(i);
    if(chatIt!=chatHistory.end()){
        for(auto item : chatHistory[name]){
            ui->lstChat->addItem(item.get());
        }

        ui->btnSend->setEnabled(true);
        ui->btnImage->setEnabled(true);
    }
    else {
        ui->btnSend->setEnabled(false);
        ui->btnImage->setEnabled(false);
    }
}

void ChatWindow::updateChatSlot(QString peer, QString msg, QBrush forColor, QBrush backColor) {
    auto chatItem = std::make_shared<QListWidgetItem>();
    chatItem->setText(msg);
    chatItem->setBackground(backColor);
    chatItem->setForeground(forColor);
    ui->lstChat->addItem(chatItem.get());
    chatHistory[peer].push_back(chatItem);
}

