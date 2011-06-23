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


#ifndef __SCRUFFY_PP_ITEM_STREAM_TOKEN_STREAM_H__
#define __SCRUFFY_PP_ITEM_STREAM_TOKEN_STREAM_H__

#include <mylang/token_stream.h>

#include <scruffy/pp_item_stream.h>

namespace Scruffy {

using namespace MyCpp;

class PpItemStreamTokenStream : public MyLang::TokenStream
{
protected:
#if 0
    class PositionMarker : public MyLang::TokenStream::PositionMarker
    {
    public:
	Ref<PpItemStream::PositionMarker> pp_pmark;
    };
#endif

    Ref<PpItemStream> pp_stream;
    Ref<PpItemStream::PositionMarker> cur_pmark;

    Ref<String> newline_replacement;

public:
  /* TokenStream interface */

    ConstMemoryDesc getNextToken ()
			   throw (InternalException);

    ConstMemoryDesc getNextToken (Ref<SimplyReferenced> *ret_user_obj,
				  void **ret_user_ptr)
			   throw (InternalException);

    void getPosition (MyLang::TokenStream::PositionMarker *ret_pmark /* non-null */)
	       throw (InternalException);

    void setPosition (MyLang::TokenStream::PositionMarker const *pmark /* non-null */)
	       throw (InternalException);

    // TODO
    MyLang::FilePosition getFilePosition ()
				   throw (InternalException)
    {
	return MyLang::FilePosition ();
    }

  /* (End of TokenStream interface) */

    void setNewlineReplacement (ConstMemoryDesc const &replacement)
    {
	newline_replacement = grab (new String (replacement));
    }

    PpItemStreamTokenStream (PpItemStream *pp_stream);
};

}

#endif /* __SCRUFFY_PP_ITEM_STREAM_TOKEN_STREAM_H__ */

