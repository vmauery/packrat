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

#include <screen_base.h>
#include <application.h>
#include <string.h>

using namespace packrat;
using std::string;

int screen_base::screen_count_ = 0;

// oh, this is an ugly hack to work around stupid ripoffline.  Grrr.
static WINDOW *_tmp_status_win = NULL;
static int _tmp_status_cols = 0;
static int _status_line_init(WINDOW *nwin, int cols) {
	_tmp_status_win = nwin;
	_tmp_status_cols = cols;
	return 0;
}

screen_base::screen_base(string id)
		: row_offset_(0), col_offset_(0),
		  cursor_x_(0), cursor_y_(0), show_cursor_(0), id_(id)
{
	if (screen_count_++ == 0) {
		info("global screen_base initialization");
		(void) ripoffline(-1, _status_line_init); // remove a line from the bottom for our status line
		status_ = _tmp_status_win;
		_tmp_status_win = NULL;
		status_cols_ = _tmp_status_cols;
		_tmp_status_cols = 0;

		(void) initscr();      /* initialize the curses library */
		keypad(stdscr, TRUE);  /* enable keyboard mapping */
		(void) cbreak();       /* take input chars one at a time, no wait for \n */
		(void) noecho();       /* don't echo input */

		if (has_colors())
		{
			start_color();
			info("COLOR_PAIRS="<<COLOR_PAIRS);
			// TODO: add a few more colors?
			init_pair(color_normal, COLOR_WHITE, COLOR_BLACK);
			init_pair(color_header, COLOR_GREEN, COLOR_BLACK);
			init_pair(color_quote_even, COLOR_MAGENTA, COLOR_BLACK);
			init_pair(color_quote_odd, COLOR_BLUE, COLOR_BLACK);
			init_pair(color_highlight, COLOR_CYAN, COLOR_BLACK);
			init_pair(color_new_msg, COLOR_YELLOW, COLOR_BLACK);
			init_pair(color_flag_msg, COLOR_RED, COLOR_BLACK);
			// cursor invert colors
			init_pair(color_c_normal, COLOR_WHITE, COLOR_BLUE);
			init_pair(color_c_header, COLOR_GREEN, COLOR_BLUE);
			init_pair(color_c_quote_even, COLOR_MAGENTA, COLOR_BLUE);
			init_pair(color_c_quote_odd, COLOR_BLACK, COLOR_BLUE);
			init_pair(color_c_highlight, COLOR_CYAN, COLOR_BLUE);
			init_pair(color_c_new_msg, COLOR_YELLOW, COLOR_BLUE);
			init_pair(color_c_flag_msg, COLOR_RED, COLOR_BLUE);
		}
	}

	// allocate a new curses window
	window_ = newwin(0, 0, 0, 0);
	// TODO: we may want to change bf to FALSE here and translate the 
	// escape codes ourself in the future.  This way, we can use just
	// plain escape to get out of a sticky situation.
	keypad(window_, TRUE);  /* enable keyboard mapping */
	// set the size
	getmaxyx(window_, rows_, cols_);
	blank_line_ = new char[cols_+1];
	memset(blank_line_, ' ', cols_);
	blank_line_[cols_] = 0;
	info("screen_count_: "<<screen_count_);
}
	
screen_base::~screen_base() {
	// destroy the window
	info("screen_count_: "<<screen_count_);
	info("destructor: "<<id());
	delwin(window_);
	delete [] blank_line_;
	if (--screen_count_ == 0) {
		info("global screen_base destructor");
		endwin();
	}
}

void screen_base::cursor_move_col(int count) {
	count += cursor_x_;
	cursor_x_ = minmax(count, 0, cols_-1);
	this->draw_cursor(cursor_y_, cursor_x_);
	refresh_ = true;
}

void screen_base::cursor_move_row(int count) {
	int scroll_lines, oldc = cursor_y_;
	cursor_y_ = minmax(count+cursor_y_, 0, rows_-1);
	scroll_lines = (count+oldc) - cursor_y_;
	// erase the old cursor
	this->erase_cursor(oldc, cursor_x_);
	if (scroll_lines) {
		// we tried to move past the top or bottom of the screen
		scroll_vertical(scroll_lines);
		this->_draw_line(cursor_y_);
	}
	// draw the new cursor
	this->draw_cursor(cursor_y_, cursor_x_);
}

void screen_base::draw_cursor(int row, int col) {
	wmove(window_, row, col);
	refresh_ = true;
}

void screen_base::erase_cursor(int row, int col) {
}

void screen_base::scroll_vertical(int count) {
	// scrolling does not move the cursor
	// we only scroll until the last line
	int nlines = buffer_->nlines();
	info("scroll_vertical: row_offset_: "<<row_offset_<<", nlines: "
	    <<nlines<<", count: "<<count<<", rows_:"<<rows_);
	if (row_offset_ + rows_ + count >= nlines) {
		info("row_offset_+rows+count >= nlines: "<<row_offset_ + rows_ + count <<">="<<nlines);
		count = nlines - row_offset_ - rows_;
	} else if (row_offset_ + count < 0) {
		info("row_offset_+count < 0: "<<row_offset_ + count <<"<0");
		count = -row_offset_;
	}
	row_offset_ += count;
	info("scroll_vertical: row_offset_: "<<row_offset_<<", nlines: "
	    <<nlines<<", count: "<<count<<", rows_:"<<rows_);
	if (count) {
		info("scrolling "<<count<<" lines");
		scrollok(window_, TRUE);
		wscrl(window_, count);
		scrollok(window_, FALSE);
	}
}

void screen_base::scroll_horizontal(int count) {
	// scrolling does not move the cursor
	// we only scroll until the last line
	// hmmmm... apparently this doesn't have a curses
	// function.  We just have to repaint the entire
	// screen.  That sounds like fun.
}

int screen_base::action(int key) {
	// this is where the keys get mapped from key to action
	// and passed on to the buffer, which is where the action
	// is actually done (since it is the one with the brains).
	// EXCEPT for things like closing the screen...  We can do
	// that.  But we should notify the buffer.
	
	// TODO: read a config file to determine key bindings
	
	// The base class ought to do the handling for generic
	// stuff, like closing, getting search string, etc., that
	// all buffers do.  Then the individual buffers can handle
	// it if we don't.
	
	int handled = 1;
	
	switch (key) {
		case 'q':
			application::get()->close_screen();
			break;
		case '/':
			// start the search
			break;
		case 5:
			// ctrl-e (scroll up)
			this->erase_cursor(cursor_y_, cursor_x_);
			this->scroll_vertical(1);
			this->_draw_line(rows_-1);
			this->draw_cursor(cursor_y_, cursor_x_);
			refresh_ = true;
			break;
		case 25:
			// ctrl-y (scroll down)
			this->erase_cursor(cursor_y_, cursor_x_);
			this->scroll_vertical(-1);
			this->_draw_line(0);
			this->draw_cursor(cursor_y_, cursor_x_);
			refresh_ = true;
			break;
		case 'b':
			// next buffer
			application::get()->next_buffer();
			break;
		case KEY_UP:
			info("move cursor up");
			this->cursor_move_row(-1);
			break;
		case KEY_DOWN:
			info("move cursor down");
			this->cursor_move_row(1);
			break;
		case KEY_LEFT:
			info("move cursor left");
			this->cursor_move_col(-1);
			break;
		case KEY_RIGHT:
			info("move cursor right");
			this->cursor_move_col(1);
			break;
		default:
			handled = 0;
			break;
	}
	if (refresh_) {
		refresh_ = false;
		wrefresh(window_);
	}
	return handled;
}

int screen_base::close() {
	// prompt for saving/closing?
	return 0;
}

void screen_base::enter() {
	// set cursor
	curs_set(show_cursor_);
	// set echo??
	// set something else??
}

void screen_base::show() {
	_show_title();
}

void screen_base::_draw_line(int row) {
	info("_draw_line: cols_ = " << cols_);
	mvwaddnstr(window_, row, 0, buffer_->get_line(row+row_offset_), cols_-10);
	int x = getcurx(window_);
	waddnstr(window_, blank_line_, cols_-x);
	refresh_ = true;
}

void screen_base::_show_title() {
	if (settings::env("TERM") == "xterm") {
		info("setting title to '"<<title_.c_str()<<"'");
		fprintf(stdout, "\033]; %s \007", title_.c_str());
	}
}

int screen_base::keypressed() {
	return wgetch(window_);
}

const char *screen_base::id() {
	return id_.c_str();
}
