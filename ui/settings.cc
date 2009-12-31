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
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <fstream>
#include <iostream>
#include <vector>

#include <settings.h>

using namespace packrat;
using std::string;
using std::vector;
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

#define setup_all_opt(id, arg, val) { #id, arg, NULL, val }
#define setup_opt(id) setup_all_opt(id, 1, 0)
settings::settings(int argc, const char *argv[]) {
	// when this function is called, only
	// error will work because application has only
	// registered at LL_Error on cerr until we know
	// what the user wants to see and where to put it

	string home = env("HOME");
	if (home.length() == 0) {
		string username = env("USERNAME");
		if (username.length() > 0)
			home = string("/home/")+username;
	}
	items_["home"] = home;
	string defconfig = items_["config"] = home + "/.packratrc";
	items_["verbose"] = "none";
	items_["log"] = home+"/packrat.log";

	struct option longopts[] = {
		setup_opt(db_path),
		setup_opt(full_name),
		setup_opt(primary_email),
		setup_opt(other_email),
		setup_opt(bcc_self),
		setup_all_opt(config, 1, 'c'),
		setup_all_opt(verbose, 1, 'v'),
		setup_all_opt(log, 1, 'l'),
	};
	char opts[ARRAY_SIZE(longopts)*3+1];
	int i, j = 0;

	for (i=0;i<(int)ARRAY_SIZE(longopts);i++) {
		int c = longopts[i].val;
		if (isalnum(c) || ispunct(c)) {
			opts[j++] = c;
			if (longopts[i].has_arg > 0)
				opts[j++] = ':';
			if (longopts[i].has_arg > 1)
				opts[j++] = ':';
		}
	}
	opts[j] = 0;

	i = -1;
	while ((j = getopt_long(argc, (char* const*)argv, opts, longopts, &i)) != -1) {
		if (i < 0) {
			for (i=0; i<(int)ARRAY_SIZE(longopts); i++) {
				if (longopts[i].val == j)
					break;
			}
		}
		if (longopts[i].has_arg == 1) {
			items_[longopts[i].name] = optarg;
		} else if (longopts[i].has_arg == 2) {
			if (optarg) {
				items_[longopts[i].name] = optarg;
			} else {
				items_[longopts[i].name] = "yes";
			}
		} else {
			items_[longopts[i].name] = "yes";
		}
		i = -1;
	}

	// parse the config file
	std::ifstream conf(items_["config"].c_str());
	if (!conf) {
		if (items_["config"] == defconfig) {
			error("no config file found... this may not work well");
		} else {
			error("failed to open config file");
			exit(1);
		}
	} else {
		string line, full_line;
		vector<string> parts;
		while (!conf.eof()) {
			getline(conf, full_line);
			line = full_line.substr(0, full_line.find('#'));
			if (line.length() == 0)
				continue;
			parts = explode("=", line, 1);
			if (parts.size() != 2) {
				error("Ignoring junk: " << full_line);
			}
			items_[stripws(parts[0])] = stripws(parts[1]);
		}
	}
}
#undef setup_all_opt
#undef setup_opt

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
