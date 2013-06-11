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
#ifndef H_ENSC_LIB_IO_H
#define H_ENSC_LIB_IO_H

#include <stdbool.h>
#include <stdlib.h>

bool read_all(int fd, void *dst, size_t len);
bool read_str(int fd, char *dst, size_t max_len);
ssize_t read_eof(int fd, void *dst, size_t max_len);


bool write_all(int fd, void const *src, size_t len);
bool write_str(int fd, char const *src, ssize_t len);

#endif	/* H_ENSC_LIB_IO_H */
