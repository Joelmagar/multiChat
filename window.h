// chatwindow.h
#pragma once
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

class ChatWindow : public QWidget {
  Q_OBJECT
public:
  ChatWindow(QWidget *parent = nullptr) : QWidget(parent) {
    chatLog = new QTextEdit(this);
    chatLog->setReadOnly(true);
    input = new QLineEdit(this);
    sendBtn = new QPushButton("Send", this);

    auto *inputRow = new QHBoxLayout();
    inputRow->addWidget(input);
    inputRow->addWidget(sendBtn);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(chatLog);
    layout->addLayout(inputRow);
    setLayout(layout);
    setWindowTitle("Chat Client");

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &ChatWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this,
            [this]() { chatLog->append("Disconnected from server."); });
    connect(sendBtn, &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(input, &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);

    socket->connectToHost("127.0.0.1", 8080);
  }

private slots:
  void onReadyRead() {
    QByteArray data = socket->readAll();
    chatLog->append(QString::fromUtf8(data));
  }

  void sendMessage() {
    QString text = input->text();
    if (text.isEmpty())
      return;
    QString fullMsg = name + ": " + text;
    socket->write(fullMsg.toUtf8());
    input->clear();
  }

public:
  QString name;

private:
  QTcpSocket *socket;
  QTextEdit *chatLog;
  QLineEdit *input;
  QPushButton *sendBtn;
};
