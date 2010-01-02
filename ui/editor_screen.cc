/* packrat - a basic curses MUA that uses the Not Much library
 *
 * Copyright © 2009 Vernon Mauery
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/ .
 *
 * Author: Vernon Mauery <vernon@mauery.com>
 */

#include <iostream>
#include <curses.h>
#include <pty.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>

#include <packrat.h>
#include <editor_screen.h>
#include <application.h>

using namespace packrat;
using std::string;

static void _close(int fd) {
	close(fd);
}

#define write_or_warn(fd, b, len) \
	do { \
	int rv = write(fd, b, len); \
	if (rv < 0) { \
		error("write failed: " << strerror(errno)); \
	} else if (rv < len) { \
		warn("write did not empty buffer ("<<rv<<" of "<<len<<" bytes written)"); \
	} \
} while(0)

int editor_screen::exec_in_pty_(const char *argv[], int *pty_fd) {
	/* Save the standard error stream. */
	struct winsize ws;
	struct termios child_tios;
#ifdef save_stderr
	int orig_stderr = dup(STDERR_FILENO);
	if (orig_stderr < 0) {
		error("dup(STDERR_FILENO) failed");
		return -1;
	}
#endif
	// turn off echo so it doesn't print what we write to it
	if (tcgetattr(STDIN_FILENO, &child_tios) < 0) {
		error("tcgetattr failed");
		return -1;
	}
	child_tios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	child_tios.c_oflag &= ~(ONLCR);
	ws.ws_row = rows_;
	ws.ws_col = cols_;

	// start a new process in a new pty returning our master fd
	pid_t pid = forkpty(pty_fd, NULL, &child_tios, &ws);
	switch (pid) {
		case -1:
			error("forkpty failed");
			return -1;
		case 0:
#ifdef save_stderr
			// reset stderr
			if (dup2(orig_stderr, STDERR_FILENO) < 0) {
				error("dup2 resetting stderr failed");
				exit(1);
			}
#endif
			execv(argv[0], (char*const*)argv);
			error("execv failed: "<<strerror(errno));
			exit(1);
		default:
#ifdef save_stderr
			_close(orig_stderr);
#endif
			break;
	}

	return pid;
}



editor_screen::editor_screen() {
}

editor_screen::editor_screen(string file)
		: screen_base(file), master_(-1), msg_file_(file)
{
	keypad(window_, FALSE);  /* enable keyboard mapping */
	nodelay(window_, TRUE);
	struct winsize ws;
	ws.ws_row = rows_;
	ws.ws_col = cols_;
	const char *args[] = {"/usr/bin/vim", "-c", "set spell spelllang=en", "-c", "set tw=72", "-c", "set filetype=mail", file.c_str(), (char *)NULL};
	// get sha1 of file to test for changes
	editor_pid_ = exec_in_pty_(args, &master_);
}

editor_screen::~editor_screen() {
	// TODO: check retval?
	info("editor " << msg_file_ << " destructor");
	waitpid(editor_pid_, NULL, 0);
	unlink(msg_file_.c_str());
}

screen_base::ptr editor_screen::create(string file) {
	screen_base::ptr ret(new editor_screen(file));
	return ret;
}

void editor_screen::cleanup_() {
	int status;
	pid_t ret = waitpid(editor_pid_, &status, 0);
	info("waitpid() returns "<<ret<<", status is "<<status<<", closing master_(fd "<<master_<<")");
	_close(master_);
	master_ = -1;
	application::get()->close_screen();
	// load viewer screen
}

int editor_screen::action(int key) {
	int handled = 1;
	// send the key to the editor
	// unless it is an escaped key
	static char escape_mode = 0;
	char c = (char)key;
	info("action: " << key << "(" << c << ")");
	if (key >= 0) {
		if (key == 1) {
			info("^A pressed: escape_mode = " << escape_mode);
			// we escape the next key and handle it ourselves
			if (escape_mode) {
				// we hit ^A twice, pass it on to the client
				info("sending ^A on to client");
				write_or_warn(master_, &c, 1);
				escape_mode = 0;
			} else {
				info("setting escape mode to true");
				escape_mode = 1;
				return handled;
			}
		} else {
			if (escape_mode) {
				// some other key was pressed, see if we wanted to handle it
				handled = screen_base::action(key);
				if (!handled) {
					write_or_warn(master_, &escape_mode, 1);
					write_or_warn(master_, &c, 1);
					escape_mode = 0;
				} else {
					return handled;
				}
			} else {
				write_or_warn(master_, &c, 1);
			}
		}
	}

	char buf_out[4096];
	int br_out = 0;
	struct pollfd fds[3];
	fds[0].fd = master_;
	fds[0].events = POLLIN|POLLHUP;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;
	int ret = poll(fds, 2, -1);
	if (ret > 0) {
		if (fds[0].revents & POLLIN) {
			info("master::POLLIN");
			br_out = read(master_, buf_out, sizeof(buf_out));
			if (br_out > 0) {
				// should we really be writing to stdout?  or /dev/tty?
				write_or_warn(STDOUT_FILENO, buf_out, br_out);
				br_out = 0;
			}
		}
		if (fds[0].revents & POLLHUP) {
			info("master::POLLHUP -- editor finished");
			cleanup_();
		}
		if (fds[1].revents & POLLIN)
			info("stdin::POLLIN");
	} else {
		info("poll failed: "<<strerror(errno));
		cleanup_();
	}
		
	return handled;
}

int editor_screen::keypressed() {
	// since we need to read and write to the child,
	// we don't want to block on wgetch
	// and not blocking on wgetch would cause a tight loop in application
	// so we need to block on poll or something
	return wgetch(window_);
}

int editor_screen::close() {
	return 0;
}

void editor_screen::show() {
	char refresh[2] = { 27, 12 };
	write(master_, &refresh, 2);
}


