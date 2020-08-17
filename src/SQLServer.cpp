#pragma once

#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

#define sql_assert(condition, message) \
  if(SQL_SUCCESS != condition) { \
    log_("SQL ERROR: %s\n", message); \
    log_("\n"); \
    exit(1); \
  }

struct SQLInfo {
  void* env = nullptr;
  void* dbc = nullptr;
};

struct AccountInfo {
  bool32 loggedin = 0;
  char username[128];
  char password[128];
  char score[16];
};

void connect_sql(SQLInfo* info, const char* connectionString) {
  void*& env = info->env;
  void*& dbc = info->dbc;
  
  sql_assert(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env),
	     "Couldn't allocate an environment handle!");
  sql_assert(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0),
	     "Couldn't set environment attributes!");
  sql_assert(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc),
	     "Couldn't allocate a database connection handle!");
  sql_assert(SQLSetConnectAttr(dbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0),
	     "Couldn't set database connection attributes!");

  SQLCHAR dbcResult[1024];
  SQLRETURN result = SQLDriverConnect(dbc, 0, (SQLCHAR*)connectionString, SQL_NTS, dbcResult, 1024, 0, SQL_DRIVER_NOPROMPT); 
  
  switch(result) {
  case SQL_NO_DATA_FOUND:
    sql_assert(result, "No data found on dbcResult!");
    break;
  case SQL_INVALID_HANDLE:
    sql_assert(result, "Invalid handle!");
    break;
  case SQL_ERROR:
    SQLCHAR state[1024];
    SQLCHAR message[1024];

    SQLGetDiagRec(SQL_HANDLE_DBC, dbc, 1, state, NULL, message, 1024, NULL);
    log_("STATE: %s, MESSAGE %s\n", state, message);
    exit(1);
    break;
  default: break;
  }
}

void* sql_statement(SQLInfo* info, const char* query) {
  void* stmt = nullptr;
  sql_assert(SQLAllocHandle(SQL_HANDLE_STMT, info->dbc, &stmt),
	     "Couldn't allocate a statement handle!");
  if(SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS) != SQL_SUCCESS) return nullptr;
  return stmt;
}

void release_sql_statement(void* stmt) {
  SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

void disconnect_sql(SQLInfo* info) {
  SQLDisconnect(info->dbc);
  SQLFreeHandle(SQL_HANDLE_DBC, info->dbc);
  SQLFreeHandle(SQL_HANDLE_ENV, info->env);
}

void sql_printall(SQLInfo* info) {
  void* stmt = sql_statement(info, "SELECT * FROM accounts");
  
  char username[128];
  char password[128];
  int64 score;
  uint32 id = 0;
  while(SQLFetch(stmt) == SQL_SUCCESS) {
    SQLGetData(stmt, 1, SQL_C_DEFAULT, &username, sizeof(username), NULL);
    SQLGetData(stmt, 2, SQL_C_DEFAULT, &password, sizeof(password), NULL);
    SQLGetData(stmt, 3, SQL_C_DEFAULT, &score,    sizeof(score),    NULL);
    log_("ID: %d\n", id);
    log_("  username: %s\n",   username);
    log_("  password: %s\n",   password);
    log_("  score:    %llu\n", score);
    id++;
  }

  release_sql_statement(stmt);
}

bool32 register_account(const AccountInfo& account, SQLInfo* info) {
  char fullStr[2048];
  strcpy(fullStr, "SELECT TOP 1 username FROM accounts WHERE username='");
  strcat(fullStr, account.username);
  strcat(fullStr, "'");
  
  void* stmt = sql_statement(info, fullStr);
  if(SQLFetch(stmt) == SQL_SUCCESS) {
    log_("This username is already taken!\n");
    release_sql_statement(stmt);
    return false;
  }
  
  _ltoa(5000, (char*)account.score, 10);

  strcpy(fullStr, "INSERT INTO accounts VALUES('");
  strcat(fullStr, account.username);
  strcat(fullStr, "', '");
  strcat(fullStr, account.password);
  strcat(fullStr, "', ");
  strcat(fullStr, account.score);
  strcat(fullStr, ")");
  stmt = sql_statement(info, fullStr);

  release_sql_statement(stmt);
  return true;
}

bool32 login_account(AccountInfo& account, SQLInfo* info) {
  char fullStr[2048];
  strcpy(fullStr, "SELECT TOP 1 username, password, score FROM accounts WHERE username='");
  strcat(fullStr, account.username);
  strcat(fullStr, "' AND password='");
  strcat(fullStr, account.password);
  strcat(fullStr, "'");
  
  void* stmt = sql_statement(info, fullStr);
  if(SQLFetch(stmt) != SQL_SUCCESS) {
    log_("This account is not registered!\n");
    release_sql_statement(stmt);
    return false;
  }

  int64 score;
  SQLGetData(stmt, 1, SQL_C_DEFAULT, &(account.username), sizeof(account.username), NULL);
  SQLGetData(stmt, 2, SQL_C_DEFAULT, &(account.password), sizeof(account.password), NULL);
  SQLGetData(stmt, 3, SQL_C_DEFAULT, &score,              sizeof(int64),            NULL);
  _ltoa(score, (char*)account.score, 10);

  release_sql_statement(stmt);
  return true;
}

bool32 update_account(const AccountInfo& account, SQLInfo* info) {
  char fullStr[2048];
  strcpy(fullStr, "UPDATE accounts SET score=");
  strcat(fullStr, account.score);
  strcat(fullStr, " WHERE username='");
  strcat(fullStr, account.username);
  strcat(fullStr, "' AND password='");
  strcat(fullStr, account.password);
  strcat(fullStr, "'");
  
  void* stmt = sql_statement(info, fullStr);
  if(!stmt) {
    strcpy(fullStr, "INSERT INTO accounts VALUES('");
    strcat(fullStr, account.username);
    strcat(fullStr, "', '");
    strcat(fullStr, account.password);
    strcat(fullStr, "', ");
    strcat(fullStr, account.score);
    strcat(fullStr, ")");
    stmt = sql_statement(info, fullStr);
  }

  bool32 success = stmt != nullptr; 
  release_sql_statement(stmt);
  return success;
}
