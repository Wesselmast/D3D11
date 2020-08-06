#include "Networking.cpp"

static IPEndPoint serverEndPoint;
static uint64 serverSocket;

void server_startup() {
  initialize();
  log_("successfully initialized winsock!\n");

  serverEndPoint = create_ip_endpoint("::", 4790);

  serverSocket = create_socket(serverEndPoint.ipversion);
  log_("successfully created socket!\n");

  socket_listen(serverSocket, serverEndPoint, 5);
  log_("socket is successfully listening!\n");
}

void server_update() {
  uint64 newSocket;
  IPEndPoint newEndPoint = accept_socket(serverSocket, newSocket, serverEndPoint.ipversion);
  log_("socket has accepted a new connection!\n");
  print_ip_endpoint(newEndPoint);

  close_socket(newSocket);
  log_("the new connection was closed!\n");

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
  close_socket(serverSocket);
  log_("successfully closed socket!\n");
  
  shutdown();
}
