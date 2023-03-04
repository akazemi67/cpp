#include "chatwindow.h"
#include "./ui_chatwindow.h"
#include "logging.h"

ChatWindow::ChatWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    ui->lstAllPeers->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->lstAllPeers, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    readPeersInfo();
}

void ChatWindow::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->lstAllPeers->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.addAction("Connect", this, SLOT(connectPeer()));

    myMenu.exec(globalPos);
}

void ChatWindow::connectPeer(){
    const QString &text= ui->lstAllPeers->currentItem()->text();
    QMessageBox::information(this, "connect","Selected: "+ text);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::on_btnImage_clicked()
{

}

void ChatWindow::on_btnSend_clicked()
{

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
    networkingOk = true;
    ui->edtPort->setEnabled(false);
    ui->edtName->setEnabled(false);
    ui->btnListen->setEnabled(false);
    QMessageBox::information(this, "BindOK", "Listening Succeeded. Waiting for connections.");
}

