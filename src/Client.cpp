static Connection client;

void client_connect(const IPEndPoint& ipEndPoint) {
  initialize();
  log_("successfully initialized winsock!\n");

  client.ipEndPoint = ipEndPoint;

  client.socket = create_socket(client.ipEndPoint.ipversion);
  log_("successfully created socket!\n");

  set_socket_blocking(client.socket, 1); //temporarily setting to blocking sockets
  log_("nonblocking!\n");

  attempt_connection(client);
  log_("client is successfully connected!\n");
}

bool32 client_update(GameState* state) {
  {
    Packet packet;
    if(!socket_recieve_packet(client.socket, packet)) {
      log_("Looks like we lost connection D:\n");
      return 0;
    }
    
    char buf[256];
    packet_extract(packet, buf); 
//    log_("%s\n", buf);
  }

  {
    Packet packet;
    packet_insert(packet, state->playerTransform.position);
    packet_insert(packet, state->playerTransform.rotation);
    packet_insert(packet, state->playerTransform.scale);
    if(!socket_send_packet(client.socket, packet)) return 0;
  }
  return 1;
}

void client_disconnect() {
  close_connection(client);
  log_("successfully closed socket!\n");

  shutdown();
}
