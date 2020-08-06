#include "Networking.cpp"

static uint64 clientSocket;

void client_connect() {
  initialize();
  log_("successfully initialized winsock!\n");

  IPEndPoint clientEndPoint = create_ip_endpoint("::1", 4790);

  clientSocket = create_socket(clientEndPoint.ipversion);
  log_("successfully created socket!\n");

  connect_socket(clientSocket, clientEndPoint);
  log_("socket is successfully connected!\n");
}

bool32 client_update() {
  Packet packet;
  packet_insert(packet, 4);
  packet_insert(packet, 2);
  packet_insert(packet, 9);
  packet_insert(packet, "Pleb");

  if(!socket_send_packet(clientSocket, packet)) return 0;
  
  log_("trying to send some data\n");
  Sleep(500);
  return 1;
}

void client_disconnect() {
  close_socket(clientSocket);
  log_("successfully closed socket!\n");

  shutdown();
}
