#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>

#define wsa_fail(errorcode) assert_(false, "WSA ERROR: %d", errorcode);

enum ESocketOption {
   TCP_NO_DELAY,
};

enum IPVersion {
   NONE,
   IPV4,
   IPV6
};

struct IPEndPoint {
  const char* ip;
  uint8 ipdata[4];
  uint16 port;
  IPVersion ipversion = IPVersion::NONE;
};

void print_bytes(const IPEndPoint& endPoint) {
  for(uint8 byte : endPoint.ipdata) {
    log_("%d.", (uint32)byte);
  }
  log_("\n");
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

  in_addr addr;
  int32 result = inet_pton(AF_INET, ip, &addr);
  
  if(result == 0) {
    assert_(false, "coulnd't create ip endpoint, entered invalid dotted decimal string!");
  }
  else if(result < 0) {
    wsa_fail(WSAGetLastError());
  }
  
  if(addr.S_un.S_addr != INADDR_NONE) {
    ipEndPoint.ip = ip;
    memcpy(&ipEndPoint.ipdata[0], &addr.S_un.S_addr, sizeof(ULONG)); 
    ipEndPoint.ipversion = IPVersion::IPV4;
  }
  return ipEndPoint;
}

void set_socket_option(uint64& socket, ESocketOption option, bool32 value) {
  int32 result;
  switch(option) {
  case ESocketOption::TCP_NO_DELAY: {
    result = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(bool32));
    break;
  }
  default:
    assert_(false, "socket option hasn't been implemented yet!");
  }
  if(result) {
    wsa_fail(WSAGetLastError());
  }
}

uint32 create_socket() {
  uint64 sresult = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sresult == INVALID_SOCKET) {
    wsa_fail(WSAGetLastError());
    return 0;
  }
  set_socket_option(sresult, ESocketOption::TCP_NO_DELAY, 1);
  return sresult;
}

void close_socket(uint64& socket) {
  int32 result = closesocket(socket);
  if(result) {
    wsa_fail(WSAGetLastError());
  }
  socket = INVALID_SOCKET;
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
}

void socket_listen(uint64& socket, const IPEndPoint& ipEndPoint, int32 backlog) {
  bind_socket(socket, create_ip_endpoint("0.0.0.0", 4790));
  int32 result = listen(socket, backlog); 
  if(result) {
    wsa_fail(WSAGetLastError());
  }
}

void accept_socket(uint64& inSocket, uint64& outSocket) {
  outSocket = accept(inSocket, nullptr, nullptr);
  if(outSocket == INVALID_SOCKET) {
    wsa_fail(WSAGetLastError());
  }
}

void connect_socket(uint64& socket, const IPEndPoint& ipEndPoint) {
  if(ipEndPoint.ipversion == IPVersion::IPV4) {
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, &(ipEndPoint.ipdata)[0], sizeof(ULONG));
    addr.sin_port = htons(ipEndPoint.port);
    
    int32 result = connect(socket, (sockaddr*)&addr, sizeof(sockaddr_in));
    if(result) {
      wsa_fail(WSAGetLastError());
    }
  }
}
