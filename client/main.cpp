#include <QApplication>
#include <QInputDialog>
#include "chatwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    bool ok;
    QString name = QInputDialog::getText(
        nullptr, "Enter Name", "Your name:", QLineEdit::Normal, "", &ok);

    if (!ok || name.trimmed().isEmpty())
        return 0;

    ChatWindow window(name);
    window.show();

    return app.exec();
}