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
#include <search_screen.h>
#include <thread_screen.h>
#include <stdio.h>

using namespace packrat;
using std::string;
using std::map;

application::ptr application::instance_;

application::application() {
	string default_path = settings::get("home");
	if (default_path.length() > 0) {
		default_path += "/mail";
	}
	string db_path = settings::get("db_path", default_path);
	if (db_path[0] == '~' && db_path[1] == '/') {
		db_path = settings::get("home") + db_path.substr(1);
	}
	db_ = notmuch_database_open(db_path.c_str(),
			NOTMUCH_DATABASE_MODE_READ_ONLY);
	if (!db_) {
		error("failed to open notmuch database");
		exit(1);
	}
}

application::~application() {
	notmuch_database_close(db_);
}

application::ptr application::get() {
	if (!instance_)
		instance_.reset(new application());

	return instance_;
}

void application::shutdown() {
	screens_.clear();
	next_.reset();
	current_.reset();
	running_ = false;
}

void application::run() {
	// get the last known state?  OOOOHHHH!!!!
	// or just get the inbox search screen_base

	current_ = search_screen("tag:inbox");
	running_ = true;

	current_->enter();
	current_->show();
	while (running_) {
		// show the current screen_base
		if (current_ != next_) {
			current_ = next_;
			current_->enter();
			current_->show();
		}
		// get a key
		int ch = current_->keypressed();
		// process the event globally

		// pass the event to the screen_base
		if (!current_->action(ch)) {
			// toss the event out the window
			info("unhandled key: "<<ch);
		}
	}
	current_.reset();
	next_.reset();
}

void application::close_screen() {
	map<string,screen_base::ptr>::iterator s;
	for (s=screens_.begin(); s!=screens_.end(); s++) {
		if (s->second == current_) {
			screens_.erase(s);
			break;
		}
	}
	info("current: "<<current_->id()<<", open screens: " <<screens_.size());
	if (screens_.empty()) {
		running_ = false;
		return;
	}
	next_ = screens_.begin()->second;
}

void application::next_buffer() {
	map<string,screen_base::ptr>::iterator s;
	for (s=screens_.begin(); s!=screens_.end(); s++) {
		if (s->second == current_) {
			s++;
			if (s == screens_.end()) {
				s = screens_.begin();
			}
			next_ = s->second;
			break;
		}
	}
	info("current: "<<current_->id()<<", next: " <<next_->id());
}

screen_base::ptr application::thread_screen(thread::ptr thread) {
	if (screens_.find(thread->id()) != screens_.end()) {
		next_ = screens_[thread->id()];
	} else {
		next_ = screens_[thread->id()] = thread_screen::create(thread);
		// TODO: to save state, here is where we would do it
	}
	return next_;
}

screen_base::ptr application::search_screen(string search) {
	if (screens_.find(search) != screens_.end()) {
		next_ = screens_[search];
	} else {
		next_ = screens_[search] = search_screen::create(search);
		// TODO: to save state, here is where we would do it
	}
	return next_;
}
