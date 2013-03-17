#include <libircclient/libircclient.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "config.h"
#include "odessa.h"
#include "hash.h"
#include "cmd.h"

/* List used in the command processing engine. */
command_type command_list[] = {
	{"!aop",	cmd_aop},
	{"!op",		cmd_op},
	{"!help",	cmd_help},
	{"!invite", cmd_invite},
	{"!passwd",	cmd_passwd},
	{"!reload", cmd_reload}
};

/* 
 * Command processor
 * Loop through the parameters given, if the first argument matches
 * one of the commands in command_list, then execute .cmd_func() and
 * pass parameters to the command function.
 */
int process_cmd(irc_session_t *session, const char *origin, const char *params) {
	DEBUG("process_cmd called.\n");
	char *cmd = chop_cmd(params);
	int cnt,found=0;

	for (cnt = 0; cnt < sizeof(command_list)/sizeof(command_type); ++cnt) {
		if (!strcmp(cmd,command_list[cnt].cmd_name)) { 
			found=1;
			break;
		}
	}
	
	free(cmd);
	
	if (found==1) 
		return command_list[cnt].cmd_func(session, origin, params);

	return COM_ERROR;
}

/*
 * cmd_aop - AutoOp
 * Parameters - passwd
 * Example - !aop letmein
 */
int cmd_aop(irc_session_t *session, const char *origin, const char *params) {
  DEBUG("cmd_aop called.\n");
  irc_ctx_t *ctx = (irc_ctx_t*)irc_get_ctx(session);
  char *anick = chop_nick(origin); 
  char *passwd = chop_params(params);
  if (auth_admin(anick,passwd)) {
	char cmdop[212];
	irc_cmd_msg(session, ctx->channel, "OP INBOUND.");
	snprintf(cmdop,sizeof(cmdop),"o %s",anick);
	irc_cmd_channel_mode(session, ctx->channel, cmdop);
	g_hash_table_insert(auth_hash_table, g_strdup(anick), "authed");
  } else {
	irc_cmd_msg(session, anick, "Just give up.");
  }
  if (anick != NULL)
	free(anick);
  return COM_OK;
}

/*
 * cmd_passwd - Change your aop password.
 * Parameters: passwd
 * Example: !passwd mynewpassword
 */
int cmd_passwd(irc_session_t *session, const char *origin, const char *params) {
	DEBUG("cmd_passwd called.\n");
	char *nick = chop_nick(origin), *new_pw_hash;
	char *new_pw = chop_params(params);
	
	if (!is_authed(nick)) {
		free(nick);
		return COM_ERROR;
	}
	
	init_admins();
	
	new_pw_hash = GetSHA512(new_pw);
	
	g_hash_table_replace(admin_hash_table, g_strdup(nick), g_strdup(new_pw_hash));
	
	unlink(ADMIN_FILE); /*flush to disk*/
	g_hash_table_foreach(admin_hash_table, (GHFunc)sync_admin_file, NULL);
	
	irc_cmd_msg(session, nick, "Password updated, and flushed to disk.\n");
	free(new_pw_hash);
	free(nick);
	return COM_OK;
}

/* 
 * cmd_reload - Reload the admin list from disk
 * Parameters: none
 * Example: !reload
 */
int cmd_reload(irc_session_t *session, const char *origin, const char *params) {
	DEBUG("cmd_reload called.\n");
	char *nick = chop_nick(origin);
	if (!is_authed(nick)) {
		free(nick);
		return COM_ERROR;
	}
	
	free_hash_table(admin_hash_table);
	init_admins();
	
	irc_cmd_msg(session, nick, "Admin list reloaded.\n");
	free(nick);
	return COM_OK;
}

/* 
 * cmd_help - Display the available commands
 * Parameters: none
 * Example: !help
 */
int cmd_help(irc_session_t *session, const char *origin, const char *params) {
	DEBUG("cmd_help called.\n");
	char *nick = chop_nick(origin);
	irc_cmd_msg(session, nick, "Odessa command list.\n");
	irc_cmd_msg(session, nick, "!aop <passwd> - Authenticate with bot.\n");
	irc_cmd_msg(session, nick, "!help         - Show this help.\n");
	
	if (!is_authed(nick)) {
		free(nick);
		return COM_OK;
	}
	
	irc_cmd_msg(session, nick, "!op <nick to op> - Op given nick in the channel.\n");	
	irc_cmd_msg(session, nick, "!passwd <newpw> - Change your password.\n");
	irc_cmd_msg(session, nick, "!reload			- Reload the op list from disk.\n");
	irc_cmd_msg(session, nick, "!invite <nick>  - Invite nick to current channel.\n");
	free(nick);
	return COM_OK;
}

/* 
 * cmd_invite - invite the given user to the channel.
 * Parameters: nick
 * Example: !invite wizard
 */
int cmd_invite(irc_session_t *session, const char *origin, const char *params) {
	DEBUG("cmd_invite called.\n");
	irc_ctx_t *ctx = (irc_ctx_t*)irc_get_ctx(session);
	char *nick = chop_nick(origin);
	char *invite_nick = chop_params(params);
	char resp[512];
	
	if (!is_authed(nick)) {
		free(nick);
		return COM_ERROR;
	}
	
	snprintf(resp,sizeof(resp),	"Invited %s to channel %s.\n", invite_nick, ctx->channel);
	irc_cmd_invite(session, invite_nick, ctx->channel);
	irc_cmd_msg(session, nick, resp);
	free(nick);
	return COM_OK;
}

/* 
 * cmd_op - Make the given nick an oper.
 * Parameters: nick
 * Example: !op wizard
 */
int cmd_op(irc_session_t *session, const char *origin, const char *params) {
	DEBUG("cmd_op called.\n");
	irc_ctx_t *ctx = (irc_ctx_t*)irc_get_ctx(session);
	char *nick = chop_nick(origin);
	char *op_nick = chop_params(params);
	char cmdop[212];
	
	if (!is_authed(nick)) {
		free(nick);
		return COM_ERROR;
	}
	
	irc_cmd_msg(session, ctx->channel, "OP INBOUND.");
	snprintf(cmdop,sizeof(cmdop),"o %s",op_nick);
	irc_cmd_channel_mode(session, ctx->channel, cmdop);
	g_hash_table_insert(auth_hash_table, g_strdup(op_nick), "authed");
	
	free(nick);
	return COM_OK;
}

/* Return only the first part of the full nick string given from the server. */
char *chop_nick(const char *original) {
	DEBUG("chop_nick called.\n");
	char *nick = strdup(original), *step;
	for (step = nick; *step != '!' && *step !='\0'; ++step);
	*step = '\0'; 
	return nick;
}

char *chop_cmd(const char *original) {
	DEBUG("chop_cmd called.\n");
	char *cmd = strdup(original), *step;
	for (step = cmd; *step != ' ' && *step != '\0'; ++step);
	*step = '\0';
	return cmd;
}

/* Return only the value after the chopped cmd in the givin parameters */
char *chop_params(const char *original) {
	DEBUG("chop_params called.\n");
	char *params = strdup(original), *step;
	for (step = params; *step != ' ' && *step != '\0'; ++step);
	*step = '\0';
	return ++step;
}

int is_authed(char *nick) {
	if (g_hash_table_lookup(auth_hash_table, nick)) 
		return 1;
	return 0;
}
