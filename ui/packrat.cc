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

using namespace packrat;

// TODO: add some config file and command line stuff here

int main(int argc, const char *argv[]) {
	// parse args
	
	// set up logging
	logger::ptr log = logger::get();
	log->add_target("log", LL_Info);
	info("Welcome to packrat, the mail client for digital horders");

	settings::ptr config = settings::load(argc, argv);

	g_mime_init(0);

	// create the application
	application::ptr app = application::get();

	// go!
	app->run();

	return 0;
}
