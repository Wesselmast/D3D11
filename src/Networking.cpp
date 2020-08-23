#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>

static bool32 connected; //I should probably get rid of this sometime soon

#define wsa_fail(errorcode) assert_(false, "WSA ERROR: %d", errorcode);

#include "Packet.cpp"

enum SocketOption {
   TCP_NO_DELAY,
   IPV6_ONLY
};

enum IPVersion {
   IP_NONE,
   IPV4,
   IPV6
};

struct IPEndPoint {
  char ip[16];
  uint8 ipdata[16];
  uint16 port;
  IPVersion ipversion = IPVersion::IP_NONE;
};

struct Connection {
  uint64 socket; 
  IPEndPoint ipEndPoint; 
  bool32 valid = 0;
};

void print_bytes(const IPEndPoint& endPoint) {
  uint8 length;
  switch(endPoint.ipversion) {
  case IPVersion::IPV4: length = 4;  break;
  case IPVersion::IPV6: length = 16; break;
  default: return; 
  }
  
  for(uint32 i = 0; i < length; i++) {
    log_("%d", (uint32)endPoint.ipdata[i]);
    if(i < length - 1) log_(".");
  }
  log_("\n");
}

void print_ip_endpoint(const IPEndPoint& endPoint) {
  log_("ip endpoint info: \n");
  switch(endPoint.ipversion) {
  case IPVersion::IPV4: log_("   ipv4\n"); break;
  case IPVersion::IPV6: log_("   ipv6\n"); break;
  default: log_("   unknown"); 
  }
  log_("   ip:   %s\n", endPoint.ip);
  log_("   data: ");
  print_bytes(endPoint);
  log_("   port: %d\n", endPoint.port);
}

void initialize() {
  WSADATA data;
  int32 result = WSAStartup(MAKEWORD(2, 2), &data);
  assert_(!result, "WSA startup failed");
  assert_(LOBYTE(data.wVersion) == 2 || 
	 HIBYTE(data.wVersion) == 2, "Could not find a usable version of WSA");
}

void shutdown() {
  WSACleanup();
}

IPEndPoint create_ip_endpoint(const char* ip, uint16 port) {
  IPEndPoint ipEndPoint = {}; 
  ipEndPoint.port = port;
  
  int32 result;

  {
    in_addr addr;
    result = inet_pton(AF_INET, ip, &addr);
    
    if(result == 1) {
      strcpy(ipEndPoint.ip, ip);
      memcpy(&ipEndPoint.ipdata[0], &addr.S_un.S_addr, 4); 
      ipEndPoint.ipversion = IPVersion::IPV4;
    }
  }
  if(result == 1) return ipEndPoint;

  {
    in6_addr addr;
    result = inet_pton(AF_INET6, ip, &addr);

    if(result == 1) {
      strcpy(ipEndPoint.ip, ip);
      memcpy(&ipEndPoint.ipdata[0], &addr.u, 16); 
      ipEndPoint.ipversion = IPVersion::IPV6;
    }
  }

  if(result == 0) {
    assert_(false, "coulnd't create ip endpoint, entered invalid dotted decimal string!");
  }
  else if(result < 0) {
    wsa_fail(WSAGetLastError());
  }
  return ipEndPoint;
}

IPEndPoint create_ip_endpoint(sockaddr* addr) {
  IPEndPoint ipEndPoint = {};
  
  if(addr->sa_family == AF_INET) {
    sockaddr_in* addr4 = (sockaddr_in*)(addr);
    ipEndPoint.ipversion = IPVersion::IPV4;
    ipEndPoint.port = ntohs(addr4->sin_port);
    memcpy(&ipEndPoint.ipdata[0], &addr4->sin_addr, 4);
    inet_ntop(AF_INET, &addr4->sin_addr, &ipEndPoint.ip[0], 16);
  }
  else if(addr->sa_family == AF_INET6){
    sockaddr_in6* addr6 = (sockaddr_in6*)(addr);
    ipEndPoint.ipversion = IPVersion::IPV6;
    ipEndPoint.port = ntohs(addr6->sin6_port);
    memcpy(&ipEndPoint.ipdata[0], &addr6->sin6_addr, 16);
    inet_ntop(AF_INET6, &addr6->sin6_addr, &ipEndPoint.ip[0], 46);
  }
  return ipEndPoint;
}

void set_socket_blocking(uint64& socket, bool32 block) {
  unsigned long blocking    = 0;
  unsigned long nonblocking = 1;
  int32 result = ioctlsocket(socket, FIONBIO, block ? &blocking : &nonblocking);

  if(result == SOCKET_ERROR) {
    wsa_fail(WSAGetLastError());
  }
}

void set_socket_option(uint64& socket, SocketOption option, bool32 value) {
  int32 result;
  switch(option) {
  case SocketOption::TCP_NO_DELAY: {
    result = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(bool32));
    break;
  }
  case SocketOption::IPV6_ONLY: {
    result = setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(bool32));
    break;
  } 
  default:
    assert_(false, "socket option hasn't been implemented yet!");
  }
  if(result) {
    wsa_fail(WSAGetLastError());
  }
}

uint32 create_socket(IPVersion version) {
  uint64 sresult;
  
  switch(version) {
  case IPVersion::IPV4: sresult = socket(AF_INET,  SOCK_STREAM, IPPROTO_TCP); break;
  case IPVersion::IPV6: sresult = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP); break;
  default: return 0;
  }
  
  if(sresult == INVALID_SOCKET) {
    wsa_fail(WSAGetLastError());
    return 0;
  }
  
  set_socket_blocking(sresult, 0);
  set_socket_option(sresult, SocketOption::TCP_NO_DELAY, 1);
  return sresult;
}

void close_socket(uint64& socket) {
  closesocket(socket);
  socket = INVALID_SOCKET;
}

void close_connection(Connection& connection) {
  close_socket(connection.socket);
  connection.socket = 0;
  connection.ipEndPoint = {};
  connection.valid = 0;
}

void bind_socket(uint64& socket, const IPEndPoint& ipEndPoint) {
  if(ipEndPoint.ipversion == IPVersion::IPV4) {
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, &(ipEndPoint.ipdata)[0], sizeof(ULONG));
    addr.sin_port = htons(ipEndPoint.port);
    
    int32 result = bind(socket, (sockaddr*)&addr, sizeof(sockaddr_in));
    if(result) {
      wsa_fail(WSAGetLastError());
    }
  }
  else if(ipEndPoint.ipversion == IPVersion::IPV6) {
    sockaddr_in6 addr = {};
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &(ipEndPoint.ipdata)[0], 16);
    addr.sin6_port = htons(ipEndPoint.port);
    
    int32 result = bind(socket, (sockaddr*)&addr, sizeof(sockaddr_in6));
    if(result) {
      wsa_fail(WSAGetLastError());
    }
  }
}

void listen_connection(Connection& c, int32 backlog) {
  if(c.ipEndPoint.ipversion == IPVersion::IPV6) {
    set_socket_option(c.socket, SocketOption::IPV6_ONLY, 0);
  }
  
  bind_socket(c.socket, c.ipEndPoint);
  int32 result = listen(c.socket, backlog); 
  if(result) {
    wsa_fail(WSAGetLastError());
  }
}

bool32 accept_connection(Connection& inC, Connection& outC) {
  if(inC.ipEndPoint.ipversion == IPVersion::IPV4) {
    sockaddr_in addr = {};
    int32 len = sizeof(sockaddr_in);
    
    outC.socket = accept(inC.socket, (sockaddr*)&addr, &len);
    if(outC.socket == INVALID_SOCKET) {
      wsa_fail(WSAGetLastError());
      return 0;
    }

    outC.ipEndPoint = create_ip_endpoint((sockaddr*)&addr);
    outC.valid = 1;
    return 1;
  }
  else if(inC.ipEndPoint.ipversion == IPVersion::IPV6) {
    sockaddr_in6 addr = {};
    int32 len = sizeof(sockaddr_in6);
    
    outC.socket = accept(inC.socket, (sockaddr*)&addr, &len);
    if(outC.socket == INVALID_SOCKET) {
      wsa_fail(WSAGetLastError());
      return 0;
    }

    outC.ipEndPoint = create_ip_endpoint((sockaddr*)&addr);
    outC.valid = 1;
    return 1; 
  }

  return 0;
}

void attempt_connection(Connection& c) {
  if(c.ipEndPoint.ipversion == IPVersion::IPV4) {
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, &(c.ipEndPoint.ipdata)[0], sizeof(ULONG));
    addr.sin_port = htons(c.ipEndPoint.port);
    
    int32 result = connect(c.socket, (sockaddr*)&addr, sizeof(sockaddr_in));
    if(result) {
      wsa_fail(WSAGetLastError());
    }
  }
  else if(c.ipEndPoint.ipversion == IPVersion::IPV6) {
    sockaddr_in6 addr = {};
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &(c.ipEndPoint.ipdata)[0], 16);
    addr.sin6_port = htons(c.ipEndPoint.port);
    
    int32 result = connect(c.socket, (sockaddr*)&addr, sizeof(sockaddr_in6));
    if(result) {
      wsa_fail(WSAGetLastError());
    }
  }
}

bool32 socket_send(uint64& socket, const void* data, int32 length) {
  int32 totalBytesSent = 0;

  while(totalBytesSent < length) {
    int32 remaining = length - totalBytesSent;
    char* offset = (char*)data + totalBytesSent;
    int32 bytesSent = send(socket, (const char*)offset, remaining, 0);
    
    if(bytesSent == SOCKET_ERROR) {
      if(bytesSent != WSAEWOULDBLOCK) {
	log_("WSA Exited with %d\n", bytesSent);
	return 0;	    
      }
    }    
    totalBytesSent += bytesSent; 
  }
  return 1;
}

bool32 socket_recieve(uint64& socket, void* data, int32 length) {
  int32 totalBytesRecieved = 0;

  while(totalBytesRecieved < length) {
    int32 remaining = length - totalBytesRecieved;
    char* offset = (char*)data + totalBytesRecieved;
    int32 bytesRecieved = recv(socket, offset, remaining, 0);
    
    if(bytesRecieved == 0) {
      return 0;
    }
    if(bytesRecieved == SOCKET_ERROR) {
      int32 error = WSAGetLastError();
      if(error != WSAEWOULDBLOCK) {
	log_("WSA Exited with %d\n", error);
	return 0;
      }
    }
    totalBytesRecieved += bytesRecieved; 
  }
  return 1;
}

bool32 socket_send_packet(uint64& socket, const Packet& packet) {
  uint16 length = htons(packet.length);
  if(!socket_send(socket, &length, sizeof(uint16))) return 0;
  if(!socket_send(socket, (const void*)packet.data, packet.length)) return 0;
  return 1;
}

bool32 socket_recieve_packet(uint64& socket, Packet& packet) {
  empty_packet(packet);

  uint16 length; 
  if(!socket_recieve(socket, &length, sizeof(uint16))) return 0;
  
  length = ntohs(length);
  assert_(length <= MAX_PACKET_SIZE, "Tried to send a package that exceeds the max packet size!");

  if(!socket_recieve(socket, &packet.data[0], length)) return 0;
  return 1;
}
