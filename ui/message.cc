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
#include <string.h>
#include <errno.h>
#include <gmime/gmime.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <sys/types.h>
#include <unistd.h>

#include <message.h>
#include <gmime-filter-reply.h>

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
	GMimeStream *stream_outmem = NULL;
	GMimeStream *stream_filter = NULL;
	GMimeStream *stream_bufout = NULL;
	GMimeStream *stream_cat = NULL;

	GMimeDataWrapper *wrapper;
	const char *charset;

	here();
	stream_outmem = g_mime_stream_mem_new();
	stream_cat = g_mime_stream_cat_new();
	g_mime_stream_cat_add_source(GMIME_STREAM_CAT(stream_cat), stream_outmem);
	stream_bufout = g_mime_stream_buffer_new(stream_cat,
		GMIME_STREAM_BUFFER_BLOCK_READ);

	here();
	charset = g_mime_object_get_content_type_parameter(part, "charset");

	if (stream_outmem) {
		stream_filter = g_mime_stream_filter_new(stream_outmem);
		g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
				g_mime_filter_crlf_new(FALSE, FALSE));
		if (charset) {
			g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
					g_mime_filter_charset_new(charset, "UTF-8"));
		}
	}

	here();
	wrapper = g_mime_part_get_content_object(GMIME_PART(part));
	if (wrapper && stream_filter) {
		g_mime_data_wrapper_write_to_stream(wrapper, stream_filter);

	here();
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
	if (stream_cat)
		g_object_unref(stream_cat);
	if (stream_bufout)
		g_object_unref(stream_bufout);
	if (stream_outmem)
		g_object_unref(stream_outmem);
}

static void show_part_content_reply(GMimeObject *part, FILE *file) {
	GMimeStream *stream_fileout = NULL;
	GMimeStream *stream_filter = NULL;

	GMimeDataWrapper *wrapper;
	const char *charset;

	here();
	stream_fileout = g_mime_stream_file_new(file);

	charset = g_mime_object_get_content_type_parameter(part, "charset");

	if (stream_fileout) {
		stream_filter = g_mime_stream_filter_new(stream_fileout);
		g_mime_stream_file_set_owner (GMIME_STREAM_FILE (stream_fileout), FALSE);		
		if (charset) {
			g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
					g_mime_filter_charset_new(charset, "UTF-8"));
		}
		g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
				g_mime_filter_reply_new(TRUE));
	}

	wrapper = g_mime_part_get_content_object(GMIME_PART(part));
	if (wrapper && stream_filter) {
		g_mime_data_wrapper_write_to_stream(wrapper, stream_filter);
	}
	if (stream_filter)
		g_object_unref(stream_filter);
	if (stream_fileout)
		g_object_unref(stream_fileout);
}

static void show_part(GMimeObject *part, int *part_count, vector<string> &lines) {
	GMimeContentDisposition *disposition;
	GMimeContentType *content_type;
	std::stringstream ss;
	const char *filename = NULL;

	here();
	disposition = g_mime_object_get_content_disposition(part);
	content_type = g_mime_object_get_content_type(GMIME_OBJECT(part));
	if (disposition &&
			strcmp(disposition->disposition, GMIME_DISPOSITION_ATTACHMENT) == 0)
	{
		filename = g_mime_part_get_filename(GMIME_PART(part));
		ss << "Attachment: ID: " << *part_count << ", Content-type: "
		   << g_mime_content_type_to_string(content_type)
		   << ", Filename: " << filename;

	here();
		lines.push_back(ss.str());
		lines.push_back("================================================================");
	}

	here();
	if (g_mime_content_type_is_type(content_type, "text", "*") &&
			!g_mime_content_type_is_type(content_type, "text", "html")) {
	here();
		show_part_content(part, lines);
	} else {
	here();
		lines.push_back(string("Non-text attachment: ") +
			g_mime_content_type_to_string(content_type));
	}

	return;
}

static void show_part_reply(GMimeObject *part, int *part_count, FILE *file) {
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

		fprintf(file, "%s\n", ss.str().c_str());
		fprintf(file, "================================================================\n");
	}

	if (g_mime_content_type_is_type(content_type, "text", "*") &&
			!g_mime_content_type_is_type(content_type, "text", "html")) {
		show_part_content_reply(part, file);
	} else {
		fprintf(file, "Non-text attachment: %s\n",
			g_mime_content_type_to_string(content_type));
	}

	return;
}

static void show_message_part(GMimeObject *part, int *part_count,
		boost::function<void(GMimeObject*,int*)> &callback) {
	*part_count = *part_count + 1;

	here();
	if (GMIME_IS_MULTIPART(part)) {
		GMimeMultipart *multipart = GMIME_MULTIPART(part);
		int i;

		for (i = 0; i < g_mime_multipart_get_count(multipart); i++) {
			show_message_part(g_mime_multipart_get_part(multipart, i),
					part_count, callback);
		}
		return;
	}

	if (GMIME_IS_MESSAGE_PART(part)) {
		GMimeMessage *mime_message;

		mime_message = g_mime_message_part_get_message(GMIME_MESSAGE_PART(part));

		show_message_part(g_mime_message_get_mime_part(mime_message),
				part_count, callback);

		return;
	}

	if (!(GMIME_IS_PART(part))) {
		fprintf(stderr, "Warning: Not displaying unknown mime part: %s.\n",
				g_type_name(G_OBJECT_TYPE(part)));
		return;
	}

	callback(part, part_count);
}

static int show_message_body(const char *filename,
		boost::function<void(GMimeObject*,int*)> &callback) {
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

	here();
	stream = g_mime_stream_file_new(file);
	g_mime_stream_file_set_owner(GMIME_STREAM_FILE(stream), FALSE);

	parser = g_mime_parser_new_with_stream(stream);

	mime_message = g_mime_parser_construct_message(parser);

	show_message_part(g_mime_message_get_mime_part(mime_message),
		&part_count, callback);

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

	boost::function<void(GMimeObject*,int*)> callback =
		boost::bind(show_part, _1, _2, boost::ref(msg_lines_));
	show_message_body(notmuch_message_get_filename(message_), callback);
	msg_lines_.push_back("");
	msg_lines_.push_back("");

	vector<string>::iterator m = msg_lines_.begin();
	for (i=0; m!=msg_lines_.end(); m++,i++) {
		info(i<<": "<<*m);
	}
}

static const struct {
    const char *header;
    const char *fallback;
    GMimeRecipientType recipient_type;
} reply_to_map[] = {
    { "reply-to", "from", GMIME_RECIPIENT_TYPE_TO  },
    { "to",         NULL, GMIME_RECIPIENT_TYPE_TO  },
    { "cc",         NULL, GMIME_RECIPIENT_TYPE_CC  },
    { "bcc",        NULL, GMIME_RECIPIENT_TYPE_BCC }
};

static int
address_is_users(const char *address) {
    string primary;
    vector<string> other;
	vector<string>::iterator i;

	primary = settings::get("primary_email", "");
	if (strcasecmp(primary.c_str(), address) == 0)
		return 1;

	other = explode(" \t,:;", settings::get("other_email", ""));
	for (i=other.begin(); i!=other.end(); i++) {
		if (strcasecmp(i->c_str(), address) == 0)
			return 1;
	}
	return 0;
}

/* For each address in 'list' that is not configured as one of the
 * user's addresses in 'config', add that address to 'message' as an
 * address of 'type'.
 *
 * The first address encountered that *is* the user's address will be
 * returned, (otherwise NULL is returned).
 */
static const char *
add_recipients_for_address_list(GMimeMessage *message,
				 GMimeRecipientType type,
				 InternetAddressList *list) {
    InternetAddress *address;
    int i;
    const char *ret = NULL;

	for (i = 0; i < internet_address_list_length (list); i++) {
		address = internet_address_list_get_address(list, i);
		if (INTERNET_ADDRESS_IS_GROUP(address)) {
			InternetAddressGroup *group;
			InternetAddressList *group_list;

			group = INTERNET_ADDRESS_GROUP(address);
			group_list = internet_address_group_get_members(group);
			if (group_list == NULL)
				continue;

			add_recipients_for_address_list(message,
					type, group_list);
		} else {
			InternetAddressMailbox *mailbox;
			const char *name;
			const char *addr;

			mailbox = INTERNET_ADDRESS_MAILBOX(address);

			name = internet_address_get_name(address);
			addr = internet_address_mailbox_get_addr(mailbox);

			if (address_is_users (addr)) {
				if (ret == NULL)
					ret = addr;
			} else {
				g_mime_message_add_recipient(message, type, name, addr);
			}
		}
	}

	return ret;
}

/* For each address in 'recipients' that is not configured as one of
 * the user's addresses in 'config', add that address to 'message' as
 * an address of 'type'.
 *
 * The first address encountered that *is* the user's address will be
 * returned, (otherwise NULL is returned).
 */
	static const char *
add_recipients_for_string(GMimeMessage *message,
		GMimeRecipientType type,
		const char *recipients)
{
	InternetAddressList *list;

	list = internet_address_list_parse_string(recipients);
	if (list == NULL)
		return NULL;

	return add_recipients_for_address_list(message, type, list);
}

string message::reply(reply_who_t who) {
	string header, header2;
    GMimeMessage *msg;
	const char *recipients, *from_addr = NULL;
	int i;

	/* The 1 means we want headers in a "pretty" order. */
	msg = g_mime_message_new(1);
	if (msg == NULL) {
		error("out of memory");
		return string();
	}

	header = notmuch_message_get_header(message_, "subject");
	if (strcasecmp(header.substr(0,3).c_str(), "re:") != 0)
		header = string("Re: ") + header;
	g_mime_message_set_subject(msg, header.c_str());

	for (i = 0; i < ARRAY_SIZE(reply_to_map); i++) {
		const char *addr;

		recipients = notmuch_message_get_header(message_,
				reply_to_map[i].header);
		if ((recipients == NULL || recipients[0] == '\0') && reply_to_map[i].fallback)
			recipients = notmuch_message_get_header(message_,
					reply_to_map[i].fallback);

		addr = add_recipients_for_string(msg,
				reply_to_map[i].recipient_type,
				recipients);
		if (from_addr == NULL)
			from_addr = addr;
	}

	if (from_addr != NULL)
		header = from_addr;
	else
		header = settings::get("primary_email", "");
	

	header = settings::get("full_name") + "<" + header;
	g_mime_object_set_header(GMIME_OBJECT(msg),
			"From", header.c_str());

	if (settings::get("bcc_self", "yes") == "yes")
		g_mime_object_set_header(GMIME_OBJECT(msg), "Bcc",
				settings::get("primary_email", "").c_str());

	header2 = string("<") +
			notmuch_message_get_message_id(message_) + string(">");

	g_mime_object_set_header(GMIME_OBJECT(msg),
			"In-Reply-To", header2.c_str());

	header = notmuch_message_get_header(message_, "references");
	if (header.length() > 0)
		header += " " + header2;
	else
		header = header2;
	g_mime_object_set_header(GMIME_OBJECT(msg),
			"References", header.c_str());

	char tmpfname[256];
	snprintf(tmpfname, sizeof(tmpfname), "/tmp/packrat-%d-msg.XXXXXX", getpid());
	int fd_out = mkstemp(tmpfname);
	info("reply mail is in "<<tmpfname);
	// XXX error checking
	FILE *file_out = fdopen(fd_out, "w");
	fprintf(file_out, "%s", g_mime_object_to_string(GMIME_OBJECT(msg)));

	g_object_unref(G_OBJECT(msg));

	boost::function<void(GMimeObject*,int*)> callback =
		boost::bind(show_part_reply, _1, _2, file_out);
	show_message_body(notmuch_message_get_filename(message_), callback);
	// what do we do with the file now... open the editor
	// who is in control?
	fclose(file_out);
	return tmpfname;
}

