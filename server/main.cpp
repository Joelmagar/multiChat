#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>

std::map<std::string, int> clients;

// username -> public key
std::map<std::string, std::string> publicKeys;

std::mutex clients_mutex;


// ============================================================
// TRIM
// ============================================================

std::string trim(const std::string &s)
{
    size_t start =
        s.find_first_not_of(" \n\r\t");

    if (start == std::string::npos)
        return "";

    size_t end =
        s.find_last_not_of(" \n\r\t");

    return s.substr(
        start,
        end - start + 1
    );
}


// ============================================================
// SEND MESSAGE
// ============================================================

void send_msg(
    int fd,
    const std::string &msg
)
{
    std::string framed =
        msg + "\n";

    send(
        fd,
        framed.c_str(),
        framed.length(),
        0
    );
}


// ============================================================
// BROADCAST
// ============================================================

void broadcast_message(
    const std::string &message,
    const std::string &sender_name
)
{
    std::lock_guard<std::mutex> lock(
        clients_mutex
    );

    for (auto &pair : clients) {

        if (pair.first != sender_name) {

            send_msg(
                pair.second,
                message
            );
        }
    }
}


// ============================================================
// HANDLE PUBLIC KEY
// ============================================================

void handle_key(
    const std::string &message,
    const std::string &sender_name
)
{
    /*
        Expected:

        KEY|username|publicKey
    */

    size_t first =
        message.find('|');

    size_t second =
        message.find('|', first + 1);

    if (
        first == std::string::npos ||
        second == std::string::npos
    ) {
        return;
    }

    std::string username =
        message.substr(
            first + 1,
            second - first - 1
        );

    std::string publicKey =
        message.substr(
            second + 1
        );

    // Only allow a client to register its own key.
    if (username != sender_name)
        return;

    {
        std::lock_guard<std::mutex> lock(
            clients_mutex
        );

        publicKeys[username] =
            publicKey;
    }


    // --------------------------------------------------------
    // Send this new user's key to everyone else
    // --------------------------------------------------------

    std::lock_guard<std::mutex> lock(
        clients_mutex
    );

    for (auto &pair : clients) {

        if (pair.first == sender_name)
            continue;

        std::string packet =
            "KEY|" +
            sender_name +
            "|" +
            publicKey;

        send_msg(
            pair.second,
            packet
        );
    }
}


// ============================================================
// HANDLE PRIVATE MESSAGE
// ============================================================

void private_message(
    const std::string &message,
    const std::string &sender_name
)
{
    /*
        Client sends:

        PRIVATE|target|encryptedMessage
    */

    size_t first =
        message.find('|');

    size_t second =
        message.find('|', first + 1);

    if (
        first == std::string::npos ||
        second == std::string::npos
    ) {
        return;
    }

    std::string target =
        message.substr(
            first + 1,
            second - first - 1
        );

    std::string encrypted =
        message.substr(
            second + 1
        );


    std::lock_guard<std::mutex> lock(
        clients_mutex
    );


    // --------------------------------------------------------
    // Check recipient
    // --------------------------------------------------------

    auto targetIt =
        clients.find(target);

    if (targetIt == clients.end()) {

        auto senderIt =
            clients.find(sender_name);

        if (senderIt != clients.end()) {

            send_msg(
                senderIt->second,
                "[System] User '" +
                target +
                "' not found."
            );
        }

        return;
    }


    // --------------------------------------------------------
    // Forward encrypted message
    // --------------------------------------------------------

    /*
        Server changes:

        PRIVATE|target|encrypted

        into:

        PRIVATE|sender|encrypted
    */

    std::string packet =
        "PRIVATE|" +
        sender_name +
        "|" +
        encrypted;

    send_msg(
        targetIt->second,
        packet
    );
}


// ============================================================
// SEND EXISTING PUBLIC KEYS TO NEW USER
// ============================================================

void send_existing_keys(
    int client_fd,
    const std::string &new_username
)
{
    std::lock_guard<std::mutex> lock(
        clients_mutex
    );

    for (auto &pair : publicKeys) {

        // Don't send the user's own key
        if (pair.first == new_username)
            continue;

        std::string packet =
            "KEY|" +
            pair.first +
            "|" +
            pair.second;

        send_msg(
            client_fd,
            packet
        );
    }
}


// ============================================================
// HANDLE CLIENT
// ============================================================

void handle_client(
    int client_fd
)
{
    // ========================================================
    // READ USERNAME
    // ========================================================

    std::string incoming;

    char ch;

    while (
        recv(
            client_fd,
            &ch,
            1,
            0
        ) > 0
    ) {

        if (ch == '\n')
            break;

        incoming += ch;
    }


    std::string username =
        trim(incoming);


    if (username.empty()) {

        close(client_fd);

        return;
    }


    // ========================================================
    // CHECK DUPLICATE USERNAME
    // ========================================================

    {
        std::lock_guard<std::mutex> lock(
            clients_mutex
        );

        if (clients.count(username)) {

            send_msg(
                client_fd,
                "USERNAME_TAKEN"
            );

            close(client_fd);

            return;
        }

        clients[username] =
            client_fd;
    }


    std::cout
        << username
        << " connected.\n";


    // ========================================================
    // SEND EXISTING USERS
    // ========================================================

    {
        std::lock_guard<std::mutex> lock(
            clients_mutex
        );

        for (auto &pair : clients) {

            if (pair.first != username) {

                send_msg(
                    client_fd,
                    "USER_JOINED:" +
                    pair.first
                );
            }
        }
    }


    // ========================================================
    // SEND EXISTING PUBLIC KEYS
    // ========================================================

    send_existing_keys(
        client_fd,
        username
    );


    // ========================================================
    // NOTIFY OTHER CLIENTS
    // ========================================================

    broadcast_message(
        "USER_JOINED:" +
        username,
        username
    );

    broadcast_message(
        username +
        " has joined the chat.",
        username
    );


    // ========================================================
    // MESSAGE LOOP
    // ========================================================

    std::string lineBuffer;

    char buf;

    while (true) {

        int bytes =
            recv(
                client_fd,
                &buf,
                1,
                0
            );


        // ====================================================
        // DISCONNECTED
        // ====================================================

        if (bytes <= 0) {

            {
                std::lock_guard<std::mutex> lock(
                    clients_mutex
                );

                clients.erase(username);

                // Remove public key
                publicKeys.erase(username);
            }


            broadcast_message(
                "USER_LEFT:" +
                username,
                username
            );


            std::cout
                << username
                << " disconnected.\n";


            close(client_fd);

            break;
        }


        // ====================================================
        // COMPLETE MESSAGE
        // ====================================================

        if (buf == '\n') {

            std::string message =
                trim(lineBuffer);

            lineBuffer.clear();


            if (message.empty())
                continue;


            // ================================================
            // PUBLIC KEY
            // ================================================

            if (
                message.rfind(
                    "KEY|",
                    0
                ) == 0
            ) {

                handle_key(
                    message,
                    username
                );

                continue;
            }


            // ================================================
            // PRIVATE MESSAGE
            // ================================================

            if (
                message.rfind(
                    "PRIVATE|",
                    0
                ) == 0
            ) {

                private_message(
                    message,
                    username
                );

                continue;
            }


            // ================================================
            // PUBLIC MESSAGE
            // ================================================

            broadcast_message(
                message,
                username
            );

        } else {

            lineBuffer += buf;
        }
    }
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    int server_fd =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );


    if (server_fd < 0) {

        std::cerr
            << "Failed to create socket.\n";

        return 1;
    }


    // Allow quick restart
    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );


    sockaddr_in server_addr{};

    server_addr.sin_family =
        AF_INET;

    server_addr.sin_port =
        htons(8080);

    server_addr.sin_addr.s_addr =
        INADDR_ANY;


    if (
        bind(
            server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0
    ) {

        std::cerr
            << "Bind failed.\n";

        close(server_fd);

        return 1;
    }


    if (
        listen(
            server_fd,
            10
        ) < 0
    ) {

        std::cerr
            << "Listen failed.\n";

        close(server_fd);

        return 1;
    }


    std::cout
        << "NetChat Server started "
        << "on port 8080...\n";


    while (true) {

        sockaddr_in client_addr{};

        socklen_t client_size =
            sizeof(client_addr);


        int client_fd =
            accept(
                server_fd,
                (struct sockaddr *)&client_addr,
                &client_size
            );


        if (client_fd >= 0) {

            std::cout
                << "New client connecting...\n";

            std::thread(
                handle_client,
                client_fd
            ).detach();
        }
    }


    close(server_fd);

    return 0;
}