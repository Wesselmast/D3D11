#pragma once

void disconnect_client(Server* server, RenderObjects* ro, uint32 client) {
  Packet packet;
  packet_insert(packet, PacketType::SERVER_DISCONNECT);
  socket_send_packet(server->connections[client].socket, packet);

  close_connection(server->connections[client]);
  destroy_object(ro, server->players[client]);
}

void server_startup(Server* server, const IPEndPoint& ipEndPoint) {
  Connection& c = server->connections[0]; 
  WSAPOLLFD& desc = server->descriptors[0];

  c.ipEndPoint = ipEndPoint;

  c.socket = create_socket(c.ipEndPoint.ipversion);
  log_("successfully created socket!\n");

  listen_connection(c, 5);
  log_("connection is successfully listening!\n");

  desc.fd = c.socket;
  desc.events = POLLRDNORM;
  desc.revents = 0;

  server->valid = true;
}

uint32 server_update(GameState* state) {
  Server* server = &(state->server);
  RenderObjects* ro = &(state->renderObjects);
  server->players[0] = state->player;
  
  WSAPOLLFD tempDescs[MAX_NUMBER_OF_CONNECTIONS];
  memcpy(tempDescs, server->descriptors, sizeof(WSAPOLLFD) * MAX_NUMBER_OF_CONNECTIONS);
  
  if(WSAPoll(tempDescs, server->count, 1) > 0) {
    
    { //SERVER SOCKET CODE
      WSAPOLLFD& socketDesc = tempDescs[0];
      if(socketDesc.revents & POLLRDNORM) {
	Connection newConnection;
	if(!accept_connection(server->connections[0], newConnection)) return 0;
	log_("socket has accepted a new connection!\n");
	
	uint32 index = 1;
	while(server->connections[index].valid) {
	  index++;
	  if(index > MAX_NUMBER_OF_CONNECTIONS) {
	    log_("can't accept another connection!\n");
	    close_connection(newConnection);
	    return 0;
	  }  
	}

	WSAPOLLFD newSocketDesc = {};
	newSocketDesc.fd = newConnection.socket;
	newSocketDesc.events = POLLRDNORM | POLLWRNORM;
	newSocketDesc.revents = 0;
		
	server->descriptors[index] = newSocketDesc;
	server->connections[index] = newConnection;
	server->players[index] = create_player(ro, models, true);
	server->count += (index == server->count);
	print_ip_endpoint(newConnection.ipEndPoint);

	{
	  for(uint32 i = 0; i < server->count; i++) {
	    if(i == index) continue;
	    Packet packet;
	    packet_insert(packet, PacketType::NEW_CONNECTION);
	    packet_insert(packet, i);
	    if(!socket_send_packet(newConnection.socket, packet)) {
	      log_("Connection lost port %d, Closing..\n", newConnection.ipEndPoint.port);
	      disconnect_client(server, ro, index);
	      return 0;
	    }
	  }
	}
	
	log_("INDEX = %d, COUNT = %d\n", index, server->count);

	{
	  for(uint32 i = 1; i < server->count; i++) {
	    if(i == index) continue;
	    Connection& c = server->connections[i];
	    if(!c.valid) continue;
	    Packet packet;
	    packet_insert(packet, PacketType::NEW_CONNECTION);
	    packet_insert(packet, index);
	    
	    if(!socket_send_packet(c.socket, packet)) {
	      log_("Connection lost port %d, Closing..\n", c.ipEndPoint.port);
	      disconnect_client(server, ro, i);
	      return 0;
	    }
	  }
	}
      }
    }

    for(uint32 i = 1; i < server->count; i++) {
      Connection& c = server->connections[i];
      if(!c.valid) continue;
      if(tempDescs[i].revents & POLLERR) {
	log_("Poll error on port %d! Closing..\n", c.ipEndPoint.port);
	disconnect_client(server, ro, i);
	continue;
      }
      if(tempDescs[i].revents & POLLHUP) {
	log_("Poll hangup on port %d! Closing..\n", c.ipEndPoint.port);
	disconnect_client(server, ro, i);
	continue;
      }
      // if(tempDescs[i].revents & POLLNVAL) {
      // 	log_("Invalid socket on port %d! Closing..\n", c.ipEndPoint.port);
      // 	server->connections[i] = {};
      // 	server->descriptors[i] = {};
      // 	close_connection(c);
      // 	continue;
      // }

      if(tempDescs[i].revents & POLLRDNORM) {
	Packet packet;
	if(!socket_recieve_packet(c.socket, packet)) {
	  log_("Connection lost port %d, Closing..\n", c.ipEndPoint.port);
	  disconnect_client(server, ro, i);
	  continue;
	}

	PacketType type;
	packet_extract(packet, type);

	switch(type) {
	case PacketType::PLAYER_TRANSFORM: {
	  Transform t;
	  packet_extract(packet, t.position); 
	  packet_extract(packet, t.rotation); 
	  set_object_transform(ro, server->players[i], t);
	  break;
	}
	case PacketType::BULLET_SPAWN: {
	  BulletDesc bDesc = {};
	  packet_extract(packet, bDesc.startPos);
	  packet_extract(packet, bDesc.direction);
	  packet_extract(packet, bDesc.speed);
	  packet_extract(packet, bDesc.lifeTime);
	  bDesc.enemyBullet = true;
	  bDesc.owner = server->players[i];
	  state->bullets.push_back(spawn_bullet(ro, models, bDesc));
	  break;
	}
	default: log_("Sent package has no server-side implementation!");
	}
      }

      if(tempDescs[i].revents & POLLWRNORM) {
	bool failed = false;
	for(uint32 j = 0; j < server->count; j++) {
	  if(j == i) continue;
	  Packet packet;
	  Transform transform = get_object_transform(ro, server->players[j]);
	  packet_insert(packet, PacketType::PLAYER_TRANSFORM);
	  packet_insert(packet, j);
	  packet_insert(packet, transform.position);
	  packet_insert(packet, transform.rotation);
	  
	  if(!socket_send_packet(c.socket, packet)) {
	    failed = true;
	    break;
	  }
	}
	if(failed) continue;
      }

	// char buf[MAX_PACKET_SIZE];
	// uint32 bytesRecieved = 0;
	// bytesRecieved = recv(tempDescs[i].fd, buf, MAX_PACKET_SIZE, 0);
	// if(bytesRecieved == 0) {
	//   log_("Connection lost port %d, v0, Closing..\n", c.ipEndPoint.port);
	//   close_connection(c);
	//   continue;
	// }
	// if(bytesRecieved == SOCKET_ERROR) {
	//   int32 error = WSAGetLastError();
	//   if(error != WSAEWOULDBLOCK) {
	//     log_("Connection lost port %d, v1, Closing..\n", c.ipEndPoint.port);
	//     close_connection(c);
	//     log_("connection %d was closed!\n", i);
	//     continue;	    
	//   }
	// }
	  //log_("recieved message from %d of size %d\n", c.ipEndPoint.port, bytesRecieved);
    }    
  }
  


  // Packet packet;
  // while(1) {
  //   if(!socket_recieve_packet(newSocket, packet)) break;
    
  //   uint32 a, b, c;
  //   char buf[256];
  //   packet_extract(packet, a); 
  //   packet_extract(packet, b); 
  //   packet_extract(packet, c); 
  //   packet_extract(packet, buf); 
  //   log_("%d, %d, %d, %s\n", a, b, c, buf);
  // }
  return 0;
}

void server_shutdown(Server* server, RenderObjects* ro) {
  for(uint32 i = 1; i < server->count; i++) {
    Connection& c = server->connections[i];
    if(!c.valid) continue;
    disconnect_client(server, ro, i);
  }
  close_connection(server->connections[0]);
  server->valid = false;
}

void server_send_bullet(Server* server, RenderObjects* ro, const BulletDesc& desc) {
  for(uint32 i = 1; i < server->count; i++) {
    Connection& c = server->connections[i];
    if(!c.valid) continue;
    
    for(uint32 j = 0; j < server->count; j++) {
      if(j == i) continue;
      Packet packet;
      Transform transform = get_object_transform(ro, server->players[j]);
      packet_insert(packet, PacketType::BULLET_SPAWN);
      packet_insert(packet, j);
      packet_insert(packet, desc.startPos);
      packet_insert(packet, desc.direction);
      packet_insert(packet, desc.speed);
      packet_insert(packet, desc.lifeTime);
      
      if(!socket_send_packet(c.socket, packet)) {
	disconnect_client(server, ro, i);
	break;
      }
    }
  }
}
