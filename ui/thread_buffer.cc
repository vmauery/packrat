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

#include <application.h>
#include <thread_buffer.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace packrat;
using std::string;
using std::stringstream;
using std::setw;
using std::setfill;
using std::left;

thread_buffer::thread_buffer(int rows, int cols, thread::ptr thread)
		: buffer(rows, cols), thread_(thread), nlines_(0) {
	// read in the entire thread
}

thread_buffer::~thread_buffer() {
}

thread_buffer::ptr thread_buffer::create(int rows, int cols, thread::ptr thread) {
	thread_buffer::ptr ret(new thread_buffer(rows, cols, thread));
	return ret;
}

int thread_buffer::action(int action_id, int row, int col) {
	int handled = 1;
	// columns don't matter in thread threades since each thread
	// is the entire row and an action anywhere in that row is the
	// same for the entire row
	int offset = page_ * rows_ + row;
	// select the thread
	switch (action_id) {
		case ARCHIVE_THREAD:
			// remove tags (all tags? how to decide current tag?)
			break;
		case DELETE_THREAD:
			// delete from database, mark T flag
			break;
		case MARK_SPAM:
			// remove all tags, add spam tag
			break;
		case KILL_THREAD:
			// not sure how to do this... special tag?
			break;
		case TAG_THREAD:
			// select tag to tag all thread with
			break;
		case FORWARD_THREAD:
			// encapsulate and forward thread
			break;
		case FLAG_THREAD:
			// mark all message files with F flag
			break;
		case MARK_THREAD_UNREAD:
			// mark all message files with no S flag
			break;
		default:
			handled = 0;
			break;
	}
	return handled;
}

void thread_buffer::resize(int y, int x) {

}

void thread_buffer::page(int line_offset) {
	buffer::page(line_offset);
	// re-run the query on the new page
}

const char *thread_buffer::get_line(int offset) {
	info("get_line("<<offset<<"), thread_ = " << thread_.get());
	const char *ret = thread_->get_line(offset);
	info("got a line");
	return ret;
}

int thread_buffer::nlines(void) {
	return thread_->nlines();
}

