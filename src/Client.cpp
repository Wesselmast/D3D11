#include "Networking.cpp"

void networking_startup() {
  initialize();
  log_("successfully initialized winsock!\n");

  uint64 socket = create_socket();
  log_("successfully created socket!\n");

  connect_socket(socket, create_ip_endpoint("127.0.0.1", 4790));
  log_("socket is successfully connected!\n");

  const uint32 len = 256;
  char buf[len];
  strcpy(buf, "The client says hello!\0");
  while(1) {
    uint32 nLen = htonl(len); 
    if(!socket_send(socket, &nLen, sizeof(uint32))) break;
    if(!socket_send(socket, &buf, len)) break;
    
    log_("trying to send some data\n");
    Sleep(1);
  }

  close_socket(socket);
  log_("successfully closed socket!\n");

  shutdown();
}
