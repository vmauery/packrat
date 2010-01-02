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

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <notmuch.h>
#include <packrat.h>

namespace packrat {

class screen_base;

class application;
class thread;
class application {
	public:
		typedef boost::shared_ptr<application> ptr;
		typedef boost::weak_ptr<application> wptr;

		static boost::shared_ptr<application> get();

	protected:
		application();
	
	public:
		~application();

		void run();
		void quit() { running_ = false; }
		void shutdown();

		void close_screen();
		void next_buffer();

		// these two methods either create a new screen_base for the
		// requested item or open the existing named screen_base
		// and set that as next_
		boost::shared_ptr<screen_base> thread_screen(boost::shared_ptr<thread> thread);
		boost::shared_ptr<screen_base> search_screen(std::string search);
		boost::shared_ptr<screen_base> editor_screen(std::string file);

		notmuch_database_t *db() { return db_; }

	protected:
		static boost::shared_ptr<application> instance_;

		std::map<std::string,boost::shared_ptr<screen_base> > screens_;
		boost::shared_ptr<screen_base> current_;
		boost::shared_ptr<screen_base> next_;
		bool running_;
		notmuch_database_t *db_;
};

}

#endif // __APPLICATION_H__
