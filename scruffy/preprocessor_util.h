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


#ifndef __SCRUFFY_PREPROCESSOR_UTIL_H__
#define __SCRUFFY_PREPROCESSOR_UTIL_H__

#include <list>

#include <mycpp/string.h>
#include <mycpp/list.h>
#include <mycpp/internal_exception.h>

#include <mylang/unichar_stream.h>

#include <scruffy/preprocessor_types.h>
#include <scruffy/parsing_exception.h>

namespace Scruffy {

using namespace MyCpp;

Ref<String> extractString (MyLang::UnicharStream *unichar_stream,
			   unsigned long len)
		    throw (InternalException,
			   ParsingException);

unsigned long matchDigit (MyLang::UnicharStream *unichar_stream)
		   throw (InternalException,
			  ParsingException);

unsigned long matchOctalDigit (MyLang::UnicharStream *unichar_stream)
			throw (InternalException,
			       ParsingException);

unsigned long matchHexadecimalDigit (MyLang::UnicharStream *unichar_stream)
			      throw (InternalException,
				     ParsingException);

unsigned long matchNondigit (MyLang::UnicharStream *unichar_stream)
		      throw (InternalException,
			     ParsingException);

unsigned long matchWhitespace (MyLang::UnicharStream *unichar_stream,
			       bool *contains_newline)
			throw (InternalException,
			       ParsingException);

unsigned long matchHeaderName (MyLang::UnicharStream *unichar_stream)
			throw (InternalException,
			       ParsingException);

unsigned long matchIdentifier (MyLang::UnicharStream *unichar_stream)
			throw (InternalException,
			       ParsingException);

unsigned long matchPpNumber (MyLang::UnicharStream *unichar_stream)
		      throw (InternalException,
			     ParsingException);

unsigned long matchSimpleEscapeSequence (MyLang::UnicharStream *unichar_stream)
				  throw (InternalException,
					 ParsingException);

unsigned long matchOctalEscapeSequence (MyLang::UnicharStream *unichar_stream)
				 throw (InternalException,
					ParsingException);

unsigned long matchHexadecimalEscapeSequence (MyLang::UnicharStream *unichar_stream)
				       throw (InternalException,
					      ParsingException);

unsigned long matchEscapeSequence (MyLang::UnicharStream *unichar_stream)
			    throw (InternalException,
				   ParsingException);

unsigned long matchUniversalCharacterName (MyLang::UnicharStream *unichar_stream)
				    throw (InternalException,
					   ParsingException);

unsigned long matchCChar (MyLang::UnicharStream *unichar_stream)
		   throw (InternalException,
			  ParsingException);

unsigned long matchCCharSequence (MyLang::UnicharStream *unichar_stream)
			   throw (InternalException,
				  ParsingException);

unsigned long matchCharacterLiteral (MyLang::UnicharStream *unichar_stream)
			      throw (InternalException,
				     ParsingException);

unsigned long matchSChar (MyLang::UnicharStream *unichar_stream)
		   throw (InternalException,
			  ParsingException);

unsigned long matchSCharSequence (MyLang::UnicharStream *unichar_stream)
			   throw (InternalException,
				  ParsingException);

unsigned long matchStringLiteral (MyLang::UnicharStream *unichar_stream)
			   throw (InternalException,
				  ParsingException);

unsigned long matchPreprocessingOpOrPunc (MyLang::UnicharStream *unichar_stream)
				   throw (InternalException,
					  ParsingException);

Size matchHeaderName (MyLang::UnicharStream *unichar_stream,
		      PpToken_HeaderName::HeaderNameType *ret_hn_type,
		      Ref<String> *ret_header_name)
	       throw (InternalException,
		      ParsingException);

// mt_throws ((InternalException, ParsingException))
unsigned long matchPreprocessingToken (MyLang::UnicharStream *unichar_stream,
				       PpTokenType *pp_token_type);

bool compareReplacementLists (List< Ref<PpItem> > *left,
			      List< Ref<PpItem> > *right);

Ref<String> ppItemsToString (List< Ref<PpItem> > *pp_items)
		      throw (ParsingException,
			     InternalException);

Ref<String> spellPpItems (List< Ref<PpItem> > *pp_items)
				 throw (InternalException,
					ParsingException);

Ref<Token> ppTokenToToken (PpToken *pp_token)
			   throw (ParsingException);

void ppTokensToTokens (List< Ref<PpToken> > *pp_tokens  /* non-null */,
		       List< Ref<Token> >   *ret_tokens /* non-null */);

void ppItemsToTokens (List< Ref<PpItem> > *pp_items   /* non-null */,
		      List< Ref<Token> >  *ret_tokens /* non-null */);

}

#endif /* __SCRUFFY_PREPROCESSOR_UTIL_H__ */

