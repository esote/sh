/*
 * sh is a minimal shell with chdir functionality.
 * Copyright (C) 2019  Esote
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <sys/stat.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void	prompt(void);
char	*rline(char *const, size_t const);
int	what(char *const []);
int	cd(char *);
int	exec(char *const []);

#define MAX	80
#define CWD_MAX	256

int
main(int argc, char *argv[])
{
	char buf[MAX];
	char *tok[MAX], *p, *last;
	char *cp;
	int i;

#ifdef __OpenBSD__
	if (unveil("/", "rx") == -1) {
		err(1, "unveil");
	}

	if (pledge("stdio rpath proc exec", NULL) == -1) {
		err(1, "pledge");
	}
#endif

	if (argc >= 2) {
		(void)fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}

	while(1) {
		i = 0;
		(void)memset(tok, 0, sizeof(tok));

		prompt();

		if ((cp = rline(buf, sizeof(buf))) == NULL) {
			(void)putchar('\n');
			return 0;
		}

		/* TODO quote parsing */
		p = strtok_r(cp, " ", &last);

		while(p) {
			if (i < MAX - 1) {
				tok[i++] = p;
			}

			p = strtok_r(NULL, " ", &last);
		}

		if (tok[0] == NULL) {
			continue;
		}

#ifdef DEBUG
		(void)printf("ret: %d\n", what(tok));
#else
		(void)what(tok);
#endif
	}

	return 0;
}

void
prompt(void)
{
	static char cwd[CWD_MAX];

	if (getcwd(cwd, CWD_MAX) != NULL) {
		(void)printf("[%s] ", cwd);
#ifdef DEBUG
	} else {
		warn("getcwd");
	}
#else
	}
#endif

	if (fputs("sh) ", stdout) == EOF) {
		errx(1, "fputs failed");
	}
}

char *
rline(char *const buf, size_t const size)
{
	char *p;

	if (fgets(buf, (int)size, stdin) == NULL) {
		return NULL;
	}

	if ((p = strchr(buf, '\n')) == NULL) {
		(void)fputs("line too long\n", stderr);
		return NULL;
	}

	*p = '\0';

	return buf;
}

int
what(char *const argv[]) {
	if (strcmp(argv[0], "cd") == 0) {
		return cd(argv[1]);
	} else {
		return exec(argv);
	}
}

int
cd(char *path)
{
	char *tmp = NULL;
	int ret = 0;

	if (path == NULL && (tmp = getenv("HOME")) != NULL) {
		path = strdup(tmp);
	}

	/* getenv or strdup failed */
	if (path == NULL) {
		warn("cd: null path");
		return 1;
	}

	/* TODO replace first '~' with $HOME */
	if (chdir(path) == -1) {
		warn("cd: '%s'", path);
		ret = 1;
	}

	if (tmp != NULL) {
		free(path);
	}

	return ret;
}

int
exec(char *const argv[]) {
	pid_t p;
	int status;

	if ((p = fork()) == 0) {
		if (execvp(argv[0], argv) == -1) {
			if (errno == ENOENT) {
				warnx("%s: command not found...", argv[0]);
			} else {
				warn("execvp failed %d", errno);
			}

			p = getpid();

			if (kill(p, SIGTERM) == -1) {
				err(1, "kill child %d", p);
			}

			return -1;
		}
	} else if (waitpid(p, &status, 0) == -1) {
		err(1, "waitpid");
	}

	return status;
}

