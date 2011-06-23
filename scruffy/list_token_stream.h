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


#include <mycpp/mycpp.h>

#include <mylang/token_stream.h>

namespace Scruffy {

class ListTokenStream : public MyLang::TokenStream,
			public virtual SimplyReferenced
{
private:
    List< Ref<Token> > * const token_list;
    List< Ref<Token> >::Element *cur_el;

public:
    ConstMemoryDesc getNextToken ()
    {
	return getNextToken (NULL, NULL);
    }

    ConstMemoryDesc getNextToken (Ref<SimplyReferenced> * const ret_user_obj,
				  void ** const ret_user_ptr)
			   throw (InternalException)
    {
	if (cur_el == NULL) {
	    if (ret_user_obj != NULL)
		*ret_user_obj = NULL;

	    if (ret_user_ptr != NULL)
		*ret_user_ptr = NULL;

	    return ConstMemoryDesc ();
	}

	Token * const token = cur_el->data;

	if (ret_user_obj != NULL)
	    *ret_user_obj = token;

	if (ret_user_ptr != NULL)
	    *ret_user_ptr = token;

	ConstMemoryDesc const ret_mem = token->str->getMemoryDesc ();

	cur_el = cur_el->next;

	return ret_mem;
    }

    void getPosition (PositionMarker * const ret_pmark /* non-null */)
	       throw (InternalException)
    {
	ret_pmark->body.ptr = cur_el;
    }

    void setPosition (PositionMarker const * const pmark /* non-null */)
	       throw (InternalException)
    {
	cur_el = static_cast < List< Ref<Token> >::Element* > (pmark->body.ptr);
    }

    MyLang::FilePosition getFilePosition ()
				   throw (InternalException)
    {
	return MyLang::FilePosition ();
    }

    ListTokenStream (List< Ref<Token> > * const token_list /* non-null */)
	: token_list (token_list),
	  cur_el (token_list->first)
    {
    }
};

}

