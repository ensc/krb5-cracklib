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

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include <krb5/pwqual_plugin.h>

#include "ensc-lib/io.h"

#define MAX_FD	1024

#ifndef __export
#  define __export __attribute__((__visibility__("default")))
#endif

static void write_errno(int fd, char const *op)
{
	char const	*s = strerror(errno);

	write_all(fd, op, strlen(op));
	write_all(fd, ": ", 2);
	write_all(fd, s, strlen(s));
}

static void xclose(int fd)
{
	if (fd != -1)
		close(fd);
}

static krb5_error_code
pwqual_cracklib_check(krb5_context context, krb5_pwqual_moddata data,
		      const char *password, const char *policy_name,
		      krb5_principal princ, const char **languages)
{
	pid_t		pid = -1;
	int		p_ctrl[2] = { -1, -1 };
	int		p_wr[2] = { -1, -1 }; /* write to child */
	int		p_rd[2] = { -1, -1 }; /* read from child */
	krb5_error_code	ret;

	/* Don't check for principals with no password policy. */
	if (policy_name == NULL)
		return 0;

	/* order is important; wr[0] becomes fd #0, rd[1] fd #1 */
	if (pipe2(p_wr, 0) < 0) {
		com_err("pwqual_cracklib_check", errno, "pipe2(<wr>)");
	} else if (pipe2(p_rd, 0) < 0) {
		com_err("pwqual_cracklib_check", errno, "pipe2(<rd>)");
	} else if (pipe2(p_ctrl, O_CLOEXEC) < 0) {
		com_err("pwqual_cracklib_check", errno, "pipe2(<ctrl>)");
	} else {
		pid = fork();

		if (pid < 0) {
			com_err("pwqual_cracklib_check", errno, "fork()");
		}
	}

	if (pid < 0) {
		xclose(p_ctrl[0]);
		xclose(p_wr[1]);
		xclose(p_rd[0]);
		xclose(p_wr[0]);
		xclose(p_rd[1]);
		xclose(p_ctrl[1]);

		ret = KADM5_PASS_Q_GENERIC;
	} else if (pid == 0) {
		int	fd;
		bool	ok;

		close(p_ctrl[0]);
		close(p_wr[1]);
		close(p_rd[0]);

		for (fd = 2; fd < MAX_FD; ++fd) {
			if (fd != p_ctrl[1] && fd != p_wr[0] && fd != p_rd[1])
				close(fd);	/* ignore errors */
		}

		fd = open("/dev/null", O_WRONLY);

		ok = false;
		if (fd < 0)
			write_errno(p_ctrl[1], "open(/dev/null)");
		else if (dup2(p_wr[0], 0) < 0)
			write_errno(p_ctrl[1], "dup2(<wr>)");
		else if (dup2(p_rd[1], 1) < 0)
			write_errno(p_ctrl[1], "dup2(<rd>)");
		else if (dup2(fd, 2) < 0)
			write_errno(p_ctrl[1], "dup2(</dev/null>)");
		else
			ok = true;

		if (ok) {
			if (p_wr[0] > 2)
				close(p_wr[0]);
			if (p_rd[1] > 2)
				close(p_rd[1]);
			if (fd > 2)
				close(fd);
		}

		if (ok) {
			execl(CHECKPASS_PROG, CHECKPASS_PROG, (char *)NULL);
			write_errno(p_ctrl[1], "execvl(<checkpass>)");
		}

		_exit(1);
	} else if (pid > 0) {
		char		msg[1024];
		ssize_t		l;
		int		status;
		char const	*username;
		size_t		username_len;
		bool		ok;

		close(p_wr[0]);
		close(p_rd[1]);
		close(p_ctrl[1]);

		if (princ->data && krb5_princ_size(context, princ) > 0) {
			username = princ->data[0].data;
			username_len = princ->data[0].length;
		} else {
			username = "";
			username_len = 0;
		}

		l = read_eof(p_ctrl[0], msg, sizeof msg);

		ok = false;
		if (l > 0)
			com_err("pwqual_cracklib_check", 0,
				"failed to start checkpass program: %.*s",
				(int)l, msg);
		else if (!write_str(p_wr[1], username, username_len))
			com_err("pwqual_cracklib_check", errno,
				"failed to send username");
		else if (!write_str(p_wr[1], password, strlen(password)))
			com_err("pwqual_cracklib_check", errno,
				"failed to send password");
		else
			ok = true;

		if (ok) {
			l = read_eof(p_rd[0], msg, sizeof msg);
			if (l < 0) {
				com_err("pwqual_cracklib_check", errno,
					"failed to read response");
				ok = false;
			}
		}

		close(p_ctrl[0]);
		close(p_wr[1]);
		close(p_rd[0]);

		if (TEMP_FAILURE_RETRY(waitpid(pid, &status, 0)) < 0) {
			com_err("pwqual_cracklib_check", errno,
				"failed to wait for child");
			ok = false;
		}

		if (ok && (!WIFEXITED(status) || WEXITSTATUS(status) != 0)) {
			if (l == 0)
				com_err("pwqual_cracklib_check", KADM5_PASS_Q_GENERIC,
					"checkpass exited with %04x", status);
			else
				krb5_set_error_message(context, KADM5_PASS_Q_GENERIC,
						       "%.*s", (int)l, msg);

			ok = false;
		}

		if (ok)
			ret = 0;
		else
			ret = KADM5_PASS_Q_GENERIC;
	}

	return ret;
}

krb5_error_code __export
pwqual_cracklib_initvt(krb5_context context, int maj_ver, int min_ver,
		       krb5_plugin_vtable vtable)
{
	struct krb5_pwqual_vtable_st	*vt;

	if (maj_ver != 1)
		return KRB5_PLUGIN_VER_NOTSUPP;

	vt = (struct krb5_pwqual_vtable_st *)vtable;
	memset(vt, 0, sizeof *vt);

	vt->name  = "cracklib";
	vt->check = pwqual_cracklib_check;

	return 0;
}
