#ifndef ODESSA_H
#define ODESSA_H

#include <libircclient/libircclient.h>

void event_connect(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count);
		   
void event_join(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count);

void event_privmsg(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count);

void event_channel(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count);

void event_quit(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count);

void event_part(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count);

struct irc_ctx {
  char *nick;
  char *username;
  char *realname;
  char *server;
  int port;
  char *password;
  char *channel;
};

typedef struct irc_ctx irc_ctx_t;
		   
#define DEBUG_APP
#ifdef DEBUG_APP
#define DEBUG(...) fprintf(stderr, __VA_ARGS__);
#else
#define DEBUG(...)
#endif

#endif
