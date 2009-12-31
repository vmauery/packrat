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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <vector>

#include <util.h>

namespace packrat {

std::vector<std::string> explode(std::string delimiter,
	std::string str, size_t count=0);

std::string stripws(std::string str);

#define min(x, y) ({                \
	typeof(x) _min1 = (x);          \
	typeof(y) _min2 = (y);          \
	(void) (&_min1 == &_min2);      \
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                \
	typeof(x) _max1 = (x);          \
	typeof(y) _max2 = (y);          \
	(void) (&_max1 == &_max2);      \
	_max1 > _max2 ? _max1 : _max2; })

#define minmax(val, min, max) ({           \
	typeof(val) __val = (val);            \
	typeof(min) __min = (min);            \
	typeof(max) __max = (max);            \
	(void) (&__val == &__min);            \
	(void) (&__val == &__max);            \
	__val = __val < __min ? __min: __val; \
	__val > __max ? __max: __val; })

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

}

#endif // __UTIL_H__
