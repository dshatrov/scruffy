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


#ifndef __SCRUFFY__PREPROCESSOR_H__
#define __SCRUFFY__PREPROCESSOR_H__

#include <mycpp/mycpp.h>
#include <mycpp/io.h>

#include <mylang/unichar_stream.h>

#include <scruffy/preprocessor_types.h>
#include <scruffy/pp_item_stream.h>
#include <scruffy/parsing_exception.h>

namespace Scruffy {

using namespace MyCpp;

// mt_throws ((InternalException, ParsingException))
unsigned long matchPreprocessingToken (MyLang::UnicharStream *unichar_stream,
				       PpTokenType *ret_pp_token_type);

class CppPreprocessor : public virtual Object
{
public:
    // mt_throws ((InternalException, ParsingException))
    typedef unsigned long (*PpTokenMatchFunc) (MyLang::UnicharStream *unichar_stream,
					       PpTokenType *ret_pp_token_type);

public:
//protected:
    class IfStackEntry
    {
    public:
	enum State {
	    If,
	    Elif,
	    Else
	};

	State state;

	enum MatchType {
	    NoMatch,
	    Match,
	    PrvMatch,
	    Skipping
	};

	MatchType match_type;

	IfStackEntry ()
	{
	}

	IfStackEntry (State state,
		      MatchType match_type)
	{
	    this->state = state;
	    this->match_type = match_type;
	}
    };

    PpTokenMatchFunc pp_token_match_func;

    List<IfStackEntry> if_stack;
    bool if_skipping;

    Map< Ref<MacroDefinition>,
	 MemberExtractor< MacroDefinition,
			  Ref<String>,
			  &MacroDefinition::name,
			  MemoryDesc,
			  AccessorExtractor< String,
					     MemoryDesc,
					     &String::getMemoryDesc > >,
	 MemoryComparator<> >
	    macro_definitions;

    Ref< List_< Ref<PpItem>, SimplyReferenced > > pp_items;

    Ref<File> source_file;

    Ref< List_< Ref<String>, SimplyReferenced > >
	    extractMacroParameters (PpItemStream *pp_stream)
			     throw (InternalException,
				    ParsingException);

    Ref< List_< Ref<PpItem>, SimplyReferenced > >
	    extractReplacementList (PpItemStream *pp_stream)
			     throw (InternalException,
				    ParsingException);

    void do_translateIfDirective (PpItemStream *pp_stream,
				  bool elif)
			   throw (ParsingException,
				  InternalException);

    void translateIfDirective (PpItemStream *pp_stream)
			throw (ParsingException,
			       InternalException);

    void translateElifDirective (PpItemStream *pp_stream)
			  throw (ParsingException,
				 InternalException);

    void do_translateIfdefDirective (PpItemStream *pp_stream,
				     bool inverse)
			   throw (InternalException,
				  ParsingException);

    void translateIfdefDirective (PpItemStream *pp_stream)
			   throw (InternalException,
				  ParsingException);

    void translateIfndefDirective (PpItemStream *pp_stream)
			    throw (InternalException,
				   ParsingException);

    void translateElseDirective (PpItemStream *pp_stream)
			  throw (InternalException,
				 ParsingException);

    void translateEndifDirective (PpItemStream *pp_stream)
			   throw (InternalException,
				  ParsingException);

    void translateIncludeDirective (PpItemStream *pp_stream)
			     throw (ParsingException,
				    InternalException);

    void translateDefineDirective (PpItemStream *pp_stream)
			    throw (InternalException,
				   ParsingException);

    void translateUndefDirective (PpItemStream *pp_stream)
			    throw (InternalException,
				   ParsingException);

    void translatePreprocessingDirective (PpItemStream *pp_stream)
				   throw (InternalException,
					  ParsingException);

    Ref< List_< Ref<PpItem>, SimplyReferenced > >
	    translateMacroInvocation (PpItemStream   *pp_token_stream,
				      MacroDefinition *mdef,
				      MacroBan        *macro_ban)
			       throw (InternalException,
				      ParsingException);

    Ref< List_< Ref<PpItem>, SimplyReferenced > >
	    translateObjectMacro (MacroDefinition *mdef,
				  MacroBan        *macro_ban)
			   throw (InternalException,
				  ParsingException);

    void evaluateDoubleSharps (List< Ref<PpItem> > *pp_items,
			       MacroBan *macro_ban)
			throw (InternalException,
			       ParsingException);

    void translatePpToken (PpItemStream *pp_token_stream,
			   PpToken       *pp_token,
			   List< Ref<PpItem> > *receiving_list)
		    throw (InternalException,
			   ParsingException);

    void predefineMacros ();

    void translationPhase3 (File *in_file)
		     throw (InternalException,
			    ParsingException);

public:
    void performPreprocessing ()
			throw (InternalException,
			       ParsingException);

    Ref< List_< Ref<PpItem>, SimplyReferenced > > getPpItems ()
    {
	return pp_items;
    }

    CppPreprocessor (File *source_file,
		     PpTokenMatchFunc pp_token_match_func = matchPreprocessingToken);
};

}

#endif /* __SCRUFFY__PREPROCESSOR_H__ */

