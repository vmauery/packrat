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

#ifndef __SEARCH_BUFFER_H__
#define __SEARCH_BUFFER_H__

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

#include <notmuch.h>
#include <packrat.h>
#include <buffer.h>
#include <thread.h>

namespace packrat {

/*
 * Buffer for thread search results.  We are
 * instantiated with a thread query handle and
 * we fill the buffer that way.  On a window
 * resize, we only need to make sure that there
 * are enough characters on each line to fill
 * an entire line.
 *
 * screen will automatically truncate the lines
 * that we provide to the window width.
 */

class search_buffer : public buffer {
	public:
		typedef boost::shared_ptr<search_buffer> ptr;
		typedef boost::weak_ptr<search_buffer> wptr;

		static boost::shared_ptr<search_buffer>
			create(int rows, int cols, std::string search);

	protected:
		search_buffer() : buffer() { }
		search_buffer(int rows, int cols, std::string search);
	
	public:
		virtual ~search_buffer();

		// called when screen wants to perform an action
		// on a line at a given y/x position the screen
		// does the translation from y/x to a character pointer
		virtual int action(buffer_action_t action_id, int row, int col);

		// called on a window resize so the lines can be recalculated
		virtual void resize(int y, int x);

		virtual void page(int line_offset);

		virtual const char *get_line(int offset);

		virtual int nlines(void);

	protected:
		// a sparse array of thread_id strings
		std::map<int, thread::ptr> threads_;
		std::map<int, std::string> thread_lines_;
		notmuch_query_t *query_;
};

}

#endif // __SEARCH_BUFFER_H__


