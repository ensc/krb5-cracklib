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

#include "io.h"

#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <et/com_err.h>

#ifndef __hidden
#  define __hidden __attribute__((__visibility__("hidden")))
#endif

ssize_t __hidden read_eof(int fd, void *dst, size_t max_len)
{
	size_t		len = 0;

	while (len < max_len) {
		ssize_t	l = read(fd, dst, max_len - len);

		if (l > 0) {
			dst += l;
			len += l;
		} else if (l == 0) {
			return len;
		} else if (errno == EINTR) {
			continue;
		} else {
			com_err("read_eof", errno, "read()");
			return -1;
		}
	}

	com_err("read_eof", 0, "too much data()");
	return -1;
}

bool __hidden read_all(int fd, void *dst, size_t len)
{
	while (len > 0) {
		ssize_t	l = read(fd, dst, len);

		if (l > 0) {
			dst += l;
			len -= l;
		} else if (l == 0) {
			com_err("read_all", 0, "read(): EOF");
			break;
		} else if (errno == EINTR) {
			continue;
		} else {
			com_err("read_all", errno, "read()");
			break;
		}
	}

	return len == 0;
}

bool __hidden read_str(int fd, char *dst, size_t max_len)
{
	uint32_t	slen;

	if (!read_all(fd, &slen, sizeof slen))
		return false;

	slen = be32toh(slen);
	if (slen >= max_len) {
		com_err("read_str", 0, "buffer too small (%zu <= %u)\n",
			max_len, (unsigned int)(slen));
		return false;
	}

	if (!read_all(fd, dst, slen))
		return false;

	dst[slen] = '\0';
	return true;
}

bool __hidden write_all(int fd, void const *src, size_t len)
{
	while (len > 0) {
		ssize_t	l = write(fd, src, len);

		if (l > 0) {
			src += l;
			len -= l;
		} else if (l == 0) {
			com_err("write_all", 0, "failed to write data()");
			break;
		} else if (errno == EINTR) {
			continue;
		} else {
			com_err("write_all", errno, "write()");
			break;
		}
	}

	return len == 0;
}

bool __hidden write_str(int fd, char const *src, ssize_t len)
{
	uint32_t	slen;

	if (len == -1)
		len = strlen(src);

	slen = htobe32(len);

	return (write_all(fd, &slen, sizeof slen) &&
		write_all(fd, src, len));
}
