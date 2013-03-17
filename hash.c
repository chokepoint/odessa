#include <openssl/sha.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#include "odessa.h"
#include "config.h"

/* debug purposes to print out every member of the hash table. */
void iterator(gpointer key, gpointer value, gpointer user_data) {
 printf(user_data, key, value);
}	

/* use foreach to loop through hash table, and save to the admin file. */
void sync_admin_file(gpointer key, gpointer value, gpointer user_data) {
	FILE *fd;
	
	if ((fd=fopen(ADMIN_FILE, "a")) == NULL) {
		perror("fopen");
		exit(1);
	}
	
	fprintf(fd,"%s %s\n",(char *)key,(char *)value);
	fclose(fd);
}
	
/* used to free up the data allocated to keep track of the hash table. */
void key_destroyed(gpointer data) {
 DEBUG("key_destroyed called.\n");
 g_free(data);
}

/* return the sha512 sum in hex format. much like sha512sum. */
char *GetSHA512(char *string) {
    SHA512_CTX sha;
    unsigned char output[SHA512_DIGEST_LENGTH];
    char *sha_step, buffer[512];
    int cnt;

    SHA512_Init(&sha);
    SHA512_Update(&sha, string,strlen(string));
    SHA512_Final(output,&sha);

	memset(buffer,'\0',sizeof(buffer));
	sha_step = buffer;
	for (cnt = 0; cnt < SHA512_DIGEST_LENGTH; cnt++) {
		sprintf(sha_step,"%02x",(unsigned char)output[cnt]);
        sha_step++; sha_step++;
    }

    return strdup(buffer);
}

/* 
 * authenticate the given admin nick and admin passwd. 
 * returns 1 for successful login, 0 for failure.
 */
int auth_admin(char *anick, char *apasswd) {
	char *ahash = (char *) g_hash_table_lookup(admin_hash_table, anick), *apasswdsha;
	
	apasswdsha = GetSHA512(apasswd);
	if (ahash) {
		if (!strcmp(apasswdsha,ahash)) {
			free(apasswdsha);
			return 1;
		}
	}
	free(apasswdsha);
	return 0;
}

void init_hash_table(GHashTable *hash_table) {
	hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)key_destroyed, NULL);
}

/* 
 * read from the admin file format "user pass". 
 * password hash should be in sha512 format. 
 * echo "newpw" -n | sha512sum 
 * to get a new password hash.
 * add each entry into admin_hash_table.
 */
void init_admins(void) {
		FILE *fd;
		char aline[MAXLINE];
		char anick[512], ahash[512];
		
		DEBUG("init_admins() called.\n");
		
		admin_hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)key_destroyed, NULL);
		
		if (!(fd=fopen(ADMIN_FILE,"r"))) {
			perror("fopen");
			exit(1);
		}
		
		while (1) {
			if (fgets(aline,MAXLINE,fd)==NULL)
				break;
			sscanf(aline,"%s %s",&anick[0],&ahash[0]);
			g_hash_table_insert(admin_hash_table, g_strdup(anick), g_strdup(ahash));
		}
		fclose(fd);
}

/* free all memory being used by admin_hash_table */
void free_hash_table(GHashTable *hash_table) {
	g_hash_table_destroy(hash_table);
}


