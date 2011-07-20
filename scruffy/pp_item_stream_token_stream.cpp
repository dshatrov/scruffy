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


#include <scruffy/pp_item_stream_token_stream.h>
#include <scruffy/preprocessor_util.h>

using namespace MyCpp;
using namespace MyLang;

namespace Scruffy {

namespace {

    void pmark_copy_func (TokenStream::PositionMarker       *to   /* non-null */,
			  TokenStream::PositionMarker const *from /* non-null */);

    void pmark_release_func (TokenStream::PositionMarker *pmark /* non-null */);

    void pmark_copy_func (TokenStream::PositionMarker       * const to   /* non-null */,
			  TokenStream::PositionMarker const * const from /* non-null */)
    {
	PpItemStream::PositionMarker * const inner_pmark =
		static_cast <PpItemStream::PositionMarker*> (from->body.ptr);
	inner_pmark->ref ();
	to->body.ptr = inner_pmark;

	to->body.copy_func = pmark_copy_func;
	to->body.release_func = pmark_release_func;
    }

    void pmark_release_func (TokenStream::PositionMarker * const pmark /* non-null */)
    {
	PpItemStream::PositionMarker * const inner_pmark =
		static_cast <PpItemStream::PositionMarker*> (pmark->body.ptr);
	inner_pmark->unref ();
    }

}

ConstMemoryDesc
PpItemStreamTokenStream::getNextToken ()
    throw (InternalException)
{
    return getNextToken (NULL, NULL);
}
    
ConstMemoryDesc
PpItemStreamTokenStream::getNextToken (Ref<SimplyReferenced> *ret_user_obj,
				       void **ret_user_ptr)
    throw (InternalException)
{
    if (ret_user_obj != NULL)
	*ret_user_obj = NULL;

    if (ret_user_ptr != NULL)
	*ret_user_ptr = NULL;

    pp_stream->setPosition (cur_pmark);

    Ref<PpItem> pp_item;
    // Whitespace is stripped here.
    do {
	PpItemStream::PpItemResult pres = pp_stream->getNextItem (&pp_item);
	if (pres == PpItemStream::PpItemEof) {
	    pp_stream->setPosition (cur_pmark);
	    return ConstMemoryDesc();
	}

	if (pp_item->type == PpItemWhitespace) {
	    Whitespace * const whsp = static_cast <Whitespace*> (pp_item.ptr());
	    if (whsp->has_newline &&
		!newline_replacement.isNull())
	    {
		pp_item = grab (new PpToken (PpTokenPpOpOrPunc, // TODO Introduce PpTokenWhitespace
					     newline_replacement,
					     NULL /* macro_ban */,
					     // TODO Real file position
					     MyLang::FilePosition()));
		break;
	    }
	}
    } while (pp_item->type != PpItemPpToken);

#if 0
// NOTE: We do not convert pp items to tokens anymore!
    Ref<Token> token = ppTokenToToken (static_cast <PpToken*> (pp_item.ptr ()));

    cur_pmark = pp_stream->getPosition ();

    if (ret_user_obj != NULL)
	*ret_user_obj = token;

    if (ret_user_ptr != NULL)
	*ret_user_ptr = static_cast <void*> (token.ptr ());

    return token->str->getMemoryDesc();
#endif

    cur_pmark = pp_stream->getPosition ();

    if (ret_user_obj) {
//	errf->print ("--- PpItemStreamTokenStream::GetNextToken: saving pp_item 0x").printHex ((UintPtr) pp_item.ptr()).pendl ();
	*ret_user_obj = pp_item;
    }

    if (ret_user_ptr)
	*ret_user_ptr = (void*) (pp_item.ptr());

    return pp_item->str->getMemoryDesc();
}

void
PpItemStreamTokenStream::getPosition (TokenStream::PositionMarker * const ret_pmark /* non-null */)
    throw (InternalException)
{
    ret_pmark->body.copy_func = pmark_copy_func;
    ret_pmark->body.release_func = pmark_release_func;
    ret_pmark->body.ptr = cur_pmark;
    cur_pmark->ref ();

#if 0
// Old variant
    Ref<TokenStream::PositionMarker> ret_pmark = grab (static_cast <TokenStream::PositionMarker*> (new PositionMarker));

    PositionMarker * const pmark = static_cast <PositionMarker*> (ret_pmark.ptr ());
    pmark->pp_pmark = cur_pmark;

    return ret_pmark;
#endif
}

void
PpItemStreamTokenStream::setPosition (TokenStream::PositionMarker const * const pmark /* non-null */)
    throw (InternalException)
{
    cur_pmark = static_cast <PpItemStream::PositionMarker*> (pmark->body.ptr);
}

PpItemStreamTokenStream::PpItemStreamTokenStream (PpItemStream * const pp_stream)
{
    if (pp_stream == NULL)
	abortIfReached ();

    this->pp_stream = pp_stream;
    this->cur_pmark = pp_stream->getPosition ();
}

}

