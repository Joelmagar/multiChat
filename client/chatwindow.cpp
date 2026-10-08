#include "chatwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMap>
#include <QStringList>

#include <QMessageBox>
ChatWindow::ChatWindow(
    const QString &userName,
    const QString &serverIp,
    QWidget *parent
)
    : QWidget(parent),
      name(userName),
      serverAddress(serverIp),
      privateTarget("")
{
    setStyleSheet(
        "background-color: #1e1e2e; "
        "color: #cdd6f4;"
    );

    // =========================
    // ONLINE USERS LABEL
    // =========================

    QLabel *onlineLabel = new QLabel("ONLINE USERS", this);

    onlineLabel->setStyleSheet(
        "font-size: 10px; "
        "font-weight: bold; "
        "color: #6c7086;"
        "padding: 12px 14px 6px 14px; "
        "letter-spacing: 1px;"
    );


    // =========================
    // USER LIST
    // =========================

    userList = new QListWidget(this);

    userList->setFixedWidth(200);
    userList->setFocusPolicy(Qt::NoFocus);

    userList->setStyleSheet(R"(
        QListWidget {
            background-color: #181825;
            border: none;
            outline: none;
            padding: 4px;
        }

        QListWidget::item {
            color: #cdd6f4;
            padding: 10px 12px;
            border-radius: 6px;
            margin: 2px 4px;
        }

        QListWidget::item:hover {
            background-color: #313244;
            color: #cdd6f4;
        }

        QListWidget::item:selected {
            background-color: #1a56db;
            color: #ffffff;
        }
    )");


    // =========================
    // EVERYONE ITEM
    // =========================

    QListWidgetItem *everyoneItem =
        new QListWidgetItem("🌐  Group");

    everyoneItem->setData(Qt::UserRole, "");

    userList->addItem(everyoneItem);
    userList->setCurrentItem(everyoneItem);


    // =========================
    // SIDEBAR LAYOUT
    // =========================

    auto *sidebarLayout = new QVBoxLayout();

    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    sidebarLayout->addWidget(onlineLabel);
    sidebarLayout->addWidget(userList);


    // =========================
    // CHAT TARGET LABEL
    // =========================

    targetLabel =
        new QLabel("Chatting with: Everyone", this);

    targetLabel->setStyleSheet(
        "font-size: 13px; "
        "font-weight: bold; "
        "color: #89b4fa;"
        "padding: 10px 14px; "
        "background-color: #181825;"
        "border-bottom: 1px solid #313244;"
    );


    // =========================
    // CHAT LOG
    // =========================

    chatLog = new QTextEdit(this);

    chatLog->setReadOnly(true);

    chatLog->setStyleSheet(R"(
        QTextEdit {
            background-color: #1e1e2e;
            color: #cdd6f4;
            border: none;
            padding: 10px 14px;
            font-size: 13px;
            font-family: monospace;
        }
    )");


    // =========================
    // INPUT
    // =========================

    input = new QLineEdit(this);

    input->setStyleSheet(R"(
        QLineEdit {
            background-color: #313244;
            color: #cdd6f4;
            border: none;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 13px;
        }

        QLineEdit:focus {
            background-color: #45475a;
        }
    )");

    input->setPlaceholderText(
        "Type a message to everyone..."
    );


    // =========================
    // SEND BUTTON
    // =========================

    sendBtn = new QPushButton("Send", this);

    sendBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1a56db;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #1e40af;
        }

        QPushButton:pressed {
            background-color: #1e3a8a;
        }
    )");


    // =========================
    // INPUT ROW
    // =========================

    auto *inputRow = new QHBoxLayout();

    inputRow->setContentsMargins(10, 8, 10, 10);
    inputRow->setSpacing(8);

    inputRow->addWidget(input);
    inputRow->addWidget(sendBtn);


    // =========================
    // INPUT WRAPPER
    // =========================

    auto *inputWrapper = new QWidget(this);

    inputWrapper->setStyleSheet(
        "background-color: #181825; "
        "border-top: 1px solid #313244;"
    );

    inputWrapper->setLayout(inputRow);


    // =========================
    // CHAT LAYOUT
    // =========================

    auto *chatLayout = new QVBoxLayout();

    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    chatLayout->addWidget(targetLabel);
    chatLayout->addWidget(chatLog);
    chatLayout->addWidget(inputWrapper);


    // =========================
    // SIDEBAR WRAPPER
    // =========================

    auto *sidebarWrapper = new QWidget(this);

    sidebarWrapper->setStyleSheet(
        "background-color: #181825;"
    );

    sidebarWrapper->setLayout(sidebarLayout);
    sidebarWrapper->setFixedWidth(200);


    // =========================
    // MAIN LAYOUT
    // =========================

    auto *mainLayout = new QHBoxLayout(this);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(sidebarWrapper);
    mainLayout->addLayout(chatLayout);

    setLayout(mainLayout);


    // =========================
    // WINDOW
    // =========================

    resize(700, 520);

    setWindowTitle("NetChat — " + name);


    // =========================
    // SOCKET
    // =========================

    socket = new QTcpSocket(this);

    connect(
        socket,
        &QTcpSocket::readyRead,
        this,
        &ChatWindow::onReadyRead
    );

    connect(
        socket,
        &QTcpSocket::disconnected,
        this,
        &ChatWindow::onDisconnected
    );

    connect(
        socket,
        &QTcpSocket::connected,
        this,
        &ChatWindow::onConnected
    );

    connect(
        sendBtn,
        &QPushButton::clicked,
        this,
        &ChatWindow::sendMessage
    );

    connect(
        input,
        &QLineEdit::returnPressed,
        this,
        &ChatWindow::sendMessage
    );

    connect(
        userList,
        &QListWidget::itemClicked,
        this,
        &ChatWindow::onUserClicked
    );


    socket->connectToHost(serverAddress, 8080);
}


// ============================================================
// CONNECTED
// ============================================================

void ChatWindow::onConnected()
{
    // Register username
    socket->write((name + "\n").toUtf8());

    // Send public key
    QString keyMessage =
        "KEY|" +
        name +
        "|" +
        QString::fromLatin1(CryptoHelper::publicKey());

    socket->write((keyMessage + "\n").toUtf8());

    QString message =
        QString(
            "<span style='color:#6c7086;'>"
            "Connected to server as "
            "<b style='color:#a6e3a1;'>%1</b>"
            "</span>"
        ).arg(name);

    addMessageToHistory("", message);

    if (privateTarget.isEmpty())
        chatLog->append(message);
}

// ============================================================
// STORE MESSAGE IN A CONVERSATION
// ============================================================

void ChatWindow::addMessageToHistory(
    const QString &target,
    const QString &message
)
{
    chatHistory[target].append(message);
}


// ============================================================
// SHOW SELECTED CONVERSATION
// ============================================================

void ChatWindow::showChatHistory(
    const QString &target
)
{
    chatLog->clear();

    if (!chatHistory.contains(target))
        return;

    const QStringList messages =
        chatHistory.value(target);

    for (const QString &message : messages)
        chatLog->append(message);
    }
}


// ============================================================
// RECEIVE DATA
// ============================================================

void ChatWindow::onReadyRead()
{
    receiveBuffer += socket->readAll();

    while (true) {

        int index =
            receiveBuffer.indexOf('\n');

        if (index == -1)
            break;

        QString msg =
            QString::fromUtf8(
                receiveBuffer.left(index)
            ).trimmed();

        receiveBuffer.remove(
            0,
            index + 1
        );

        if (msg.isEmpty())
            continue;


        // =========================
        // USERNAME TAKEN
        // =========================

        if (msg == "USERNAME_TAKEN") {

            chatLog->append(
                "<span style='color:#f38ba8;'>"
                "[System] Username already taken."
                "</span>"
            );

            sendBtn->setEnabled(false);
            input->setEnabled(false);

            continue;
        }


        // =========================
        // PUBLIC KEY
        // =========================

        if (msg.startsWith("KEY|")) {

            QStringList parts =
                msg.split("|");

            if (parts.size() >= 3) {

                QString username =
                    parts[1];

                QByteArray publicKey =
                    parts[2].toLatin1();

                publicKeys[username] =
                    publicKey;
            }

            continue;
        }


        // =========================
        // USER JOINED
        // =========================

        if (msg.startsWith("USER_JOINED:")) {

            QString user =
                msg.mid(12).trimmed();

            bool exists = false;

            for (int i = 0;
                 i < userList->count();
                 i++) {

                if (
                    userList->item(i)
                        ->data(Qt::UserRole)
                        .toString()
                    == user
                ) {
                    exists = true;
                    break;
                }
            }

            if (!exists && !user.isEmpty()) {

                QListWidgetItem *item =
                    new QListWidgetItem(
                        "👤  " + user
                    );

                item->setData(
                    Qt::UserRole,
                    user
                );

                userList->addItem(item);
            }

            continue;
        }


        // =========================
        // USER LEFT
        // =========================

        if (msg.startsWith("USER_LEFT:")) {

            QString user =
                msg.mid(10).trimmed();

            for (int i = 0;
                 i < userList->count();
                 i++) {

                if (
                    userList->item(i)
                        ->data(Qt::UserRole)
                        .toString()
                    == user
                ) {

                    delete userList->takeItem(i);
                    break;
                }
            }

            if (privateTarget == user) {

                privateTarget = "";

                targetLabel->setText(
                    "Chatting with: Everyone"
                );

                input->setPlaceholderText(
                    "Type a message..."
                );

                userList->setCurrentRow(0);

                showChatHistory("");
            }

            continue;
        }


        // =========================
        // PRIVATE MESSAGE
        // =========================

        if (msg.startsWith("PRIVATE|")) {

            QStringList parts =
                msg.split("|");

            if (parts.size() >= 3) {

                QString sender =
                    parts[1];

                QByteArray encrypted =
                    parts[2].toLatin1();

                QString decrypted =
                    CryptoHelper::decrypt(
                        encrypted
                    );

                if (decrypted.isEmpty()) {

                    if (privateTarget == sender) {

                        chatLog->append(
                            QString(
                                "<span style='color:#f38ba8;'>"
                                "[Crypto] Unable to decrypt "
                                "message from %1."
                                "</span>"
                            ).arg(sender)
                        );
                    }

                    continue;
                }

                QString displayMsg =
                    QString(
                        "<span style='color:#cba6f7;'>"
                        "[Private from %1]: %2"
                        "</span>"
                    )
                    .arg(sender)
                    .arg(decrypted);

                addMessageToHistory(
                    sender,
                    displayMsg
                );

                if (privateTarget == sender)
                    chatLog->append(displayMsg);
            }

            continue;
        }


        // =========================
        // NORMAL PUBLIC MESSAGE
        // =========================

        QString displayMsg =
            QString(
                "<span style='color:#cdd6f4;'>"
                "%1"
                "</span>"
            ).arg(msg);

        addMessageToHistory(
            "",
            displayMsg
        );

        if (privateTarget.isEmpty())
            chatLog->append(displayMsg);
    }
}

// ============================================================
// DISCONNECTED
// ============================================================

void ChatWindow::onDisconnected()
{
    QString systemMsg =
        "<span style='color:#f38ba8;'>"
        "[System] Disconnected from server."
        "</span>";

    addMessageToHistory(
        privateTarget,
        systemMsg
    );

    chatLog->append(systemMsg);

    sendBtn->setEnabled(false);
    input->setEnabled(false);
}


// ============================================================
// USER SELECTED
// ============================================================

void ChatWindow::onUserClicked(QListWidgetItem *item)
{
    privateTarget =
        item->data(Qt::UserRole).toString();

    if (privateTarget.isEmpty()) {

        targetLabel->setText(
            "Chatting with: Everyone"
        );

        input->setPlaceholderText(
            "Type a message to everyone..."
        );

    } else {

        targetLabel->setText(
            "Chatting with: " +
            privateTarget +
            "  🔒 private"
        );

        input->setPlaceholderText(
            "Private message to " +
            privateTarget +
            "..."
        );
    }

    showChatHistory(privateTarget);
}

// ============================================================
// SEND MESSAGE
// ============================================================

void ChatWindow::sendMessage()
{
    QString text = input->text().trimmed();

    if (text.isEmpty())
        return;

    // =========================
    // PUBLIC MESSAGE
    // =========================
    if (privateTarget.isEmpty()) {

        QString fullMsg =
            name + ": " + text;

        QString displayMsg =
            QString(
                "<span style='color:#89dceb;'>%1</span>"
            ).arg(fullMsg);

        addMessageToHistory("", displayMsg);

        chatLog->append(displayMsg);

        socket->write(
            (fullMsg + "\n").toUtf8()
        );

        input->clear();

        return;
    }

    // =========================
    // PRIVATE MESSAGE
    // =========================

    if (!publicKeys.contains(privateTarget)) {

        chatLog->append(
            QString(
                "<span style='color:#f38ba8;'>"
                "[Crypto] Public key for %1 is not available."
                "</span>"
            ).arg(privateTarget)
        );

        return;
    }

    QByteArray encrypted =
        CryptoHelper::encrypt(
            text,
            publicKeys[privateTarget]
        );

    if (encrypted.isEmpty()) {

        chatLog->append(
            "<span style='color:#f38ba8;'>"
            "[Crypto] Encryption failed."
            "</span>"
        );

        return;
    }

    QString packet =
        "PRIVATE|" +
        privateTarget +
        "|" +
        QString::fromLatin1(encrypted);

    socket->write(
        (packet + "\n").toUtf8()
    );

    QString displayMsg =
        QString(
            "<span style='color:#cba0f7;'>"
            "[You → %1]: %2"
            "</span>"
        )
        .arg(privateTarget)
        .arg(text);

    addMessageToHistory(
        privateTarget,
        displayMsg
    );

    chatLog->append(displayMsg);

    input->clear();
}