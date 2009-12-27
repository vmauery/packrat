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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <packrat.h>

namespace packrat {

class settings;
class settings {
	public:
		typedef boost::shared_ptr<settings> ptr;
		typedef boost::weak_ptr<settings> wptr;

		// this is called to get the settings value of a key
		static std::string get(std::string name, std::string def=std::string());

		// this is called to parse args and settings file
		static boost::shared_ptr<settings> load(int argc, const char *argv[]);

	protected:
		settings();
		settings(int argc, const char *argv[]);
	
	public:
		~settings();
		std::string item(std::string name, std::string def);

	protected:
		static boost::shared_ptr<settings> instance_;

		std::map<std::string,std::string> items_;
};

}

#endif // __SETTINGS_H__
