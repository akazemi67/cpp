#include "chatwindow.h"
#include "./ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
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


void ChatWindow::on_btnConnect_clicked()
{

}

