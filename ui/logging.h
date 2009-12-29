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

/**************************************************************************************************
 *     CVS $Id: logging.h 380 2003-10-10 20:19:32Z vhmauery $
 * DESCRIPTION: Utilities for Logging
 *     AUTHORS: Marc Strämke, Darren Hart
 *  START DATE: 2003/JUN/28
 *
 *   COPYRIGHT: 2003 by Darren Hart, Vernon Mauery, Marc Strämke, Dirk Hörner
 *     LICENSE: This software is licenced under the Libstk license available with the source as
 *              license.txt or at http://www.libstk.org/index.php?page=docs/license
 *************************************************************************************************/

#ifndef _LOGGING_H
#define _LOGGING_H

#define HAVE_LOGGING 1

#ifdef HAVE_LOGGING

#ifdef __cplusplus

#include <sstream>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>
#include <packrat.h>

namespace packrat
{
    typedef enum
    {
        LL_Info,
        LL_Warning,
        LL_Error,
        LL_None,
        // need to know the length of the enum
        LL_LENGTH
    } log_level;

    class logger;
    class logger
    {
    private:
        class target_info
        {
			public:
				const char *name;
				std::ostream* outstream;
				int min_level;      // Everything Greater or equal to this gets logged

				target_info() : name(NULL) {}
				~target_info()
				{
					if (name != NULL)
					{
						delete outstream;
					}
				}
				bool operator==(target_info &rhs) const
				{
					return rhs.outstream == outstream;
				}
				bool operator==(std::ostream *rhs) const
				{
					return rhs == outstream;
				}
				bool operator==(const std::string& rhs) const
				{
					return rhs == name;
				}
        };

        static boost::shared_ptr<logger> instance_;
        typedef std::list<target_info> Ttargets;
        Ttargets targets;

        std::vector<std::string> severity_names_;

    public:
		typedef boost::shared_ptr<logger> ptr;
        static boost::shared_ptr<logger> get();
		static void shutdown();
        logger();
        ~logger();
        void add_target(std::ostream* target, log_level min_level);
        void add_target(const char *target, log_level min_level, bool truncate=false);
        void remove_target(std::ostream* target);
        void remove_target(const std::string& target);
        void log(const std::string& filename, int line, const std::string& message,
                log_level severity);
        void write(const std::string& filename, int line, const char *message,
				size_t len, log_level severity);
    };

}

#define log_dump(msg,len) \
	packrat::logger::get()->write(__FILE__, __LINE__, msg, len, packrat::LL_Info)

#define info(msg) { \
    std::ostringstream stream; \
    stream << msg ; \
    packrat::logger::get()->log(__FILE__, __LINE__, stream.str(), packrat::LL_Info);\
}

#define warn(msg) { \
    std::ostringstream stream; \
    stream << msg ; \
    packrat::logger::get()->log(__FILE__, __LINE__, stream.str(), packrat::LL_Warning);\
}
#define error(msg) { \
    std::ostringstream stream; \
    stream << msg ; \
    packrat::logger::get()->log(__FILE__, __LINE__, stream.str(), packrat::LL_Error);\
}
#define here(A) info("")

#else // __cplusplus

#define info(FMT, ...) _log(0, __FILE__, __LINE__, FMT, ##__VA_ARGS__)
#define warn(FMT, ...) _log(1, __FILE__, __LINE__, FMT, ##__VA_ARGS__)
#define error(FMT, ...) _log(2, __FILE__, __LINE__, FMT, ##__VA_ARGS__)
#define here(A) info("")
void _log(int severity, const char *file, int line, const char *fmt, ...);

#endif // __cplusplus

#else // LOGGING
#define here(A)
#define info(msg)
#define warn(msg)
#define error(msg)
#endif // LOGGING

#endif
