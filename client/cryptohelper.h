#pragma once

#include <QString>
#include <QByteArray>

class CryptoHelper
{
public:
    static bool initialize();

    static QByteArray publicKey();

    static QByteArray encrypt(
        const QString &message,
        const QByteArray &recipientPublicKey
    );

    static QString decrypt(
        const QByteArray &encryptedMessage
    );

private:
    static QByteArray publicKeyData;
    static QByteArray privateKeyData;
};