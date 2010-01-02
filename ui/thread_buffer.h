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

#ifndef __THREAD_BUFFER_H__
#define __THREAD_BUFFER_H__

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

#include <notmuch.h>
#include <packrat.h>
#include <buffer.h>
#include <thread.h>

namespace packrat {

/*
 * Buffer for a thread of messages instantiated
 * with a thread we fill the buffer that way.
 * On a window resize, we only need to make sure
 * that there are enough characters on each line
 * to fill an entire line.
 *
 * screen will automatically truncate the lines
 * that we provide to the window width.
 */

class thread_buffer : public buffer {
	public:
		typedef boost::shared_ptr<thread_buffer> ptr;
		typedef boost::weak_ptr<thread_buffer> wptr;

		static boost::shared_ptr<thread_buffer>
			create(int rows, int cols, thread::ptr thread);

	protected:
		thread_buffer() : buffer() { }
		thread_buffer(int rows, int cols, thread::ptr thread);
	
	public:
		virtual ~thread_buffer();

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
		// this reads in each message into their respective buffers
		void read_thread(void);

		// this is the guts of our buffer
		thread::ptr thread_;
		int nlines_;
};

}

#endif // __THREAD_BUFFER_H__

