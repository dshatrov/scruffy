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


#ifndef __SCRUFFY__PHASE3_ITEM_STREAM_H__    
#define __SCRUFFY__PHASE3_ITEM_STREAM_H__    

#include <mycpp/list.h>
#include <mycpp/internal_exception.h>

#include <scruffy/pp_item_stream.h>

namespace Scruffy {

using namespace MyCpp;

class Phase3ItemStream : public PpItemStream
{
protected:
    class PositionMarker : public PpItemStream::PositionMarker
    {
    public:
	List< Ref<PpItem> >::Element *pp_el;
	Ref<PpItemStream::PositionMarker> stream_pmark;
    };

    List< Ref<PpItem> > pp_items;
    Ref<PpItemStream> pp_stream;

    List< Ref<PpItem> >::Element *cur_pp;
    Ref<PpItemStream::PositionMarker> cur_pmark;

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

    void prependItems (List< Ref<PpItem> > *items);

    void trim ();

    bool hasPrependedItems ();

    Phase3ItemStream (PpItemStream *pp_stream)
	       throw (InternalException);
};

}

#endif /* __SCRUFFY__PHASE3_ITEM_STREAM_H__ */

