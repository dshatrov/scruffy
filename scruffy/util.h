/*  Scruffy - C/C++ parser and source code analyzer
    Copyright (C) 2011 Dmitry Shatrov

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef __SCRUFFY_UTIL_H__
#define __SCRUFFY_UTIL_H__

#include <mycpp/internal_exception.h>

namespace Scruffy {

struct StringNumberPair
{
    const char    *string;
    unsigned long  number;
};

void validateStringNumberPairs (const StringNumberPair *pairs,
				unsigned long           num_pairs)
			 throw (MyCpp::InternalException);

bool matchStringNumberPairs (const char             *str,
			     unsigned long          *ret_number,
			     const StringNumberPair *pairs,
			     unsigned long           num_pairs);

void validateStrings (const char    **strs,
		      unsigned long   num_strs)
	       throw (MyCpp::InternalException);

unsigned long matchStrings (const char     *str,
			    const char    **strs,
			    unsigned long   num_strs);

bool numbersAreZero (const unsigned long *number_set,
		     unsigned long        nnumbers);

bool numberIsMaxOf (unsigned long        number,
		    const unsigned long *number_set,
		    unsigned long        nnumbers);

unsigned long maxNumberOf (const unsigned long *number_set,
			   unsigned long        nnumbers);

}

#endif /* __SCRUFFY_UTIL_H__ */

