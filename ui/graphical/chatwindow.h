#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QListWidgetItem>
#include "PeersInfo.h"
#include "dataio.h"
#include "UiNetlibInterfaces.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatWindow; }
QT_END_NAMESPACE

class ChatWindow : public QMainWindow, public UiCallbacks {
    Q_OBJECT

public:
    ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

    void newAuthMessage(std::unique_ptr<AuthMessage> ptr) override;
    void newTextMessage(std::unique_ptr<TextMessage> ptr) override;
    void newImageMessage(std::unique_ptr<ImageMessage> ptr) override;
    void peerDisconnect(const Peer &peer) override;
    void bindSucceeded() override;

signals:
    void bindSignal();
    void updateChatSignal(QString peer, QString msg, QBrush forColor, QBrush backColor);

private slots:
    void bindSlot();
    void updateChatSlot(QString, QString, QBrush, QBrush);

    void on_btnImage_clicked();
    void on_btnSend_clicked();
    void on_btnListen_clicked();
    void showContextMenu(const QPoint&);
    void connectPeer();
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:
    Ui::ChatWindow *ui;
    std::shared_ptr<std::map<std::string, Peer>> peersInfo;
    std::shared_ptr<NetOps> networking;
    std::map<QString, std::vector<std::shared_ptr<QListWidgetItem>>> chatHistory;

    void readPeersInfo();
};

#endif
