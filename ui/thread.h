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

#ifndef __THREAD_H__
#define __THREAD_H__

#include <packrat.h>
#include <notmuch.h>
#include <message.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace packrat {

class thread {
	public:
		typedef boost::shared_ptr<thread> ptr;
		typedef boost::weak_ptr<thread> wptr;

		static boost::shared_ptr<thread> create(notmuch_thread_t *t);

	protected:
		thread();
		thread(notmuch_thread_t *t);

	public:
		~thread();

		const std::string &id() const;
		std::string blurb();
		void render();

		// remove all tags
		void archive();

		void add_tag(const char *tag);
		void del_tag(const char *tag);

		void load_messages();
		int message_count(bool total=true);
		// db interface
		int db_message_count();
		std::string authors();
		std::string subject();

		// here offset is the actual offset into the thread's lines
		const char *get_line(int offset);

		int nlines(void);

	protected:
		//message::ptr line_to_message_(int offset);
		void msg_loader(notmuch_messages_t *msgs, int depth);

		notmuch_thread_t *thread_;
		std::string id_;
		// these two are in sync -- each message has one offset, the line offset it starts at
		std::vector<message::ptr> messages_;
		std::vector<int> line_offsets_;
		bool loaded_messages_;
		void *ctx_;
};

}

#endif // __THREAD_H__
