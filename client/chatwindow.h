#pragma once

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QMap>
#include <QStringList>
#include "cryptohelper.h"


class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(
     
        const QString &userName,
        const QString &serverIp,
        QWidget *parent = nullptr
    );

private slots:
    void onReadyRead();
    void onDisconnected();
    void onConnected();
    void sendMessage();
    void onUserClicked(QListWidgetItem *item);

private:
    QMap<QString, QStringList> chatHistory;

    void addMessageToHistory(
        const QString &target,
        const QString &message
    );

    void showChatHistory(
        const QString &target
    );

    QString name;
    QString serverAddress;
    QString privateTarget;

    QByteArray receiveBuffer;

    QMap<QString, QByteArray> publicKeys;

    QTcpSocket *socket;

    QTextEdit *chatLog;
    QLineEdit *input;
    QPushButton *sendBtn;
    QListWidget *userList;
    QLabel *targetLabel;
};