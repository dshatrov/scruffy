/*  MyLang - Utility library for writing parsers
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


#ifndef SCRUFFY__BYTE_STREAM__H__
#define SCRUFFY__BYTE_STREAM__H__


#include <libmary/libmary.h>


namespace Scruffy {

using namespace M;

class ByteStream : public StReferenced
{
public:
    class PositionMarker : public StReferenced
    {
    };

    enum ByteResult {
	ByteNormal = 0,
	ByteEof
    };

    virtual ByteResult getNextByte (char *c)
			     throw (InternalException) = 0;

    virtual StRef<PositionMarker> getPosition ()
                                        throw (InternalException) = 0;

    virtual void setPosition (PositionMarker *pmark)
                       throw (InternalException) = 0;
};

}


#endif /* SCRUFFY__BYTE_STREAM__H__ */

