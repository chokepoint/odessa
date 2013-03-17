#include <libircclient/libircclient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "odessa.h"
#include "hash.h"
#include "cmd.h"

int main(int argc, char **argv)
{
  irc_callbacks_t callbacks;
  irc_session_t *session;
  irc_ctx_t ctx;

  init_admins();
  auth_hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)key_destroyed, NULL);
  
  memset(&callbacks, 0, sizeof(callbacks));

  callbacks.event_connect = event_connect;
  callbacks.event_privmsg = event_privmsg; 
  callbacks.event_channel = event_channel;
  callbacks.event_join = event_join;
  callbacks.event_quit = event_quit;
  callbacks.event_part = event_part;
  
  session = irc_create_session(&callbacks);

  if(!session) {
    fprintf(stderr, "Error creating IRC session, exiting.\n");
    return 1;
  }

  ctx.nick     = strdup(NICK);
  ctx.username = strdup(USERNAME);
  ctx.realname = strdup(REALNAME);
  ctx.server   = strdup(SERVER);
  ctx.port     = PORT;
  ctx.password = strdup(PASSWORD);
  ctx.channel  = strdup(CHANNEL);

  irc_set_ctx(session, &ctx);

  while(1) {
    if(irc_connect(session, SERVER, PORT, PASSWORD, NICK, USERNAME, REALNAME) != 0) {
      fprintf(stderr, "Could not connect: %s\n", irc_strerror(irc_errno(session)));
      //return 1;
      continue;
    }
    
    irc_run(session);
    sleep(90);
  }
  free_hash_table(admin_hash_table);
  free_hash_table(auth_hash_table);
  return 0;
}

void event_connect(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count)
{
  DEBUG("event_connect called\n");

  irc_ctx_t *ctx = (irc_ctx_t*)irc_get_ctx(session);

  irc_cmd_join(session, ctx->channel, 0);
}

void event_privmsg(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count)
{
  DEBUG("event_privmsg called\n");

  process_cmd(session, origin, params[1]);
  return;
}

void event_join(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count)
{
  DEBUG("event_join called\n");

  irc_ctx_t *ctx = (irc_ctx_t*)irc_get_ctx(session);
  char *anick,mymsg[512];

  anick = chop_nick(origin);
  if (strcmp(anick,NICK)) { 
	snprintf(mymsg,512,"GIVE US THE FUCKING CHILD %s!",anick);
	irc_cmd_msg(session, ctx->channel, mymsg);
  }
  free(anick);
  return;
}


void event_channel(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count)
{
  DEBUG("event_channel called\n");

  //irc_ctx_t *ctx = (irc_ctx_t*)irc_get_ctx(session);

  return;
}

void event_quit(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count)
{
  DEBUG("event_quit called\n");

  char *nick = chop_nick(origin);
  
  if (is_authed(nick)) 
	  g_hash_table_remove(auth_hash_table, nick);

  free(nick);
  return;
}

void event_part(irc_session_t *session,
		   const char *event,
		   const char *origin,
		   const char **params,
		   unsigned int count)
{
  DEBUG("event_channel called\n");

  char *nick = chop_nick(origin);
  
  if (is_authed(nick)) 
	  g_hash_table_remove(auth_hash_table, nick);

  free(nick);
  return;
}

