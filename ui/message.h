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

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <packrat.h>
#include <notmuch.h>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace packrat {

typedef enum {
	msg_reply_sender,
	msg_reply_all,
	msg_reply_list,
	msg_reply_none,
} reply_who_t;

class message {
	public:
		typedef boost::shared_ptr<message> ptr;
		typedef boost::weak_ptr<message> wptr;

		static boost::shared_ptr<message> create(notmuch_message_t *t, int depth=0);
	
	protected:
		message();
		message(notmuch_message_t *m, int depth);
	
	public:
		~message();

		// for these, the mark means it marks it as such, false means it removes the mark
		void mark_draft(bool mark=true);
		void mark_flag(bool mark=true);
		void mark_forwarded(bool mark=true); // mark_passed
		void mark_replied(bool mark=true);
		void mark_read(bool mark=true);
		void mark_trash(bool mark=true);

		std::string reply(reply_who_t who);
		std::string forward();
		std::string bounce();
		std::string edit();

		std::string source();
		std::map<std::string,std::string> parts();
		std::string part(std::string name);

		notmuch_messages_t *replies();

		void render(int indent=0);
		int nlines();
		const char *get_line(int offset);

	protected:
		notmuch_message_t *message_;
		std::vector<std::string> msg_lines_;
		int depth_;
		void *ctx_;
};

}

#endif // __MESSAGE_H__

