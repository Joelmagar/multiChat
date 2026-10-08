

#include <QApplication>
#include <QInputDialog>
#include "chatwindow.h"
#include <QMessageBox>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    bool ok;
    QString name = QInputDialog::getText(
        nullptr, "Enter Name", "Your name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty())
        return 0;
    name = name.trimmed();

if (name.contains(' ')) {
    QMessageBox::warning(
        nullptr,
        "Invalid Username",
        "Username cannot contain spaces."
    );
    return 0;
}
    QString serverIp = QInputDialog::getText(
        nullptr, "Server Address", "Server IP:", QLineEdit::Normal, "127.0.0.1", &ok);
    if (!ok || serverIp.trimmed().isEmpty())
        return 0;

    ChatWindow window(name, serverIp);
    window.show();

    return app.exec();
}