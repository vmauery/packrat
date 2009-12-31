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

#include <string>
#include <vector>

#include <packrat.h>
#include <util.h>

namespace packrat {

using std::string;
using std::vector;

vector<string> explode(string delimiter, string str, size_t count) {
	vector<string> ret;
	size_t start = 0, end = 0;
	string sub;
	while (end != string::npos) {
		start = str.find_first_not_of(delimiter, start);
		if (start == string::npos) {
			break;
		}
		end = str.find_first_of(delimiter, start);
		if (end == string::npos) {
			sub = str.substr(start);
		} else {
			sub = str.substr(start, end-start);
		}
		ret.push_back(sub);
		if (ret.size() == count) {
			start = str.find_first_not_of(delimiter, end-start);
			ret.push_back(str.substr(start));
			break;
		}
		start = end + 1;
	}
	return ret;
}

string stripws(string str) {
	const char *delimeter = " \t\r\n";
	size_t start, end;
	start = str.find_first_not_of(delimeter);
	end = str.find_last_not_of(delimeter);
	return str.substr(start, end-start+1);
}

}
