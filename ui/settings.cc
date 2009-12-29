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

#include <settings.h>
#include <stdlib.h>

using namespace packrat;
using std::string;
using std::map;

settings::ptr settings::instance_;
map<string,string> items_;

// this is called to get the settings value of a key
string settings::get(string name, string def) {
	string ret;
	if (!instance_) {
		throw string("packrat::settings not instantiated yet");
	}
	return instance_->item(name, def);
}

// this is called to parse args and settings file
settings::ptr settings::load(int argc, const char *argv[]) {
	if (!instance_) {
		instance_.reset(new settings(argc, argv));
	}
	return instance_;
}

settings::settings() { }

settings::settings(int argc, const char *argv[]) {
	items_["db_path"] = "/home/vhmauery/.mail/ltc-imap";
}

settings::~settings() { }

string settings::item(string name, string def) {
	if (items_.find(name) != items_.end()) {
		return items_[name];
	}
	return def;
}

string settings::env(string name) {
	char *v = getenv(name.c_str());
	if (!v)
		return string();
	return v;
}
