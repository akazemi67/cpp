#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ChatWindow; }
QT_END_NAMESPACE

class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void on_btnImage_clicked();

    void on_btnSend_clicked();

    void on_btnConnect_clicked();

private:
    Ui::ChatWindow *ui;
};
#endif
