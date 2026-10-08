#include "cryptohelper.h"

#include <sodium.h>

QByteArray CryptoHelper::publicKeyData;
QByteArray CryptoHelper::privateKeyData;

bool CryptoHelper::initialize()
{
    if (sodium_init() < 0)
        return false;

    publicKeyData.resize(crypto_box_PUBLICKEYBYTES);
    privateKeyData.resize(crypto_box_SECRETKEYBYTES);

    crypto_box_keypair(
        reinterpret_cast<unsigned char *>(publicKeyData.data()),
        reinterpret_cast<unsigned char *>(privateKeyData.data())
    );

    return true;
}

QByteArray CryptoHelper::publicKey()
{
    return publicKeyData.toBase64();
}

QByteArray CryptoHelper::encrypt(
    const QString &message,
    const QByteArray &recipientPublicKey
)
{
    QByteArray publicKey =
        QByteArray::fromBase64(recipientPublicKey);

    if (publicKey.size() != crypto_box_PUBLICKEYBYTES)
        return QByteArray();

    QByteArray plaintext = message.toUtf8();

    QByteArray ciphertext(
        crypto_box_SEALBYTES + plaintext.size(),
        Qt::Uninitialized
    );

    crypto_box_seal(
        reinterpret_cast<unsigned char *>(ciphertext.data()),
        reinterpret_cast<const unsigned char *>(plaintext.constData()),
        plaintext.size(),
        reinterpret_cast<const unsigned char *>(publicKey.constData())
    );

    return ciphertext.toBase64();
}

QString CryptoHelper::decrypt(
    const QByteArray &encryptedMessage
)
{
    QByteArray ciphertext =
        QByteArray::fromBase64(encryptedMessage);

    if (ciphertext.size() < crypto_box_SEALBYTES)
        return QString();

    QByteArray plaintext(
        ciphertext.size() - crypto_box_SEALBYTES,
        Qt::Uninitialized
    );

    int result = crypto_box_seal_open(
        reinterpret_cast<unsigned char *>(plaintext.data()),
        reinterpret_cast<const unsigned char *>(ciphertext.constData()),
        ciphertext.size(),
        reinterpret_cast<const unsigned char *>(publicKeyData.constData()),
        reinterpret_cast<const unsigned char *>(privateKeyData.constData())
    );

    if (result != 0)
        return QString();

    return QString::fromUtf8(plaintext);
}