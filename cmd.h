#ifndef CMD_H
#define CMD_H

typedef struct s_command_type {
  char *cmd_name;
  int (*cmd_func)();
} command_type;

#define COM_ERROR 0
#define COM_OK 1

char *chop_cmd(const char *original);
char *chop_nick(const char *original);
char *chop_params(const char *original);

int is_authed(char *nick);

int process_cmd(irc_session_t *session, const char *origin, const char *params);

int cmd_aop(irc_session_t *session, const char *origin, const char *params);
int cmd_passwd(irc_session_t *session, const char *origin, const char *params);
int cmd_reload(irc_session_t *session, const char *origin, const char *params);
int cmd_help(irc_session_t *session, const char *origin, const char *params);
int cmd_invite(irc_session_t *session, const char *origin, const char *params);
int cmd_op(irc_session_t *session, const char *origin, const char *params);

#endif
