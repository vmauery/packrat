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

#ifndef __SEARCH_SCREEN_H__
#define __SEARCH_SCREEN_H__

#include <boost/shared_ptr.hpp>
#include <string>

#include <packrat.h>
#include <screen_base.h>

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

class search_screen : public screen_base {
	public:
		typedef boost::shared_ptr<search_screen> ptr;
		typedef boost::weak_ptr<search_screen> wptr;
	
	protected:
		search_screen();
		search_screen(const char *search);
	
	public:
		virtual ~search_screen();

		static screen_base::ptr create(const char *search);

		//virtual void cursor_move_col(int count);
		//virtual void cursor_move_row(int count);

		virtual void draw_cursor(int row, int col);
		virtual void erase_cursor(int row, int col);

		virtual int action(int key);
		virtual int close();
		virtual void show();

	protected:
		virtual void _show_title();

};

}

#endif // __SEARCH_SCREEN_H__

