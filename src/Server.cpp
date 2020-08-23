const uint32 MAX_NUMBER_OF_CONNECTIONS = 4;

struct Bridge {
  Connection connections[MAX_NUMBER_OF_CONNECTIONS];
  WSAPOLLFD  descriptors[MAX_NUMBER_OF_CONNECTIONS];
  uint32 players[MAX_NUMBER_OF_CONNECTIONS];
  uint32 count = 1;
};

static Bridge serverBridge;

void disconnect_client(RenderObjects* ro, uint32 client) {
  Packet packet;
  packet_insert(packet, PacketType::SERVER_DISCONNECT);
  socket_send_packet(serverBridge.connections[client].socket, packet);

  close_connection(serverBridge.connections[client]);
  destroy_object(ro, serverBridge.players[client]);
}

void server_startup(const IPEndPoint& ipEndPoint) {
  log_("successfully initialized winsock!\n");
  
  Connection server = {};
  server.ipEndPoint = ipEndPoint;

  server.socket = create_socket(server.ipEndPoint.ipversion);
  log_("successfully created socket!\n");

  listen_connection(server, 5);
  log_("connection is successfully listening!\n");

  WSAPOLLFD socketDesc = {};
  socketDesc.fd = server.socket;
  socketDesc.events = POLLRDNORM;
  socketDesc.revents = 0;
  
  serverBridge.descriptors[0] = socketDesc;
  serverBridge.connections[0] = server;

  connected = true;
}

uint32 server_update(GameState* state) {
  RenderObjects* ro = &(state->renderObjects);
  serverBridge.players[0] = state->player;
  
  WSAPOLLFD tempDescs[MAX_NUMBER_OF_CONNECTIONS];
  Connection& server = serverBridge.connections[0];
  memcpy(tempDescs, serverBridge.descriptors, sizeof(WSAPOLLFD) * MAX_NUMBER_OF_CONNECTIONS);
  
  if(WSAPoll(tempDescs, serverBridge.count, 1) > 0) {
    
    { //SERVER SOCKET CODE
      WSAPOLLFD& socketDesc = tempDescs[0];
      if(socketDesc.revents & POLLRDNORM) {
	Connection newConnection;
	if(!accept_connection(server, newConnection)) return 0;
	log_("socket has accepted a new connection!\n");
	
	uint32 index = 1;
	while(serverBridge.connections[index].valid) {
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
		
	serverBridge.descriptors[index] = newSocketDesc;
	serverBridge.connections[index] = newConnection;
	serverBridge.players[index] = create_player(ro, models, true);
	serverBridge.count += (index == serverBridge.count);
	print_ip_endpoint(newConnection.ipEndPoint);

	{
	  for(uint32 i = 0; i < serverBridge.count; i++) {
	    if(i == index) continue;
	    Packet packet;
	    packet_insert(packet, PacketType::NEW_CONNECTION);
	    packet_insert(packet, i);
	    if(!socket_send_packet(newConnection.socket, packet)) {
	      log_("Connection lost port %d, Closing..\n", newConnection.ipEndPoint.port);
	      disconnect_client(ro, index);
	      return 0;
	    }
	  }
	}
	
	log_("INDEX = %d, COUNT = %d\n", index, serverBridge.count);

	{
	  for(uint32 i = 1; i < serverBridge.count; i++) {
	    if(i == index) continue;
	    Connection& c = serverBridge.connections[i];
	    if(!c.valid) continue;
	    Packet packet;
	    packet_insert(packet, PacketType::NEW_CONNECTION);
	    packet_insert(packet, index);
	    
	    if(!socket_send_packet(c.socket, packet)) {
	      log_("Connection lost port %d, Closing..\n", c.ipEndPoint.port);
	      disconnect_client(ro, i);
	      return 0;
	    }
	  }
	}
      }
    }

    for(uint32 i = 1; i < serverBridge.count; i++) {
      Connection& c = serverBridge.connections[i];
      if(!c.valid) continue;
      if(tempDescs[i].revents & POLLERR) {
	log_("Poll error on port %d! Closing..\n", c.ipEndPoint.port);
	disconnect_client(ro, i);
	continue;
      }
      if(tempDescs[i].revents & POLLHUP) {
	log_("Poll hangup on port %d! Closing..\n", c.ipEndPoint.port);
	disconnect_client(ro, i);
	continue;
      }
      // if(tempDescs[i].revents & POLLNVAL) {
      // 	log_("Invalid socket on port %d! Closing..\n", c.ipEndPoint.port);
      // 	serverBridge.connections[i] = {};
      // 	serverBridge.descriptors[i] = {};
      // 	close_connection(c);
      // 	continue;
      // }

      if(tempDescs[i].revents & POLLRDNORM) {
	Packet packet;
	if(!socket_recieve_packet(c.socket, packet)) {
	  log_("Connection lost port %d, Closing..\n", c.ipEndPoint.port);
	  disconnect_client(ro, i);
	  continue;
	}

	PacketType type;
	packet_extract(packet, type);

	switch(type) {
	case PacketType::PLAYER_TRANSFORM: {
	  Transform t;
	  packet_extract(packet, t.position); 
	  packet_extract(packet, t.rotation); 
	  set_object_transform(ro, serverBridge.players[i], t);
	  break;
	}
	default: log_("Sent package has no server-side implementation!");
	}
      }

      if(tempDescs[i].revents & POLLWRNORM) {
	bool failed = false;
	for(uint32 j = 0; j < serverBridge.count; j++) {
	  if(j == i) continue;
	  Packet packet;
	  Transform transform = get_object_transform(ro, serverBridge.players[j]);
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

void server_shutdown(RenderObjects* ro) {
  for(uint32 i = 1; i < serverBridge.count; i++) {
    Connection& c = serverBridge.connections[i];
    if(!c.valid) continue;
    disconnect_client(ro, i);
  }
  close_connection(serverBridge.connections[0]);
  connected = false;
}
