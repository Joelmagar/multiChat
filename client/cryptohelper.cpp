#include "cryptohelper.h"

#include <QDebug>
#include <sodium.h>

QByteArray CryptoHelper::publicKeyData;
QByteArray CryptoHelper::privateKeyData;

bool CryptoHelper::initialize()
{
    // Don't regenerate the key if already initialized
    if (publicKeyData.size() == crypto_box_PUBLICKEYBYTES &&
        privateKeyData.size() == crypto_box_SECRETKEYBYTES) {
        qDebug() << "[Crypto] Already initialized";
        return true;
    }

    if (sodium_init() < 0) {
        qDebug() << "[Crypto] sodium_init failed";
        return false;
    }

    publicKeyData.resize(crypto_box_PUBLICKEYBYTES);
    privateKeyData.resize(crypto_box_SECRETKEYBYTES);

    int result = crypto_box_keypair(
        reinterpret_cast<unsigned char *>(publicKeyData.data()),
        reinterpret_cast<unsigned char *>(privateKeyData.data())
    );

    if (result != 0) {
        qDebug() << "[Crypto] Keypair generation failed";

        publicKeyData.clear();
        privateKeyData.clear();

        return false;
    }

    qDebug() << "[Crypto] Keypair generated";
    qDebug() << "[Crypto] Public key bytes:"
             << publicKeyData.size();
    qDebug() << "[Crypto] Private key bytes:"
             << privateKeyData.size();

    return true;
}

QByteArray CryptoHelper::publicKey()
{
    if (publicKeyData.size() != crypto_box_PUBLICKEYBYTES) {
        qDebug() << "[Crypto] ERROR: Public key is not initialized!";
        qDebug() << "[Crypto] Current public key size:"
                 << publicKeyData.size();

        return QByteArray();
    }

    QByteArray encoded = publicKeyData.toBase64();

    qDebug() << "[Crypto] Sending public key";
    qDebug() << "[Crypto] Raw bytes:"
             << publicKeyData.size();
    qDebug() << "[Crypto] Base64 bytes:"
             << encoded.size();

    return encoded;
}

QByteArray CryptoHelper::encrypt(
    const QString &message,
    const QByteArray &recipientPublicKey
)
{
    qDebug() << "==============================";
    qDebug() << "[Crypto] ENCRYPT";

    QByteArray publicKey =
        QByteArray::fromBase64(recipientPublicKey);

    qDebug() << "Recipient key base64 size:"
             << recipientPublicKey.size();

    qDebug() << "Decoded public key size:"
             << publicKey.size();

    if (publicKey.size() != crypto_box_PUBLICKEYBYTES) {
        qDebug() << "[Crypto] INVALID PUBLIC KEY!";
        qDebug() << "Expected:"
                 << crypto_box_PUBLICKEYBYTES;
        qDebug() << "Received:"
                 << publicKey.size();

        return QByteArray();
    }

    QByteArray plaintext = message.toUtf8();

    QByteArray ciphertext(
        crypto_box_SEALBYTES + plaintext.size(),
        Qt::Uninitialized
    );

    int result = crypto_box_seal(
        reinterpret_cast<unsigned char *>(ciphertext.data()),
        reinterpret_cast<const unsigned char *>(plaintext.constData()),
        static_cast<unsigned long long>(plaintext.size()),
        reinterpret_cast<const unsigned char *>(publicKey.constData())
    );

    qDebug() << "[Crypto] crypto_box_seal result:"
             << result;

    if (result != 0) {
        qDebug() << "[Crypto] Encryption failed";
        return QByteArray();
    }

    QByteArray encoded = ciphertext.toBase64();

    qDebug() << "[Crypto] Encryption successful";

    return encoded;
}

QString CryptoHelper::decrypt(
    const QByteArray &encryptedMessage
)
{
    QByteArray ciphertext =
        QByteArray::fromBase64(encryptedMessage);

    if (ciphertext.size() < crypto_box_SEALBYTES) {
        qDebug() << "[Crypto] Invalid ciphertext";
        return QString();
    }

    QByteArray plaintext(
        ciphertext.size() - crypto_box_SEALBYTES,
        Qt::Uninitialized
    );

    int result = crypto_box_seal_open(
        reinterpret_cast<unsigned char *>(plaintext.data()),
        reinterpret_cast<const unsigned char *>(ciphertext.constData()),
        static_cast<unsigned long long>(ciphertext.size()),
        reinterpret_cast<const unsigned char *>(publicKeyData.constData()),
        reinterpret_cast<const unsigned char *>(privateKeyData.constData())
    );

    if (result != 0) {
        qDebug() << "[Crypto] Decryption failed";
        return QString();
    }

    return QString::fromUtf8(plaintext);
}