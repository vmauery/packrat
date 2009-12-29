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
#include <search_screen.h>
#include <search_buffer.h>

using namespace packrat;
using std::string;

search_screen::search_screen() {
}

search_screen::search_screen(string search)
		: screen_base(search)
{
	// init the screen_base
	buffer_ = search_buffer::create(rows_, cols_, search);
	title_ = search;
}

search_screen::~search_screen() {
}

screen_base::ptr search_screen::create(string search) {
	screen_base::ptr ret(new search_screen(search));
	return ret;
}

/*
void search_screen::cursor_move_col(int count) {
	
}

void search_screen::cursor_move_row(int count) {
}
*/

void search_screen::draw_cursor(int row, int col) {
	info("draw_cursor: row "<<row);
	wattron(window_, COLOR_PAIR(color_c_normal));
	_draw_line(row);
	wattroff(window_, COLOR_PAIR(color_c_normal));
}

void search_screen::erase_cursor(int row, int col) {
	info("erase_cursor: row "<<row);
	wattroff(window_, COLOR_PAIR(color_c_normal));
	_draw_line(row);
}

int search_screen::action(int key) {
	int handled = screen_base::action(key);
	if (handled)
		return handled;
	handled = 1;
	switch(key) {
		case KEY_ENTER:
		case 10:
			// open message
			buffer_->action(search_buffer::VIEW_THREAD, cursor_y_, cursor_x_);
			break;
		default:
			handled = 0;
			break;
	}
	return handled;
}

int search_screen::close() {
	return 0;
}

void search_screen::show() {
	screen_base::show();
	int row;
	for (row = 0; row < rows_; row++) {
		if (row == cursor_y_)
			draw_cursor(row, cursor_x_);
		else
			_draw_line(row);
	}
}

