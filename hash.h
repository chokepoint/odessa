#ifndef HASH_H
#define HASH_H

#include <glib-2.0/glib.h>

#define MAXLINE 1024

GHashTable* admin_hash_table;
GHashTable* auth_hash_table;

char *GetSHA512(char *string);
int auth_admin(char *ahandle, char *apasswd);
void init_admins(void);
void free_hash_table(GHashTable *hash_table);
void sync_admin_file(gpointer key, gpointer value, gpointer user_data);
void init_hash_table(GHashTable *hash_table);
void key_destroyed(gpointer data);

#endif
