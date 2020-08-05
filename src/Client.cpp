#include "Networking.cpp"

void networking_startup() {
  initialize();
  log_("successfully initialized winsock!\n");

  uint64 socket = create_socket();
  log_("successfully created socket!\n");

  connect_socket(socket, create_ip_endpoint("127.0.0.1", 4790));
  log_("socket is successfully connected!\n");

  close_socket(socket);
  log_("successfully closed socket!\n");

  shutdown();
}
