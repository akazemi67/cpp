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

    void bindSucceeded() override;

    void newAuthMessage(std::string peerName, std::unique_ptr<AuthMessage> authMsg) override;
    void newTextMessage(std::string peerName, std::unique_ptr<TextMessage> txtMsg) override;
    void newImageMessage(std::string peerName, std::unique_ptr<ImageMessage> imgMsg) override;
    void peerDisconnected(const std::string peerName) override;

signals:
    void bindSignal();
    void updateChatSignal(QString peer, QString msg);
    void imageRecvSignal(QString title, const std::vector<uint8_t>&imageData);
    void removePeerSignal(QString peer);

private slots:
    void bindSlot();
    void updateChatSlot(QString, QString);
    void imageRecvSlot(QString title, const std::vector<uint8_t>&imageData);
    void removePeerSlot(QString peer);

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
    std::map<QString, std::vector<QString>> chatHistory;

    void readPeersInfo();
    void updateCurrentPeerChat();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif
