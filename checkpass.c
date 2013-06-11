#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <stdint.h>
#include <string.h>

#include <pwd.h>

#include <crack.h>

#include "ensc-lib/io.h"

int main(void)
{
	char	username[1024];
	char	passphrase[1024];
	char const	*res;

	if (!read_str(0, username, sizeof username) ||
	    !read_str(0, passphrase, sizeof passphrase))
		return EX_OSERR;

#ifdef HAVE_FascistCheckUser
	res = FascistCheckUser(passphrase, NULL, username, NULL);
#else
	res = FascistCheck(passphrase, NULL);
#endif

	memset(passphrase, 0, sizeof passphrase);

	if (res)
		write_all(1, res, strlen(res));

	__asm__ __volatile__("" ::: "memory"); /* enforce wiping of passphrase */

	return res ? 1 : 0;
}
