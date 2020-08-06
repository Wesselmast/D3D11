#include "Networking.cpp"

const uint32 MAX_NUMBER_OF_CONNECTIONS = 4;

struct Bridge {
  Connection connections[MAX_NUMBER_OF_CONNECTIONS];
  WSAPOLLFD  descriptors[MAX_NUMBER_OF_CONNECTIONS];
  uint32 count = 1;
};

static Bridge serverBridge;

void server_startup() {
  initialize();
  log_("successfully initialized winsock!\n");
  
  Connection server = {};
  server.ipEndPoint = create_ip_endpoint("::", 4790);

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
}

void server_update() {
  WSAPOLLFD tempDescs[MAX_NUMBER_OF_CONNECTIONS];
  Connection& server = serverBridge.connections[0];
  memcpy(tempDescs, serverBridge.descriptors, sizeof(WSAPOLLFD) * MAX_NUMBER_OF_CONNECTIONS);
  
  if(WSAPoll(tempDescs, serverBridge.count, 1) > 0) {
    
    { //SERVER SOCKET CODE
      WSAPOLLFD& socketDesc = tempDescs[0];
      if(socketDesc.revents & POLLRDNORM) {
	Connection newConnection;
	if(!accept_connection(server, newConnection)) return;
	log_("socket has accepted a new connection!\n");
	
	uint32 index = 1;
	while(serverBridge.connections[index].valid) {
	  index++;
	  if(index > MAX_NUMBER_OF_CONNECTIONS) {
	    log_("can't accept another connection!\n");
	    close_connection(newConnection);
	    return;
	  }  
	}

	WSAPOLLFD newSocketDesc = {};
	newSocketDesc.fd = newConnection.socket;
	newSocketDesc.events = POLLRDNORM;
	newSocketDesc.revents = 0;
	
	serverBridge.descriptors[index] = newSocketDesc;
	serverBridge.connections[index] = newConnection;
	serverBridge.count = index + (index == serverBridge.count); 
	print_ip_endpoint(newConnection.ipEndPoint);
      }
    }

    for(uint32 i = 1; i < serverBridge.count; i++) {
      Connection& c = serverBridge.connections[i];
      if(!c.valid) continue;
      if(tempDescs[i].revents & POLLERR) {
	log_("Poll error on port %d", c.ipEndPoint.port);
	close_connection(c);
	serverBridge.connections[i] = {};
	serverBridge.descriptors[i] = {};
	log_("connection %d was closed!\n", i);
	continue;
      }
      if(tempDescs[i].revents & POLLHUP) {
	log_("Poll hangup on port %d", c.ipEndPoint.port);
	close_connection(c);
	serverBridge.connections[i] = {};
	serverBridge.descriptors[i] = {};
	log_("connection %d was closed!\n", i);
	continue;
      }
      if(tempDescs[i].revents & POLLNVAL) {
      	log_("Invalid socket on port %d", c.ipEndPoint.port);
      	close_connection(c);
      	serverBridge.connections[i] = {};
      	serverBridge.descriptors[i] = {};
      	log_("connection was closed!\n");
      	continue;
      }

      if(tempDescs[i].revents & POLLRDNORM) {
	char buf[MAX_PACKET_SIZE];
	uint32 bytesRecieved = 0;
	bytesRecieved = recv(tempDescs[i].fd, buf, MAX_PACKET_SIZE, 0);
	if(bytesRecieved == 0) {
	  log_("Connection lost port %d, v0", c.ipEndPoint.port);
	  close_connection(c);
	  serverBridge.connections[i] = {};
	  serverBridge.descriptors[i] = {};
	  log_("connection %d was closed!\n", i);
	  continue;
	}
	if(bytesRecieved == SOCKET_ERROR) {
	  int32 error = WSAGetLastError();
	  if(error != WSAEWOULDBLOCK) {
	    log_("Connection lost port %d, v1", c.ipEndPoint.port);
	    close_connection(c);
	    serverBridge.connections[i] = {};
	    serverBridge.descriptors[i] = {};
	    log_("connection %d was closed!\n", i);
	    continue;	    
	  }
	}
	if(bytesRecieved > 0) {
	  //log_("recieved message from %d of size %d\n", c.ipEndPoint.port, bytesRecieved);
	}
      }
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
}

void server_shutdown() {
  close_connection(serverBridge.connections[0]);
  shutdown();
}
