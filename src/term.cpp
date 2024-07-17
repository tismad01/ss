#include "../inc/term.hpp"
#include <sys/types.h>
#include <sys/select.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

addr_t term_in = 0xFFFFFF04;
addr_t term_out = 0xFFFFFF00;

termios told;

void term_init() {
	tcgetattr(STDIN_FILENO, &told);
	termios tnew = told;

	// disable echoing and buffering of input characters
	tnew.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSANOW, &tnew);

	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
}

void term_close() {
	tcsetattr(STDIN_FILENO, TCSANOW, &told);
}

int kbhit() {
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

