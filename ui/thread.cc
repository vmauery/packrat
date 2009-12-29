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

#include <thread.h>
extern "C" {
#include <talloc.h>
}

using namespace packrat;
using std::vector;
using std::string;

/*
0	|	class thread
0	|		render a one line blurb
0	|		render the whole thread (with quoting collapsed)
	|			highlight hyperlinks
	|		re-render parts (expand/collapse messages)
0	|		responds to actions from thread_screen
	|			for marking, deleting, archiving, killing, etc.
	|		render attachments by mime type
	|			text/html
	|			ical (libical0)
	|			text/plain (if we find a patch, render it as such)
	|				first line ^---, second line ^+++, third line ^@@...@@$
	|			text/x-patch
0	|			text/x-... (render as text/plain for starters)
	|				render using vim highlighting???
	|		launch viewer
*/

thread::thread() {
}

thread::thread(notmuch_thread_t *t) : thread_(t), loaded_messages_(false) {
	ctx_ = talloc_new(NULL);
	talloc_reference(ctx_, t);
	id_ = notmuch_thread_get_thread_id(t);
}

thread::~thread() {
	talloc_free(ctx_);
}

thread::ptr thread::create(notmuch_thread_t *t) {
	thread::ptr ret(new thread(t));
	return ret;
}

const string &thread::id() const {
	return id_;
}

string thread::blurb() {
}

void thread::render() {
	if (!loaded_messages_)
		load_messages();

	int offset = 0;
	vector<message::ptr>::iterator i = messages_.begin();
	for (; i != messages_.end(); i++) {
		(*i)->render();
		line_offsets_.push_back(offset);
		offset += (*i)->nlines();
	}
}

int thread::nlines() {
	int lines = 0;
	vector<message::ptr>::iterator i = messages_.begin();
	for (; i != messages_.end(); i++) {
		lines += (*i)->nlines();
	}
	return lines;
}

const char *thread::get_line(int offset) {
	info("get_line("<<offset<<")");
	if (!loaded_messages_) {
		info("loading messages");
		load_messages();
		render();
	}
	info("have "<<messages_.size()<<" messages loaded");
	vector<int>::reverse_iterator i = line_offsets_.rbegin();
	vector<message::ptr>::reverse_iterator m = messages_.rbegin();
	for (; i != line_offsets_.rend() && m != messages_.rend(); i++, m++) {
		if (*i <= offset) {
			return (*m)->get_line(offset - *i);
		}
	}
	return NULL;
}

// remove all tags
void thread::archive() {
}

void thread::add_tag(const char *tag) {
}

void thread::del_tag(const char *tag) {
}

void thread::msg_loader(notmuch_messages_t *msgs, int depth) {
	for (; notmuch_messages_has_more (msgs);
			notmuch_messages_advance (msgs))
	{
		message::ptr m = message::create(
				notmuch_messages_get(msgs), depth
			);
		messages_.push_back(m);
		msg_loader(m->replies(), depth+1);
	}
}

void thread::load_messages() {
	loaded_messages_ = true;
	// load all the messages for this thread
	notmuch_messages_t *msg_iter = notmuch_thread_get_toplevel_messages(thread_);
	msg_loader(msg_iter, 0);
}

int thread::message_count(bool total) {
	if (!loaded_messages_)
		load_messages();
	return messages_.size();
}

int thread::db_message_count() {
	return notmuch_thread_get_total_messages(thread_);
}

string thread::authors() {
	return notmuch_thread_get_authors(thread_);
}

string thread::subject() {
	return notmuch_thread_get_subject(thread_);
}
