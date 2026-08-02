#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

void receive_messages(int socket_fd) {
  char buffer[1024];
  while (true) {
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
      std::cout << "\nDisconnected from server.\n";
      exit(0);
    }
    std::cout << "\n" << buffer << "\n> " << std::flush;
  }
}

int main() {
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1",
            &server_addr.sin_addr); // Connect to localhost

  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    std::cerr << "Connection to server failed!\n";
    return 1;
  }

  std::cout << "Connected to server! Enter your name: ";
  std::string name;
  std::getline(std::cin, name);

  // Spin up a thread specifically to watch for incoming texts
  std::thread(receive_messages, client_fd).detach();

  std::string input;
  while (true) {
    std::cout << "> ";
    std::getline(std::cin, input);
    if (input == "exit")
      break;

    if (!input.empty()) {
      std::string full_message = name + ": " + input;
      send(client_fd, full_message.c_str(), full_message.length(), 0);
    }
  }

  close(client_fd);
  return 0;
}
