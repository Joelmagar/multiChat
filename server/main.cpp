#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

std::vector<int> clients;
std::mutex clients_mutex;

void broadcast_message(const std::string &message, int sender_fd) {
  std::lock_guard<std::mutex> lock(clients_mutex);
  for (int client_fd : clients) {
    if (client_fd != sender_fd) {
      send(client_fd, message.c_str(), message.length(), 0);
    }
  }
}

void handle_client(int client_fd) {
  char buffer[1024];
  while (true) {
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
      std::cout << "Client disconnected: " << client_fd << "\n";
      std::lock_guard<std::mutex> lock(clients_mutex);
      clients.erase(std::remove(clients.begin(), clients.end(), client_fd),
                    clients.end());
      close(client_fd);
      break;
    }
    std::string message(buffer);
    std::cout << "Received: " << message << std::endl;
    broadcast_message(message, client_fd);
  }
}

int main() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  listen(server_fd, 10);
  std::cout << "Chat Server started on port 8080...\n";
  while (true) {
    sockaddr_in client_addr{};
    socklen_t client_size = sizeof(client_addr);
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_size);
    if (client_fd >= 0) {
      std::cout << "New client connected!\n";
      {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_fd);
      }
      std::thread(handle_client, client_fd).detach();
    }
  }
  close(server_fd);
  return 0;
}