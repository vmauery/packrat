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
#include <gmime/gmime.h>
#include <iostream>
#include <string>

using namespace packrat;
using std::string;

void quit(int sig) {
	const char *signame;
	switch (sig) {
		case SIGINT: signame = "SIGINT"; break;
		case SIGQUIT: signame = "SIGQUIT"; break;
		case SIGTERM: signame = "SIGTERM"; break;
		case SIGABRT: signame = "SIGABRT"; break;
		case SIGPIPE: signame = "SIGPIPE"; break;
		case SIGHUP: signame = "SIGHUP"; break;
		case SIGSEGV: signame = "SIGSEGV"; break;
		default: signame = "SIG???"; break;
	}
	application::get()->shutdown();
	error("Received signal " << signame << "(" << sig << "), quitting...");
	exit(1);
}

int main(int argc, const char *argv[]) {
	// parse args
	
	// set up logging
	logger::ptr log = logger::get();
	log->add_target(&std::cerr, LL_Error);

	settings::ptr config = settings::load(argc, argv);

	// set verbosity
	string value = settings::get("verbose");
	log_level v = LL_None;
	if (value == "none") {
		log->remove_target(&std::cerr);
	} else if (value == "info") {
		v = LL_Info;
	} else if (value == "warn") {
		v = LL_Warning;
	} else if (value == "error") {
		v = LL_Error;
	}
	value = settings::get("log", "");
	if (value.length() > 0 && v != LL_None) {
		log->add_target(value.c_str(), v);
	}

	info("Welcome to packrat, the mail client for digital horders");

	g_mime_init(0);

	signal(SIGINT, quit);
	signal(SIGQUIT, quit);
	signal(SIGTERM, quit);
	signal(SIGABRT, quit);
	signal(SIGPIPE, quit);
	signal(SIGHUP, quit);
	signal(SIGSEGV, quit);

	// create the application
	application::ptr app = application::get();

	// go!
	app->run();

	return 0;
}
