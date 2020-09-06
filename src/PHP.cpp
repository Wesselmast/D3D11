#pragma once

#include "wininet.h"

static LPCSTR acceptTypes[] = {"*/*", NULL};
static TCHAR headers[] = _T("Content-Type: application/x-www-form-urlencoded");

struct AccountInfo {
  bool32 loggedin = 0;
  char username[128];
  char password[128];
  int64 id = 0;
  int64 score = 5000;
};

struct PHP {
  HINTERNET iSession;
  HINTERNET iConnect;
};

void php_connect(PHP* php) {
  php->iSession = InternetOpen("test", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  if(!php->iSession) {
    log_("unable to open session! ERRORCODE: %d\n", (int32)GetLastError());
    return;
  }
  
  php->iConnect = InternetConnect(php->iSession, _T("84.84.69.166"), 4970, 
				  NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
  if(!php->iConnect) {
    log_("unable to connect to server! ERRORCODE: %d\n", (int32)GetLastError());
    return;
  }  
}

void php_disconnect(PHP* php) {
  if(php->iConnect) InternetCloseHandle(php->iConnect);
  if(php->iSession) InternetCloseHandle(php->iSession);
}

uint32 get_status_code(HINTERNET& request) {
  char buffer[256];
  DWORD bufLen = 256;
  
  HttpQueryInfo(request, HTTP_QUERY_STATUS_CODE, &buffer, &bufLen, NULL);
  return atoi(buffer);
}

HINTERNET php_request_base(PHP* php, const char* request) {
  HINTERNET iRequest = HttpOpenRequest(php->iConnect, "POST", _T("/index.php"), 
				       NULL, NULL, acceptTypes, 0, 0);
  if(!iRequest) {
    log_("unable to open a server request! ERRORCODE: %d\n", (int32)GetLastError());
  } 

  bool32 result = HttpSendRequest(iRequest, headers, strlen(headers), 
				  (LPVOID)request, strlen(request));
  if(!result) {
    log_("failed to send request %d\n", (int32)GetLastError());
  }
  
  return iRequest;
}

void php_request_int(PHP* php, const char* request, int64* data) {
  HINTERNET iRequest = php_request_base(php, request);
  
  uint32 statusCode = get_status_code(iRequest);
  if(statusCode == 200) {
    const int bufSize = 1024;
    char buff[bufSize];
    
    DWORD bytesRead;
    InternetReadFile(iRequest, buff, bufSize, &bytesRead);

   *data = atol(buff); 
  }

  if(iRequest) InternetCloseHandle(iRequest);
}


// hasn't been tested but should work
void php_request_str(PHP* php, const char* request, char* data) {
  HINTERNET iRequest = php_request_base(php, request);
  
  uint32 statusCode = get_status_code(iRequest);
  if(statusCode == 200) {
    const int bufSize = 4086;
    char buff[bufSize];
    
    DWORD bytesRead;
    InternetReadFile(iRequest, buff, bufSize, &bytesRead);

    memcpy(data, buff, bytesRead);
  }

  if(iRequest) InternetCloseHandle(iRequest);
}
