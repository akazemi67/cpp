#include "chatwindow.h"
#include "./ui_chatwindow.h"
#include <QMenu>
#include <QMessageBox>

ChatWindow::ChatWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    ui->lstAllPeers->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->lstAllPeers, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    ui->lstAllPeers->addItem("Test1");
    ui->lstAllPeers->addItem("Test2");
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

void ChatWindow::on_btnListen_clicked()
{

}

