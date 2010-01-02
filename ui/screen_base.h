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

#ifndef __SCREEN_BASE_H__
#define __SCREEN_BASE_H__

#include <boost/shared_ptr.hpp>
#include <curses.h>
#include <string>

#include <packrat.h>
#include <buffer.h>

namespace packrat {

/*
class screen_base
        scrolling pages
        cursor up/down
        cursor left/right
        action on a line
        close buffer
        next buffer
*/

class screen_base {
	public:
		typedef boost::shared_ptr<screen_base> ptr;
		typedef boost::weak_ptr<screen_base> wptr;

		enum {
			color_normal = 1,
			color_header,
			color_quote_odd,
			color_quote_even,
			color_highlight,
			color_new_msg,
			color_flag_msg,
			// cursor colors (some kind of color inversion going on...)
			color_c_normal,
			color_c_header,
			color_c_quote_odd,
			color_c_quote_even,
			color_c_highlight,
			color_c_new_msg,
			color_c_flag_msg,
		};
		#define COLOR(C) (color_##C)
		#define CURSOR_COLOR(C) (color_c_##C)
	
	protected:
		screen_base() { }
		screen_base(std::string id);
	
	public:
		virtual ~screen_base();

		virtual void cursor_move_col(int count);
		virtual void cursor_move_row(int count);
		virtual void scroll_vertical(int count);
		virtual void scroll_horizontal(int count);
		// key action at current position
		virtual int action(int key);
		virtual int close();
		virtual void show();
		virtual void enter();
		virtual void draw_cursor(int row, int col);
		virtual void erase_cursor(int row, int col);
		virtual int keypressed ();
		const char *id();

	protected:
		virtual void _show_title();
		virtual void _draw_line(int row);

		WINDOW *window_;
		WINDOW *status_;
		int status_cols_;
		buffer::ptr buffer_;
		int row_offset_;
		int col_offset_;
		int rows_;
		int cols_;
		int cursor_x_;
		int cursor_y_;
		int show_cursor_;
		std::string title_;
		std::string id_;
		char *blank_line_;
		bool refresh_;

		static int screen_count_;

};

}

#endif // __SCREEN_BASE_H__
