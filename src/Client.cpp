static Connection client;

static uint32 otherPlayers[MAX_NUMBER_OF_CONNECTIONS];

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
  RenderObjects* ro = &(state->renderObjects);
  {
    Packet packet;
    if(!socket_recieve_packet(client.socket, packet)) {
      log_("Looks like we lost connection D:\n");
      return 0;
    }

    PacketType type;
    packet_extract(packet, type);
    
    switch(type) {
    case PacketType::PLAYER_TRANSFORM: {
      Transform t;
      uint32 index;
      packet_extract(packet, index);
      packet_extract(packet, t.position); 
      packet_extract(packet, t.rotation); 
      set_object_transform(ro, otherPlayers[index], t);
      break;
    }
    case PacketType::NEW_CONNECTION: {
      uint32 newConnection;
      packet_extract(packet, newConnection);
      otherPlayers[newConnection] = create_player(ro, models, true);
      break;
    }
    default: log_("Sent package has no server-side implementation!");
    }
  }
  {
    Packet packet;
    packet_insert(packet, PacketType::PLAYER_TRANSFORM);
    packet_insert(packet, state->playerTransform.position);
    packet_insert(packet, state->playerTransform.rotation);
    if(!socket_send_packet(client.socket, packet)) return 0;
  }
  return 1;
}

void client_disconnect() {
  close_connection(client);
  log_("successfully closed socket!\n");

  shutdown();
}
