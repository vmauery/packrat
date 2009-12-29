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

extern "C" {
#include <talloc.h>
}
#include <iostream>
#include <sstream>
#include <message.h>
#include <string.h>
#include <errno.h>
#include <gmime/gmime.h>

using namespace packrat;
using std::string;
using std::vector;

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

message::message() {
}

message::message(notmuch_message_t *m, int depth) : message_(m), depth_(depth) {
	ctx_ = talloc_new(NULL);
	talloc_reference(ctx_, m);
}

message::~message() {
	talloc_free(ctx_);
}

message::ptr message::create(notmuch_message_t *t, int depth) {
	message::ptr ret(new message(t, depth));
	return ret;
}

void message::mark_draft(bool mark) {
}

void message::mark_flag(bool mark) {
}

void message::mark_forwarded(bool mark) {
}

void message::mark_replied(bool mark) {
}

void message::mark_read(bool mark) {
}

void message::mark_trash(bool mark) {
}

string message::reply(reply_who_t who) {
	string ret;
	return ret;
}

std::string message::forward() {
	string ret;
	return ret;
}

std::string message::bounce() {
	string ret;
	return ret;
}

std::string message::edit() {
	string ret;
	return ret;
}


std::string message::source() {
	string ret;
	return ret;
}

std::map<std::string,std::string> message::parts() {
	std::map<string,string> ret;
	return ret;
}

std::string message::part(std::string name) {
	string ret;
	return ret;
}

notmuch_messages_t *message::replies() {
	// load all the replies to this message
	notmuch_messages_t *msg_iter = notmuch_message_get_replies(message_);
	return msg_iter;
}

static void show_part_content(GMimeObject *part, vector<string> &lines) {
	GMimeStream *stream_stdout = NULL;
	GMimeStream *stream_filter = NULL;
	GMimeStream *stream_bufout = NULL;
	GMimeStream *stream_cat = NULL;

	GMimeDataWrapper *wrapper;
	const char *charset;

	stream_stdout = g_mime_stream_mem_new();
	stream_cat = g_mime_stream_cat_new();
	g_mime_stream_cat_add_source(GMIME_STREAM_CAT(stream_cat), stream_stdout);
	stream_bufout = g_mime_stream_buffer_new(stream_cat,
		GMIME_STREAM_BUFFER_BLOCK_READ);

	charset = g_mime_object_get_content_type_parameter(part, "charset");

	if (stream_stdout) {
		stream_filter = g_mime_stream_filter_new(stream_stdout);
		g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
				g_mime_filter_crlf_new(FALSE, FALSE));
		if (charset) {
			g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
					g_mime_filter_charset_new(charset, "UTF-8"));
		}
	}

	wrapper = g_mime_part_get_content_object(GMIME_PART(part));
	if (wrapper && stream_filter) {
		g_mime_data_wrapper_write_to_stream(wrapper, stream_filter);

		GByteArray *line = g_byte_array_sized_new(4096);
		while (!g_mime_stream_eos(stream_bufout)) {
			g_mime_stream_buffer_readln(stream_bufout, line);
			int len = line->len;
			if (len <= 0) {
				break;
			}
			if (len > 0 && (line->data[len-1] == '\n' || line->data[len-1] == '\r')) {
				line->data[len-1] = 0;
			}
			if (len > 1 && (line->data[len-2] == '\n' || line->data[len-2] == '\r')) {
				line->data[len-2] = 0;
			}
			line->len = 0;
			lines.push_back((const char *)line->data);
		}

	}
	if (stream_filter)
		g_object_unref(stream_filter);
	if (stream_stdout)
		g_object_unref(stream_stdout);
}

static void show_part(GMimeObject *part, int *part_count, vector<string> &lines) {
	GMimeContentDisposition *disposition;
	GMimeContentType *content_type;
	std::stringstream ss;
	const char *filename = NULL;

	disposition = g_mime_object_get_content_disposition(part);
	content_type = g_mime_object_get_content_type(GMIME_OBJECT(part));
	if (disposition &&
			strcmp(disposition->disposition, GMIME_DISPOSITION_ATTACHMENT) == 0)
	{
		filename = g_mime_part_get_filename(GMIME_PART(part));
		ss << "Attachment: ID: " << *part_count << ", Content-type: "
		   << g_mime_content_type_to_string(content_type)
		   << ", Filename: " << filename;

		lines.push_back(ss.str());
		lines.push_back("================================================================");
	}

	if (g_mime_content_type_is_type(content_type, "text", "*") &&
			!g_mime_content_type_is_type(content_type, "text", "html")) {
		show_part_content(part, lines);
	} else {
		lines.push_back("Non-text content...");
	}

	return;
}

static void show_message_part(GMimeObject *part, int *part_count, vector<string> &lines) {
	*part_count = *part_count + 1;

	if (GMIME_IS_MULTIPART(part)) {
		GMimeMultipart *multipart = GMIME_MULTIPART(part);
		int i;

		for (i = 0; i < g_mime_multipart_get_count(multipart); i++) {
			show_message_part(g_mime_multipart_get_part(multipart, i),
					part_count, lines);
		}
		return;
	}

	if (GMIME_IS_MESSAGE_PART(part)) {
		GMimeMessage *mime_message;

		mime_message = g_mime_message_part_get_message(GMIME_MESSAGE_PART(part));

		show_message_part(g_mime_message_get_mime_part(mime_message),
				part_count, lines);

		return;
	}

	if (!(GMIME_IS_PART(part))) {
		fprintf(stderr, "Warning: Not displaying unknown mime part: %s.\n",
				g_type_name(G_OBJECT_TYPE(part)));
		return;
	}

	show_part(part, part_count, lines);
}

int show_message_body(const char *filename, vector<string> &lines) {
	GMimeStream *stream = NULL;
	GMimeParser *parser = NULL;
	GMimeMessage *mime_message = NULL;
	int ret = 0;
	FILE *file = NULL;
	int part_count = 0;

	file = fopen(filename, "r");
	if (! file) {
		fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
		ret = 1;
		goto DONE;
	}

	stream = g_mime_stream_file_new(file);
	g_mime_stream_file_set_owner(GMIME_STREAM_FILE(stream), FALSE);

	parser = g_mime_parser_new_with_stream(stream);

	mime_message = g_mime_parser_construct_message(parser);

	show_message_part(g_mime_message_get_mime_part(mime_message), &part_count, lines);

DONE:
	if (mime_message)
		g_object_unref(mime_message);

	if (parser)
		g_object_unref(parser);

	if (stream)
		g_object_unref(stream);

	if (file)
		fclose(file);

	return ret;
}

int message::nlines() {
	if (msg_lines_.size() == 0) {
		render();
	}
	return msg_lines_.size();
}

const char *message::get_line(int offset) {
	info("get_line("<<offset<<")");
	if (msg_lines_.size() == 0) {
		render();
	}
	if (offset >= 0 && offset < msg_lines_.size()) {
		return msg_lines_[offset].c_str();
	}
	return NULL;
}

void message::render(int indent) {

	const char *headers[] = {
		"Subject",
		"From",
		"To",
		"Cc",
		"Bcc",
		"Date",
		"List-ID",
	};
	const char *name, *value;
	unsigned int i;

	info("render message:"<<std::endl
			<<"\tid:"<<notmuch_message_get_message_id(message_)<<std::endl
			<<"\tdepth:"<<indent<<std::endl
			<<"\tmatch:"<<notmuch_message_get_flag(message_, NOTMUCH_MESSAGE_FLAG_MATCH)<<std::endl
			<<"\tfilename: "<<notmuch_message_get_filename(message_)
		);

	for (i = 0; i < ARRAY_SIZE(headers); i++) {
		name = headers[i];
		value = notmuch_message_get_header(message_, name);
		if (value && value[0]) {
			std::stringstream line;
			line << name << ": " << value;
			msg_lines_.push_back(line.str());
		}
	}

	msg_lines_.push_back("");

	show_message_body(notmuch_message_get_filename(message_), msg_lines_);
	msg_lines_.push_back("");
	msg_lines_.push_back("");

	vector<string>::iterator m = msg_lines_.begin();
	for (i=0; m!=msg_lines_.end(); m++,i++) {
		info(i<<": "<<*m);
	}
}

/*

	static void
show_messages(notmuch_messages_t *messages, int indent,
		notmuch_bool_t entire_thread)
{
	notmuch_message_t *message;
	notmuch_bool_t match;
	int next_indent;

	for (;
			notmuch_messages_has_more(messages);
			notmuch_messages_advance(messages))
	{
		message = notmuch_messages_get(messages);

		match = notmuch_message_get_flag(message, NOTMUCH_MESSAGE_FLAG_MATCH);

		next_indent = indent;

		if (match || entire_thread) {
			show_message_(message, indent);
			next_indent = indent + 1;
		}

		show_messages(notmuch_message_get_replies(message),
				next_indent, entire_thread);

		notmuch_message_destroy(message);
	}
}
*/
