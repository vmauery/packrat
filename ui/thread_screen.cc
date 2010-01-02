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

#include <curses.h>
#include <iostream>
#include <thread_screen.h>
#include <thread_buffer.h>
#include <application.h>

using namespace packrat;
using std::string;

thread_screen::thread_screen() {
}

thread_screen::thread_screen(thread::ptr thread)
		: screen_base(thread->id())
{
	// init the screen_base
	buffer_ = thread_buffer::create(rows_, cols_, thread);
	title_ = thread->subject();
	show_cursor_ = 1;
}

thread_screen::~thread_screen() {
}

screen_base::ptr thread_screen::create(thread::ptr thread) {
	screen_base::ptr ret(new thread_screen(thread));
	return ret;
}

/*
void thread_screen::cursor_move_col(int count) {
	
}

void thread_screen::cursor_move_row(int count) {
}

void thread_screen::draw_cursor(int row, int col) {
	info("draw_cursor: row "<<row);
	wattron(window_, COLOR_PAIR(color_c_normal));
	_draw_line(row);
	wattroff(window_, COLOR_PAIR(color_c_normal));
}

void thread_screen::erase_cursor(int row, int col) {
	info("erase_cursor: row "<<row);
	wattroff(window_, COLOR_PAIR(color_c_normal));
	_draw_line(row);
}
*/

int thread_screen::action(int key) {
	int handled = screen_base::action(key);
	if (handled)
		return handled;
	handled = 1;
	switch(key) {
		case KEY_ENTER:
		case 10:
			break;
		case 'a':
			info("archive thread "<<thread_->id());
		case 'q':
			info("exit thread buffer "<<thread_->id());
			application::get()->close_screen();
			break;
		case 'r':
			info("thread_screen: reply to message");
			buffer_->action(REPLY_MESSAGE, cursor_y_ + row_offset_,
					cursor_x_ + col_offset_);
			break;
		default:
			handled = 0;
			break;
	}
	return handled;
}

int thread_screen::close() {
	return 0;
}

void thread_screen::show() {
	screen_base::show();
	int row;
	for (row = 0; row < rows_; row++) {
		_draw_line(row);
	}
	draw_cursor(cursor_y_, cursor_x_);
}
