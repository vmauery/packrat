/* notmuch - Not much of an email program, (just index and search)
 *
 * Copyright © 2009 Carl Worth
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
 * Author: Carl Worth <cworth@cworth.org>
 */

#include "notmuch-client.h"

static const char *
_get_tags_as_string (void *ctx, notmuch_message_t *message)
{
    notmuch_tags_t *tags;
    int first = 1;
    const char *tag;
    char *result;

    result = talloc_strdup (ctx, "");
    if (result == NULL)
	return NULL;

    for (tags = notmuch_message_get_tags (message);
	 notmuch_tags_has_more (tags);
	 notmuch_tags_advance (tags))
    {
	tag = notmuch_tags_get (tags);

	result = talloc_asprintf_append (result, "%s%s",
					 first ? "" : " ", tag);
	first = 0;
    }

    return result;
}

/* Get a nice, single-line summary of message. */
static const char *
_get_one_line_summary (void *ctx, notmuch_message_t *message)
{
    const char *from;
    time_t date;
    const char *relative_date;
    const char *tags;

    from = notmuch_message_get_header (message, "from");

    date = notmuch_message_get_date (message);
    relative_date = notmuch_time_relative_date (ctx, date);

    tags = _get_tags_as_string (ctx, message);

    return talloc_asprintf (ctx, "%s (%s) (%s)",
			    from, relative_date, tags);
}

static void
show_part_content (GMimeObject *part)
{
    GMimeStream *stream_stdout = g_mime_stream_file_new (stdout);
    GMimeStream *stream_filter = NULL;
    GMimeDataWrapper *wrapper;
    const char *charset;

    charset = g_mime_object_get_content_type_parameter (part, "charset");

    if (stream_stdout) {
	g_mime_stream_file_set_owner (GMIME_STREAM_FILE (stream_stdout), FALSE);
	stream_filter = g_mime_stream_filter_new(stream_stdout);
	g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
				 g_mime_filter_crlf_new(FALSE, FALSE));
        if (charset) {
          g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
                                   g_mime_filter_charset_new(charset, "UTF-8"));
        }
    }

    wrapper = g_mime_part_get_content_object (GMIME_PART (part));
    if (wrapper && stream_filter)
	g_mime_data_wrapper_write_to_stream (wrapper, stream_filter);
    if (stream_filter)
	g_object_unref(stream_filter);
    if (stream_stdout)
	g_object_unref(stream_stdout);
}

static void
show_part (GMimeObject *part, int *part_count)
{
    GMimeContentDisposition *disposition;
    GMimeContentType *content_type;

    disposition = g_mime_object_get_content_disposition (part);
    if (disposition &&
	strcmp (disposition->disposition, GMIME_DISPOSITION_ATTACHMENT) == 0)
    {
	const char *filename = g_mime_part_get_filename (GMIME_PART (part));
	content_type = g_mime_object_get_content_type (GMIME_OBJECT (part));

	printf ("\fattachment{ ID: %d, Content-type: %s\n",
		*part_count,
		g_mime_content_type_to_string (content_type));
	printf ("Attachment: %s (%s)\n", filename,
		g_mime_content_type_to_string (content_type));

	if (g_mime_content_type_is_type (content_type, "text", "*") &&
	    !g_mime_content_type_is_type (content_type, "text", "html"))
	{
	    show_part_content (part);
	}

	printf ("\fattachment}\n");

	return;
    }

    content_type = g_mime_object_get_content_type (GMIME_OBJECT (part));

    printf ("\fpart{ ID: %d, Content-type: %s\n",
	    *part_count,
	    g_mime_content_type_to_string (content_type));

    if (g_mime_content_type_is_type (content_type, "text", "*") &&
	!g_mime_content_type_is_type (content_type, "text", "html"))
    {
	show_part_content (part);
    }
    else
    {
	printf ("Non-text part: %s\n",
		g_mime_content_type_to_string (content_type));
    }

    printf ("\fpart}\n");
}

static void
show_message (void *ctx, notmuch_message_t *message, int indent)
{
    const char *headers[] = {
	"Subject", "From", "To", "Cc", "Bcc", "Date"
    };
    const char *name, *value;
    unsigned int i;

    printf ("\fmessage{ id:%s depth:%d match:%d filename:%s\n",
	    notmuch_message_get_message_id (message),
	    indent,
	    notmuch_message_get_flag (message, NOTMUCH_MESSAGE_FLAG_MATCH),
	    notmuch_message_get_filename (message));

    printf ("\fheader{\n");

    printf ("%s\n", _get_one_line_summary (ctx, message));

    for (i = 0; i < ARRAY_SIZE (headers); i++) {
	name = headers[i];
	value = notmuch_message_get_header (message, name);
	if (value)
	    printf ("%s: %s\n", name, value);
    }

    printf ("\fheader}\n");
    printf ("\fbody{\n");

    show_message_body (notmuch_message_get_filename (message), show_part);

    printf ("\fbody}\n");

    printf ("\fmessage}\n");
}


static void
show_messages (void *ctx, notmuch_messages_t *messages, int indent,
	       notmuch_bool_t entire_thread)
{
    notmuch_message_t *message;
    notmuch_bool_t match;
    int next_indent;

    for (;
	 notmuch_messages_has_more (messages);
	 notmuch_messages_advance (messages))
    {
	message = notmuch_messages_get (messages);

	match = notmuch_message_get_flag (message, NOTMUCH_MESSAGE_FLAG_MATCH);

	next_indent = indent;

	if (match || entire_thread) {
	    show_message (ctx, message, indent);
	    next_indent = indent + 1;
	}

	show_messages (ctx, notmuch_message_get_replies (message),
		       next_indent, entire_thread);

	notmuch_message_destroy (message);
    }
}

int
notmuch_show_command (void *ctx, unused (int argc), unused (char *argv[]))
{
    notmuch_config_t *config;
    notmuch_database_t *notmuch;
    notmuch_query_t *query;
    notmuch_threads_t *threads;
    notmuch_thread_t *thread;
    notmuch_messages_t *messages;
    char *query_string;
    int entire_thread = 0;
    int i;

    for (i = 0; i < argc && argv[i][0] == '-'; i++) {
	if (strcmp (argv[i], "--") == 0) {
	    i++;
	    break;
	}
        if (strcmp(argv[i], "--entire-thread") == 0) {
	    entire_thread = 1;
	} else {
	    fprintf (stderr, "Unrecognized option: %s\n", argv[i]);
	    return 1;
	}
    }

    argc -= i;
    argv += i;

    config = notmuch_config_open (ctx, NULL, NULL);
    if (config == NULL)
	return 1;

    query_string = query_string_from_args (ctx, argc, argv);
    if (query_string == NULL) {
	fprintf (stderr, "Out of memory\n");
	return 1;
    }

    if (*query_string == '\0') {
	fprintf (stderr, "Error: notmuch show requires at least one search term.\n");
	return 1;
    }

    notmuch = notmuch_database_open (notmuch_config_get_database_path (config),
				     NOTMUCH_DATABASE_MODE_READ_ONLY);
    if (notmuch == NULL)
	return 1;

    query = notmuch_query_create (notmuch, query_string);
    if (query == NULL) {
	fprintf (stderr, "Out of memory\n");
	return 1;
    }

    for (threads = notmuch_query_search_threads (query);
	 notmuch_threads_has_more (threads);
	 notmuch_threads_advance (threads))
    {
	thread = notmuch_threads_get (threads);

	messages = notmuch_thread_get_toplevel_messages (thread);

	if (messages == NULL)
	    INTERNAL_ERROR ("Thread %s has no toplevel messages.\n",
			    notmuch_thread_get_thread_id (thread));

	show_messages (ctx, messages, 0, entire_thread);

	notmuch_thread_destroy (thread);
    }

    notmuch_query_destroy (query);
    notmuch_database_close (notmuch);

    return 0;
}
