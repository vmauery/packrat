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

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <boost/shared_ptr.hpp>
#include <packrat.h>

namespace packrat {

/*
 * Base class for search result buffers
 * thread message buffers, etc.  The screen
 * class owns one of these and uses it for
 * rendering.
 *
 * The premise here is to provide a line
 * interface to what might otherwise be a
 * non-linear chunk of data.  The subclasses
 * of buffer must provide some way of parsing
 * their type of data and presenting it to
 * the screen class.
 */

class buffer {
	public:
		typedef boost::shared_ptr<buffer> ptr;
		typedef boost::weak_ptr<buffer> wptr;
		
	protected:
		buffer() : rows_(0), cols_(0), page_(0) { }
		buffer(int rows, int cols) : rows_(rows), cols_(cols), page_(0) { }
	
	public:
		virtual ~buffer() { }

		// called when screen wants to perform an action
		// on a line at a given y/x position the screen
		// rows and col is the offset into that row on the current page
		virtual int action(int action_id, int row, int col) = 0;

		// called at render time to get each row to render
		virtual const char *get_line(int offset) = 0;

		// called on a window resize so the lines can be recalculated
		virtual void resize(int y, int x) {
			rows_ = y;
			cols_ = x;
		}
		
		virtual void page(int p) {
			page_ = p;
		}
		virtual int page(void) {
			return page_;
		}

		virtual int nlines(void) = 0;

	protected:
		int rows_;
		int cols_;
		int page_;

};

}

#endif // __BUFFER_H__

