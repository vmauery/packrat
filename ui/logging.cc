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
 *     CVS $Id: logging.cpp 380 2003-10-10 20:19:32Z vhmauery $
 * DESCRIPTION: Utilities for Logging
 *     AUTHORS: Marc Strämke, Darren Hart
 *  START DATE: 2003/Jun/28
 *
 *   COPYRIGHT: 2003 by Darren Hart, Vernon Mauery, Marc Strämke, Dirk Hörner
 *     LICENSE: This software is licenced under the Libstk license available with the source as 
 *              license.txt or at http://www.libstk.org/index.php?page=docs/license
 *************************************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <logging.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef THREADED
#include <pthread.h>

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

#define lock(A) pthread_mutex_lock(A)
#define unlock(A) pthread_mutex_unlock(A)

#else

#define lock(A) do { } while(0)
#define unlock(A) do { } while(0)

#endif


using namespace packrat;

boost::shared_ptr<logger> logger::instance_;

boost::shared_ptr<logger> logger::get()
{
	if(!instance_)
		instance_.reset(new logger());
	return instance_;
}

void logger::shutdown() {
	instance_.reset();
}

logger::logger()
{
	severity_names_.resize(LL_LENGTH);
	severity_names_[LL_Info] = "Info";
	severity_names_[LL_Warning] = "Warning";
	severity_names_[LL_Error] = "Error";
	severity_names_[LL_None] = "None";
}

logger::~logger()
{
	log(__FILE__, __LINE__, std::string("destructor"), LL_Info);
}

void logger::add_target(const char *target, log_level min_level, bool truncate)
{
	target_info temp;
	temp.name = target;
	std::ios_base::openmode mode = std::ios_base::out |
			(truncate?std::ios_base::trunc:std::ios_base::app);
	std::ofstream *os = new std::ofstream(target, mode);
	temp.outstream = os;
	temp.min_level = min_level;
	targets.push_back(temp);
	temp.name = NULL;
}

void logger::add_target(std::ostream* target, log_level min_level)
{
	target_info temp;
	temp.outstream = target;
	temp.min_level = min_level;
	targets.push_back(temp);
}

void logger::remove_target(const std::string& target)
{
	Ttargets::iterator iter = std::find(targets.begin(), targets.end(), target);
	targets.erase(iter);
}

void logger::remove_target(std::ostream* target)
{
	Ttargets::iterator iter = std::find(targets.begin(), targets.end(), target);
	targets.erase(iter);
}

void logger::log(const std::string& filename, int line, const std::string& message, 
		log_level severity)
{
	for (Ttargets::iterator iter=targets.begin();iter!=targets.end();iter++)
	{
		if(severity >= iter->min_level)
			*iter->outstream << severity_names_[severity] << "! " << filename << ":" << line 
							 << " \t" << message << std::endl;
			iter->outstream->flush();
	}
	
}

void logger::write(const std::string& filename, int line, const char *msg, size_t len,
		log_level severity)
{
	for (Ttargets::iterator iter=targets.begin();iter!=targets.end();iter++)
	{
		if(severity >= iter->min_level)
			*iter->outstream << severity_names_[severity] << "! " << filename << ":" << line << std::endl;
			iter->outstream->write(msg, len);
			*iter->outstream << std::endl;
			iter->outstream->flush();
	}
	
}

extern "C"
void _log(int severity, const char *file, int line, const char *fmt, ...) {
	va_list ap;
	char *msg;
	int len;

	va_start(ap, fmt);
	len = asprintf(&msg, fmt, ap);
	va_end(ap);
	
    packrat::logger::get()->log(file, line, msg, (log_level)severity);
	free(msg);
}

