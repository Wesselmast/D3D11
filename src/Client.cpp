#include "Networking.cpp"

static Connection client;

void client_connect() {
  initialize();
  log_("successfully initialized winsock!\n");

  client.ipEndPoint = create_ip_endpoint("::1", 4790);

  client.socket = create_socket(client.ipEndPoint.ipversion);
  log_("successfully created socket!\n");

  set_socket_blocking(client.socket, 1); //temporarily setting to blocking sockets
  log_("nonblocking!\n");

  attempt_connection(client);
  log_("client is successfully connected!\n");
}

bool32 client_update() {
  Packet packet;
  packet_insert(packet, 4);
  packet_insert(packet, 2);
  packet_insert(packet, 9);
  packet_insert(packet, "Pleb");

  if(!socket_send_packet(client.socket, packet)) return 0;
  
  log_("trying to send some data\n");
  return 1;
}

void client_disconnect() {
  close_connection(client);
  log_("successfully closed socket!\n");

  shutdown();
}
