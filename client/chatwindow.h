#pragma once
#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(const QString &userName, QWidget *parent = nullptr);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onConnected();
    void sendMessage();

private:
    QString name;
    QTcpSocket *socket;
    QTextEdit *chatLog;
    QLineEdit *input;
    QPushButton *sendBtn;
};