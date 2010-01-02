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
#include <search_buffer.h>
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

search_buffer::search_buffer(int rows, int cols, string search)
		: buffer(rows, cols) {
	int line;
	query_ = notmuch_query_create(application::get()->db(), search.c_str());
	// fill the threads_ vector
	info ("rows is " << rows << ", rows_ is " << rows_);
	notmuch_threads_t *threads = notmuch_query_search_threads(query_);
	for (line = 0; notmuch_threads_has_more (threads) && (line < rows_);
		 notmuch_threads_advance (threads), ++line)
	{
		info("grabbing thread #"<<line);
		threads_[line] = thread::create(notmuch_threads_get(threads));
	}
}

search_buffer::~search_buffer() {
	notmuch_query_destroy(query_);
}

search_buffer::ptr search_buffer::create(int rows, int cols, string search) {
	search_buffer::ptr ret(new search_buffer(rows, cols, search));
	return ret;
}

int search_buffer::action(buffer_action_t action_id, int row, int col) {
	int handled = 1;
	// columns don't matter in thread searches since each thread
	// is the entire row and an action anywhere in that row is the
	// same for the entire row
	int offset = page_ * rows_ + row;
	thread::ptr thread = threads_[offset];
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
		case SEARCH_THREADS:
			// start a new search (in a new buffer if it is different that current search... but do we kill this search?)
			break;
		case SEARCH_SAVE:
			// save the current search in a local database... maybe future notmuch feature
			break;
		case SELECT_THREAD:
			// mark it
			break;
		case SELECT_ALL_THREADS:
			// mark all threads
			break;
		case VIEW_THREAD:
			// open the thread view for this thread
			info("open thread screen for: '" << thread->subject() << "'");
			application::get()->thread_screen(thread);
			break;
		default:
			handled = 0;
			break;
	}
	return handled;
}

void search_buffer::resize(int y, int x) {

}

void search_buffer::page(int line_offset) {
	buffer::page(line_offset);
	// re-run the query on the new page
}
const char *search_buffer::get_line(int offset) {
	info("get_line: offset = "<<offset);
	if (thread_lines_.find(offset) == thread_lines_.end()) {
		thread::ptr thread = threads_[offset];
		// eventually this should be replaced by a call to thread::blurb()

		int author_width = 35;
		int subject_width = 40;

		// TODO: notmuch_thread_get_tags(notmuch_thread_t *thread);
		int count = thread->db_message_count();
		string authors = thread->authors();
		string subject = thread->subject();
		std::stringstream ss;
		ss << left << setfill(' ')
		   << setw(author_width) << authors.substr(0,author_width) << " ";
		if (count > 999) {
			ss << 999;
		} else if (count > 1) {
			ss << "(" << count << ")";
			if (count < 10)
				ss << " ";
			if (count < 100)
				ss << " ";
			ss << " ";
		} else {
			ss << "      ";
		}
		ss << setw(subject_width) << subject;
		thread_lines_[offset] = ss.str();
	}
	return thread_lines_[offset].c_str();
}

int search_buffer::nlines(void) {
	return rows_;
}
