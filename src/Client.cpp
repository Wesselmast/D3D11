#pragma once

void client_connect(Client* client, const IPEndPoint& ipEndPoint) {
  Connection& c = client->connection;

  c.ipEndPoint = ipEndPoint;

  c.socket = create_socket(c.ipEndPoint.ipversion);
  log_("successfully created socket!\n");

  set_socket_blocking(c.socket, 1); //temporarily setting to blocking sockets
  log_("nonblocking!\n");

  attempt_connection(c);
  log_("client is successfully connected!\n");

  client->otherPlayerCount = 0;
  client->valid = true;
}

void client_disconnect(Client* client, RenderObjects* ro) {
  for(uint32 i = 0; i < client->otherPlayerCount; i++) {
    destroy_object(ro, client->otherPlayers[i]);
  }
  client->otherPlayerCount = 0;
  close_connection(client->connection);
  client->valid = false;
}

bool32 client_update(GameState* state) {
  Client* client = &(state->client); 
  RenderObjects* ro = &(state->renderObjects);

  {
    Packet packet;
    if(!socket_recieve_packet(client->connection.socket, packet)) {
      log_("Looks like we lost connection D:\n"); 
      client_disconnect(client, ro);      
      return 1;
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
      set_object_transform(ro, client->otherPlayers[index], t);
      break;
    }
    case PacketType::NEW_CONNECTION: {
      uint32 newConnection;
      packet_extract(packet, newConnection);
      client->otherPlayers[newConnection] = create_player(ro, models, true);
      client->otherPlayerCount++;
      break;
    }
    case PacketType::SERVER_DISCONNECT: {
      log_("wadwd");
      client_disconnect(client, ro);      
      return 1;
    }
    default: log_("Sent package has no server-side implementation!");
    }
  }
  {
    Packet packet;
    packet_insert(packet, PacketType::PLAYER_TRANSFORM);
    packet_insert(packet, state->playerTransform.position);
    packet_insert(packet, state->playerTransform.rotation);
    if(!socket_send_packet(client->connection.socket, packet)) {
      client_disconnect(client, ro);      
      return 1;
    }
  }

  return 0;
}
