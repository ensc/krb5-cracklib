/*	--*- c -*--
 * Copyright (C) 2013 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
