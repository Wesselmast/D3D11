#include "Networking.cpp"

void networking_startup() {
  initialize();
  log_("successfully initialized winsock!\n");

  uint64 socket = create_socket();
  log_("successfully created socket!\n");

  socket_listen(socket, create_ip_endpoint("0.0.0.0", 4790), 5);
  log_("socket is successfully listening!\n");

  uint64 newSocket;
  accept_socket(socket, newSocket);
  log_("socket has accepted a new connection!\n");


  char buf[256];
  while(1) {
    uint32 len;
    if(!socket_recieve(newSocket, &len, sizeof(uint32))) break;

    len = ntohl(len);
    assert_(len < MAX_PACKET_SIZE, "Tried to send a package that exceeds the max packet size!");

    if(!socket_recieve(newSocket, &buf[0], len)) break;
    log_("%s\n", buf);
  }

  close_socket(newSocket);
  log_("the new connection was closed!\n");

  close_socket(socket);
  log_("successfully closed socket!\n");

  shutdown();
}
