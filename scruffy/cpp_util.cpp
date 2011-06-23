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


#include <mycpp/io.h>

#include <scruffy/cpp_util.h>

using namespace MyCpp;

namespace Scruffy {

namespace Cpp {

static void
do_dumpContainerInfo (Container const &container,
		      bool      const  dump_parent)
{
    switch (container.getType ()) {
	case Container::_Namespace: {
	    Namespace const &namespace_ = static_cast <Namespace const &> (container);
	    errf->print ("namespace ").print (namespace_.getPrimaryName ());
	} break;
	case Container::_Class: {
	    Class const &class_ = static_cast <Class const &> (container);
	    errf->print ("class");
	    if (!class_.name.isNull ())
		errf->print (" ").print (class_.name->toString ());
	} break;
	default:
	    abortIfReached ();
    }

    if (dump_parent) {
	errf->print (" (parent ");
	if (container.getParentContainer () != NULL) {
	    do_dumpContainerInfo (*container.getParentContainer (), false /* dump_parent */);
	}
	errf->print (")");
    }
}

void
dumpContainerInfo (Container const &container)
{
    do_dumpContainerInfo (container, true /* dump_parent */);
    errf->print ("\n");
}

void
dumpMemberInfo (Member const * const member /* non-null */)
{
    switch (member->getType ()) {
	case Member::_Type: {
	    errf->print ("_Type");
	} break;
	case Member::_Object: {
	    errf->print ("_Object");
	} break;
	case Member::_Function: {
	    errf->print ("_Function");
	} break;
	case Member::_TypeTemplateParameter: {
	    errf->print ("_TypeTemplateParameter");
	} break;
	case Member::_DependentType: {
	    errf->print ("_DependentType");
	} break;
	case Member::_DependentObject: {
	    errf->print ("_DependentObject");
	} break;
	default:
	    abortIfReached ();
    }

    if (!member->name.isNull ())
	errf->print (" ").print (member->name->toString ());
}

}

}

