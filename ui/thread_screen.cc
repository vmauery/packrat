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

#include <thread_screen.h>

using namespace packrat;

thread_screen::thread_screen() {
}

thread_screen::thread_screen(const char *thread_id)
		: screen_base(thread_id) {
}

thread_screen::~thread_screen() {
}

screen_base::ptr thread_screen::create(const char *thread_id) {
	screen_base::ptr ret(new thread_screen(thread_id));
	return ret;
}

void thread_screen::cursor_move_col(int count) {
}

void thread_screen::cursor_move_row(int count) {
}

int thread_screen::action(int key) {
	return 0;
}

int thread_screen::close() {
	return 0;
}

void thread_screen::show() {
}
