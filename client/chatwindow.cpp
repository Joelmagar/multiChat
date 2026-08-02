#include "chatwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ChatWindow::ChatWindow(const QString &userName, QWidget *parent)
    : QWidget(parent), name(userName) {

    chatLog = new QTextEdit(this);
    chatLog->setReadOnly(true);

    input = new QLineEdit(this);
    input->setPlaceholderText("Type a message...");

    sendBtn = new QPushButton("Send", this);

    auto *inputRow = new QHBoxLayout();
    inputRow->addWidget(input);
    inputRow->addWidget(sendBtn);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(chatLog);
    layout->addLayout(inputRow);
    setLayout(layout);
    resize(420, 520);
    setWindowTitle("Chat Client - " + name);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ChatWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ChatWindow::onDisconnected);
    connect(socket, &QTcpSocket::connected, this, &ChatWindow::onConnected);
    connect(sendBtn, &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(input, &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);

    socket->connectToHost("127.0.0.1", 8080);
}

void ChatWindow::onConnected() {
    chatLog->append("Connected to server as " + name);
}

void ChatWindow::onReadyRead() {
    QByteArray data = socket->readAll();
    chatLog->append(QString::fromUtf8(data));
}

void ChatWindow::onDisconnected() {
    chatLog->append("Disconnected from server.");
    sendBtn->setEnabled(false);
    input->setEnabled(false);
}

void ChatWindow::sendMessage() {
    QString text = input->text();
    if (text.isEmpty()) return;

    QString fullMsg = name + ": " + text;
    socket->write(fullMsg.toUtf8());
    input->clear();
}