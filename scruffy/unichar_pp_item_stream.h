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


#ifndef __SCRUFYY__UNICHAR_PP_ITEM_STREAM_H__
#define __SCRUFYY__UNICHAR_PP_ITEM_STREAM_H__

#include <mycpp/internal_exception.h>

#include <mylang/unichar_stream.h>

#include <scruffy/pp_item_stream.h>
#include <scruffy/preprocessor.h>

namespace Scruffy {

using namespace MyCpp;

class UnicharPpItemStream : public PpItemStream
{
protected:
    class PositionMarker : public PpItemStream::PositionMarker
    {
    public:
	Ref<MyLang::UnicharStream::PositionMarker> unichar_pmark;
    };

    Ref<MyLang::UnicharStream> unichar_stream;
    CppPreprocessor::PpTokenMatchFunc pp_token_match_func;

    Ref<MyLang::UnicharStream::PositionMarker> cur_pmark;

public:
  /* PpItemStream interface */

    PpItemResult getNextItem (Ref<PpItem> *pp_item)
			    throw (InternalException,
				   ParsingException);

    PpItemResult getHeaderName (Ref<PpToken_HeaderName> *ret_hn_token)
			 throw (ParsingException,
				InternalException);

    Ref<PpItemStream::PositionMarker> getPosition ()
			    throw (InternalException);

    void setPosition (PpItemStream::PositionMarker *pmark)
			    throw (InternalException);

    MyLang::FilePosition getFpos (PpItemStream::PositionMarker *pmark);

    MyLang::FilePosition getFpos ();

  /* (End of PpItemStream interface) */

    UnicharPpItemStream (MyLang::UnicharStream *unichar_stream,
			 CppPreprocessor::PpTokenMatchFunc pp_token_match_func)
			    throw (InternalException);
};

}

#endif /* __SCRUFYY__UNICHAR_PP_ITEM_STREAM_H__ */
