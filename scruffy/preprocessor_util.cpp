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


#include <cstring>

#include <string>
//#include <boost/unordered/unordered_set.hpp>

#include <libmary/hash.h>

#include <mycpp/mycpp.h>
#include <mycpp/io.h>
#include <mycpp/direct_array_file.h>

#include <mylang/util.h>
#include <mylang/file_byte_stream.h>
#include <mylang/utf8_unichar_stream.h>

#include <scruffy/preprocessor_util.h>
#include <scruffy/util.h>

#define DEBUG(a) ;
// Internal
#define DEBUG_INT(a) ;

using namespace std;

using namespace MyCpp;
using namespace MyLang;

namespace Scruffy {

namespace Scruffy_PreprocessorUtil_priv {}
using namespace Scruffy_PreprocessorUtil_priv;

Ref<String>
extractString (UnicharStream *unichar_stream,
	       unsigned long  len)
    throw (InternalException,
	   ParsingException)
{
    Ref<String> ret_str;

    /* TODO Come up with something like a smart pointer. */
    Unichar *uchars = new Unichar [len]; // no grab() needed
    unsigned long i;
    for (i = 0; i < len; i++) {
	UnicharStream::UnicharResult ures;
	ures = unichar_stream->getNextUnichar (&uchars [i]);
	if (ures != UnicharStream::UnicharNormal) {
	    delete[] uchars;
	    throw InternalException ();
	}
    }

    ret_str = ucs4_to_utf8 (uchars, len);
    if (ret_str.isNull ()) {
	delete[] uchars;
	throw InternalException ();
    }

    delete[] uchars;
    return ret_str;
}

#if 0
/* hex-quad:
 * hexadecimal-digit hexadecimal-digit hecadecimal-digit hexadecimal-digit
 */
bool isHexQuad (const char *str)
{
    return false;
}

#endif

const unsigned long num_digits = 10;
const char* digits [num_digits] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

const unsigned long num_octal_digits = 8;
const char* octal_digits [num_octal_digits] = {
    "0", "1", "2", "3", "4", "5", "6", "7"
};

const unsigned long num_hex_digits = 22;
const char *hex_digits [num_hex_digits] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "a", "b", "c", "d", "e", "f",
    "A", "B", "C", "D", "E", "F"
};

const unsigned long num_nondigits = 53;
const char* nondigits [num_nondigits] = {
    "_", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
	 "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	 "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
	 "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
};

const unsigned long num_whitespace = 4;
const char* whitespace [num_whitespace] = {
    /* TODO Take a look at the standard. */
    " ", "\t", "\n", "\r"
};

Size
matchSingleChar (UnicharStream *unichar_stream,
		 Unichar target_uc)
    throw (ParsingException,
	   InternalException)
{
    abortIf (unichar_stream == NULL);

    Unichar uc;
    UnicharStream::UnicharResult res;
    res = unichar_stream->getNextUnichar (&uc);
    if (res != UnicharStream::UnicharNormal)
	return 0;

    if (uc == target_uc)
	return 1;

    return 0;
}

/* sign: one of
 *     + -
 */
bool
isSign (Unichar uc)
{
    if (uc == 0x2b /* '+' */ ||
	uc == 0x2d /* '-' */)
    {
	return true;
    }

    return false;
}

Size
matchSign (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    abortIf (unichar_stream == NULL);

    Unichar uc;
    UnicharStream::UnicharResult res;
    res = unichar_stream->getNextUnichar (&uc);
    if (res != UnicharStream::UnicharNormal)
	return 0;

    if (uc == 0x2b /* '+' */ ||
	uc == 0x2d /* '-' */)
    {
	return 1;
    }

    return 0;
}

/* digit: one of
 *     0 1 2 3 4 5 6 7 8 9
 */
bool
isDigit (Unichar uc)
{
    if (uc >= 0x30 /* '0' */ && uc <= 0x39 /* '9' */)
	return true;

    return false;
}

/* nonzero-digit: one of
 *     1 2 3 4 5 6 7 8 9
 */
bool
isNonzeroDigit (Unichar uc)
{
    if (uc >= 0x31 /* '1' */ && uc <= 0x39 /* '9' */)
	return true;

    return false;
}

/* nondigit: one of
 *     universal-character-name
 *     _ a b c d e f g h i j k l m
 *       n o p q r s t u v w x y z
 *       A B C D E F G H I J K L M
 *       N O P Q R S T U V W X Y Z
 */
bool
isNondigit (Unichar uc)
{
    if ((uc >= 0x41 /* 'A' */ && uc <= 0x5a /* 'Z' */) ||
	(uc >= 0x61 /* 'a' */ && uc <= 0x7a /* 'z' */) ||
	(uc == 0x5f /* '_' */))
    {
	return true;
    }

    return false;
}

/* octal-digit: one of
 *     0 1 2 3 4 5 6 7
 */
bool
isOctalDigit (Unichar uc)
{
    if (uc >= 0x30 /* 0 */ && uc <= 0x37 /* '7' */)
	return true;

    return false;
}

/* hexadecimal-digit: one of
 *     0 1 2 3 4 5 6 7 8 9
 *     a b c d e f
 *     A B C D E F
 */
bool
isHexadecimalDigit (Unichar uc)
{
    if ((uc >= 0x30 /* '0' */ && uc <= 0x39 /* '9' */) ||
	(uc >= 0x41 /* 'A' */ && uc <= 0x46 /* 'F' */) ||
	(uc >= 0x61 /* 'a' */ && uc <= 0x66 /* 'F' */))
    {
	return true;
    }

    return false;
}

/* digit: one of
 *     0 1 2 3 4 5 6 7 8 9
 */
unsigned long
matchDigit (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    unsigned long i;
    for (i = 0; i < num_digits; i++) {
	if (!utf8_validate_sz (digits [i], NULL))
	    throw InternalException ();

	Unichar digit = utf8_valid_to_unichar (digits [i]);
	if (uc == digit)
	    return 1;
    }

    return 0;
}

/* octal-digit: one of
 *     0 1 2 3 4 5 6 7
 */
unsigned long
matchOctalDigit (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    unsigned long i;
    for (i = 0; i < num_octal_digits; i++) {
	if (!utf8_validate_sz (octal_digits [i], NULL))
	    throw InternalException ();

	Unichar octal_digit = utf8_valid_to_unichar (octal_digits [i]);
	if (uc == octal_digit)
	    return 1;
    }

    return 0;
}

/* hexadecimal-digit: one of
 *     0 1 2 3 4 5 6 7 8 9
 *     a b c d e f
 *     A B C D E F
 */
unsigned long
matchHexadecimalDigit (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    unsigned long i;
    for (i = 0; i < num_hex_digits; i++) {
	if (!utf8_validate_sz (hex_digits [i], NULL))
	    throw InternalException ();

	Unichar hex_digit = utf8_valid_to_unichar (hex_digits [i]);
	if (uc == hex_digit)
	    return 1;
    }

    return 0;
}

/* nondigit: one of
 *     universal-character-name
 *     _ a b c d e f g h i j k l m
 *       n o p q r s t u v w x y z
 *       A B C D E F G H I J K L M
 *       N O P Q R S T U V W X Y Z
 */
unsigned long
matchNondigit (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    unsigned long i;
    for (i = 0; i < num_nondigits; i++) {
	if (!utf8_validate_sz (nondigits [i], NULL))
	    throw InternalException ();

	Unichar nondigit = utf8_valid_to_unichar (nondigits [i]);
	if (uc == nondigit)
	    return 1;
    }

    return 0;
}

#if 0
bool
isSign (const char *cbuf)
{
    if (cbuf [0] == '+' ||
	cbuf [0] == '-')
    {
	return true;
    }

    return false;
}

/* q-char:
 *     any member of the source character set except
 *         new-line and "
 */
bool
isQChar (const char *str)
{
    return false;
}

/* q-char-sequence:
 *     q-char
 *     q-char-sequence q-char
 */
bool
isQCharSequence (const char *str)
{
    return false;
}

/* h-char:
 *     any member of the source character set except
 *         new-line and >
 */
bool
isHChar (const char *str)
{
    return false;
}

/* h-char-sequence:
 *     h-char
 *     h-char-sequence h-char
 */
bool
isHCharSequence (const char *str)
{
    return false;
}
#endif

unsigned long
matchWhitespace (UnicharStream *unichar_stream,
		 bool          *contains_newline)
    throw (InternalException,
	   ParsingException)
{
    DEBUG (
	errf->print ("Scruffy.Preprocessor.matchWhitespace").pendl ();
    )

    if (unichar_stream == NULL)
	abortIfReached ();

    unsigned long total_len = 0;

    if (contains_newline != NULL)
	*contains_newline = false;

    const char *backslash = "\\",
	       *fwslash = "/",
	       *star = "*";

    if (!utf8_validate_sz (backslash, NULL) ||
	!utf8_validate_sz (fwslash, NULL)   ||
	!utf8_validate_sz (star, NULL))
    {
	throw InternalException ();
    }

    // TODO Just use the codepoint values.
    Unichar backslash_uc = utf8_valid_to_unichar (backslash),
	    fwslash_uc   = utf8_valid_to_unichar (fwslash),
	    star_uc      = utf8_valid_to_unichar (star);

    Unichar uc;
    UnicharStream::UnicharResult ures;

    for (;;) {
	ures = unichar_stream->getNextUnichar (&uc);
	if (ures != UnicharStream::UnicharNormal)
	    return total_len;

	DEBUG (
	    errf->print ("Scruffy.Preprocessor.matchWhitespace: "
			 "unichar: 0x").printHex ((Uint32) uc).pendl ();
	)

	bool is_whitespace = false;
	unsigned long i;
	for (i = 0; i < num_whitespace; i++) {
	    if (!utf8_validate_sz (whitespace [i], NULL))
		throw InternalException ();

	    // TODO Just use the codepoint values.
	    Unichar whsp_uc = utf8_valid_to_unichar (whitespace [i]);
	    if (uc == whsp_uc) {
		DEBUG (
		    errf->print ("Scruffy.Preprocessor.matchWhitespace: "
				 "whitespace").pendl ();
		)

		total_len ++;
		is_whitespace = true;
		break;
	    }
	}
	if (is_whitespace) {
	  // Odd codepath introduced to be able to use 'continue' operator
	  // for the outer loop.

	    if (isNewline (uc)) {
		DEBUG (
		    errf->print ("Scruffy.Preprocessor.matchWhitespace: "
				 "newline").pendl ();
		)

		if (contains_newline != NULL)
		    *contains_newline = true;
	    }

	    continue;
	}

	if (uc == backslash_uc) {
	    DEBUG (
		errf->print ("Scruffy.Preprocessor.matchWhitespace: "
			     "backslash").pendl ();
	    )

	    ures = unichar_stream->getNextUnichar (&uc);
	    if (ures != UnicharStream::UnicharNormal)
		return total_len;

	    DEBUG (
		errf->print ("Scruffy.Preprocessor.matchWhitespace: "
			     "next unichar: 0x").printHex ((Uint32) uc).pendl ();
	    )

	    if (isNewline (uc)) {
		DEBUG (
		    errf->print ("Scruffy.Preprocessor.matchWhitespace: "
				 "backslash + newline").pendl ();
		)

		total_len += 2;
	    } else
		return total_len;
	} else
	if (uc == fwslash_uc) {
	    DEBUG (
		errf->print ("Scruffy.Preprocessor.matchWhitespace: "
			     "fwslash").pendl ();
	    )

	    ures = unichar_stream->getNextUnichar (&uc);
	    if (ures != UnicharStream::UnicharNormal)
		return total_len;

	    DEBUG (
		errf->print ("Scruffy.Preprocessor.matchWhitespace: "
			     "next unichar: 0x").printHex ((Uint32) uc).pendl ();
	    )

	    if (uc == star_uc) {
		DEBUG (
		    errf->print ("Scruffy.Preprocessor.matchWhitespace: "
				 "fwslash + star").pendl ();
		)

		total_len += 2;

		for (;;) {
		  // Parsing a multiline comment.

		    ures = unichar_stream->getNextUnichar (&uc);
		    if (ures != UnicharStream::UnicharNormal) {
			throw ParsingException (unichar_stream->getFpos (),
						String::forData ("unterminated multiline comment"));
		    }
		    total_len ++;

/* Multiline comments are removed early, hence newlines inside them are irrelevant
		    if (isNewline (uc)) {
			if (contains_newline != NULL)
			    *contains_newline = true;
		    }
*/

		    if (uc == star_uc) {
			ures = unichar_stream->getNextUnichar (&uc);
			if (ures != UnicharStream::UnicharNormal) {
			    throw ParsingException (unichar_stream->getFpos (),
						    String::forData ("unterminated multiline comment"));
			}
			total_len ++;

/* Multiline comments are removed early, hence newlines inside them are irrelevant
			if (isNewline (uc)) {
			    if (contains_newline != NULL)
				*contains_newline = true;
			}
*/

			if (uc == fwslash_uc)
			    break;
		    }
		}
	    } else
	    if (uc == fwslash_uc) {
		DEBUG (
		    errf->print ("Scruffy.Preprocessor.matchWhitespace: "
				 "fwslash + fwslash").pendl ();
		)

		total_len += 2;

		for (;;) {
		  // Parsing a single-line comment.

		    ures = unichar_stream->getNextUnichar (&uc);
		    if (ures == UnicharStream::UnicharEof) {
			errf->print ("Scruffy.matchWhitespace: "
				     "warning: single-line comment does not "
				     "end with a newline")
			     .pendl ();

			return total_len;
		    }

		    if (ures != UnicharStream::UnicharNormal)
			abortIfReached ();

		    total_len ++;

		    if (isNewline (uc)) {
			if (contains_newline != NULL)
			    *contains_newline = true;

			break;
		    }
		}
	    } else
		return total_len;
	} else
	    return total_len;
    }

    /* Unreachable */
    abortIfReached ();
    return 0;
}

/* identifier:
 *     nondigit
 *     identifier nondigit
 *     identifier digit
 *
 * Non-recurrent: nondigit (nondigit | digit)*
 */
unsigned long
matchIdentifier (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    unsigned long total_len = 0;

    unsigned long nondigit_len,
		  digit_len;

    nondigit_len = matchNondigit (unichar_stream);
    if (nondigit_len == 0)
	return 0;

    total_len += nondigit_len;

    for (;;) {
	Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

	nondigit_len = matchNondigit (unichar_stream);

	unichar_stream->setPosition (pmark);
	digit_len = matchDigit (unichar_stream);

	const unsigned long nnumbers = 2;
	unsigned long number_set [nnumbers] = {
	    nondigit_len,
	    digit_len
	};

	if (numbersAreZero (number_set, nnumbers))
	    break;

	unsigned long max_len = maxNumberOf (number_set, nnumbers);

	total_len += max_len;

	unichar_stream->setPosition (pmark);
	unichar_stream->skip (max_len);
    }

    return total_len;
}

/* pp-number:
 *     digit
 *     . digit
 *     pp-number digit
 *     pp-number nondigit
 *     pp-number e sign
 *     pp-number E sign
 *     pp-number .
 */
Size
matchPpNumber (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    abortIf (unichar_stream == NULL);

    Size total_len = 0;

    Unichar uc;
    bool first_char = true;
    bool skip_get = false;
    for (;;) {
	if (!skip_get) {
	    UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
	    if (res != UnicharStream::UnicharNormal)
		return total_len;
	} else {
	    skip_get = false;
	}

	if (isDigit (uc)) {
	    first_char = false;
	    total_len ++;
	    continue;
	}

	if (uc == 0x2e /* '.' */) {
	    total_len ++;
	    continue;
	}

	if (!first_char) {
	    if (uc == 0x45 /* 'E' */ ||
		uc == 0x65 /* 'e' */)
	    {
		total_len ++;

		UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
		if (res != UnicharStream::UnicharNormal)
		    return total_len;

		if (isSign (uc)) {
		    total_len ++;
		    continue;
		}

		// We've got a nondigit ('E' or 'e').
		skip_get = true;
		continue;
	    }

	    if (isNondigit (uc)) {
		total_len ++;
		continue;
	    }
	}

	break;
    }

#if 0
  // DEBUG
    if (total_len > 0)
	errf->print ("NUMBER: len: ").print (total_len).pendl ();
  // (DEBUG)
#endif

    return total_len;
}

const unsigned long num_ses_chars = 11;
const char* ses_chars [num_ses_chars] = {
    "'", "\"", "?", "\\", "a", "b", "f", "n", "r", "t", "v"
};

/* simple-escape-sequence: one of
 *     \' \" \? \\
 *     \a \b \f \n \r \t \v
 */
unsigned long
matchSimpleEscapeSequence (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    const char *backslash = "\\";

    if (!utf8_validate_sz (backslash, NULL))
	throw InternalException ();

    const Unichar backslash_uc = utf8_valid_to_unichar (backslash);

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc != backslash_uc)
	return 0;

    unsigned long i;
    for (i = 0; i < num_ses_chars; i++) {
	if (!utf8_validate_sz (ses_chars [i], NULL))
	    throw InternalException ();

	Unichar ses_char = utf8_valid_to_unichar (ses_chars [i]);
	if (uc == ses_char)
	    return 2;
    }

    return 0;
}

/* octal-escape-sequence:
 *     \ octal-digit
 *     \ octal-digit octal-digit
 *     \ octal-digit octal-digit octal-digit
 */
unsigned long
matchOctalEscapeSequence (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    const char *backslash = "\\";

    if (!utf8_validate_sz (backslash, NULL))
	throw InternalException ();

    const Unichar backslash_uc = utf8_valid_to_unichar (backslash);

    unsigned long total_len = 0;

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc != backslash_uc)
	return 0;

    total_len ++;

    unsigned long i;
    for (i = 0; i < 3; i++) {
	Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

	unsigned long octal_digit_len = matchOctalDigit (unichar_stream);
	if (octal_digit_len == 0)
	    break;

	total_len += octal_digit_len;
	unichar_stream->setPosition (pmark);
	unichar_stream->skip (octal_digit_len);
    }

    if (i == 0)
	return 0;

    return total_len;
}

/* hexadecimal-escape-sequence:
 *     \x hexadecimal-digit
 *     hxadecimal-escape-sequence hexadecimal-digit
 */
unsigned long
matchHexadecimalEscapeSequence (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    const char *backslash = "\\",
	       *x_small = "x";

    if (!utf8_validate_sz (backslash, NULL) ||
	!utf8_validate_sz (x_small, NULL))
    {
	throw InternalException ();
    }

    const Unichar backslash_uc = utf8_valid_to_unichar (backslash),
		  x_small_uc = utf8_valid_to_unichar (x_small);

    unsigned long total_len = 0;

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc != backslash_uc)
	return 0;

    total_len ++;

    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc != x_small_uc)
	return 0;

    total_len ++;

    unsigned long i;
    for (i = 0; i < 3; i++) {
	Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

	unsigned long hex_digit_len = matchHexadecimalDigit (unichar_stream);
	if (hex_digit_len == 0)
	    break;

	total_len += hex_digit_len;
	unichar_stream->setPosition (pmark);
	unichar_stream->skip (hex_digit_len);
    }

    if (i == 0)
	return 0;

    return total_len;
}

/* escape-sequence:
 *     simple-escape-sequence
 *     octal-escape-sequence
 *     hexadecimal-escape-sequence
 */
unsigned long
matchEscapeSequence (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

    unsigned long simple_es_len = matchSimpleEscapeSequence (unichar_stream);

    unichar_stream->setPosition (pmark);
    unsigned long octal_es_len = matchOctalEscapeSequence (unichar_stream);

    unichar_stream->setPosition (pmark);
    unsigned long hex_es_len = matchHexadecimalEscapeSequence (unichar_stream);

    const unsigned long nnumbers = 3;
    const unsigned long number_set [nnumbers] = {
	simple_es_len,
	octal_es_len,
	hex_es_len
    };

    return maxNumberOf (number_set, nnumbers);
}

/* universal-character-name:
 * \u hex-quad
 * \U hex-quad hex-quad
 */
unsigned long
matchUniversalCharacterName (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    // TODO
    return 0;
}

/* c-char:
 *     any member of the source character set except
 *         the single-quote ', backslash \, or new-line character
 *     escape-sequence
 *     universal-character-name
 */
unsigned long
matchCChar (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    const char *single_quote = "'",
	       *backslash = "\\",
	       *newline = "\n";

    if (!utf8_validate_sz (single_quote, NULL) ||
	!utf8_validate_sz (backslash, NULL)    ||
	!utf8_validate_sz (newline, NULL))
    {
	throw InternalException ();
    }

    Unichar single_quote_uc = utf8_valid_to_unichar (single_quote),
	    backslash_uc    = utf8_valid_to_unichar (backslash),
	    newline_uc      = utf8_valid_to_unichar (newline);

    if (uc == single_quote_uc ||
	uc == backslash_uc    ||
	uc == newline_uc)
    {
	unichar_stream->setPosition (pmark);
	unsigned long escape_sequence_len = matchEscapeSequence (unichar_stream);

	unichar_stream->setPosition (pmark);
	unsigned long ucn_len = matchUniversalCharacterName (unichar_stream);

	const unsigned long nnumbers = 2;
	const unsigned long number_set [nnumbers] = {
	    escape_sequence_len,
	    ucn_len
	};

	return maxNumberOf (number_set, nnumbers);
    }

    return 1;
}

/* c-char-sequence:
 *     c-char
 *     c-char-sequence c-char
 */
unsigned long
matchCCharSequence (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    unsigned long total_len = 0;

    Ref<UnicharStream::PositionMarker> pmark;
    for (;;) {
	pmark = unichar_stream->getPosition ();

	unsigned long cchar_len = matchCChar (unichar_stream);
	if (cchar_len == 0)
	    break;

	total_len += cchar_len;

	unichar_stream->setPosition (pmark);
	unichar_stream->skip (cchar_len);
    }

    return total_len;
}

/* character-literal:
 *     'c-char-sequence'
 *     L'c-char-sequence'
 */
unsigned long
matchCharacterLiteral (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    const char *single_quote = "'",
	       *l_large = "L";

    if (!utf8_validate_sz (single_quote, NULL) ||
	!utf8_validate_sz (l_large, NULL))
    {
	throw InternalException ();
    }

    const Unichar single_quote_uc = utf8_valid_to_unichar (single_quote),
		  l_large_uc = utf8_valid_to_unichar (l_large);

    unsigned long total_len = 0;

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc == l_large_uc) {
	ures = unichar_stream->getNextUnichar (&uc);
	if (ures != UnicharStream::UnicharNormal)
	    return 0;

	if (uc != single_quote_uc)
	    return 0;

	total_len += 2;
    } else
    if (uc == single_quote_uc) {
	total_len ++;
    } else
	return 0;

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();
    unsigned long cchar_seq_len = matchCCharSequence (unichar_stream);
    if (cchar_seq_len == 0)
	return 0;

    unichar_stream->setPosition (pmark);
    unichar_stream->skip (cchar_seq_len);
    total_len += cchar_seq_len;

    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc != single_quote_uc)
	return 0;

    total_len ++;

    return total_len;
}

/* s-char:
 *     any member of the source character set except
 *         the double-quote ", backslash \, or new-line character
 *     escape-sequence
 *     universal-character-name
 */
unsigned long
matchSChar (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    const char *double_quote = "\"",
	       *backslash = "\\",
	       *newline = "\n";

    if (!utf8_validate_sz (double_quote, NULL) ||
	!utf8_validate_sz (backslash, NULL)    ||
	!utf8_validate_sz (newline, NULL))
    {
	throw InternalException ();
    }

    Unichar double_quote_uc = utf8_valid_to_unichar (double_quote),
	    backslash_uc    = utf8_valid_to_unichar (backslash),
	    newline_uc      = utf8_valid_to_unichar (newline);

    if (uc == double_quote_uc ||
	uc == backslash_uc    ||
	uc == newline_uc)
    {
	unichar_stream->setPosition (pmark);
	unsigned long escape_sequence_len = matchEscapeSequence (unichar_stream);

	unichar_stream->setPosition (pmark);
	unsigned long ucn_len = matchUniversalCharacterName (unichar_stream);

	const unsigned long nnumbers = 2;
	const unsigned long number_set [nnumbers] = {
	    escape_sequence_len,
	    ucn_len
	};

	return maxNumberOf (number_set, nnumbers);
    }

    return 1;
}

/* s-char-sequence:
 *     s-char
 *     s-char-sequence s-char
 */
unsigned long
matchSCharSequence (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    unsigned long total_len = 0;

    Ref<UnicharStream::PositionMarker> pmark;
    for (;;) {
	pmark = unichar_stream->getPosition ();

	unsigned long schar_len = matchSChar (unichar_stream);
	if (schar_len == 0)
	    break;

	total_len += schar_len;

	unichar_stream->setPosition (pmark);
	unichar_stream->skip (schar_len);
    }

    return total_len;
}

/* string-literal:
 *     "s-char-sequence_{opt}"
 *     L"s-char-sequence_{opt}"
 */
unsigned long
matchStringLiteral (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    const char *double_quote = "\"",
	       *l_large = "L";

    if (!utf8_validate_sz (double_quote, NULL) ||
	!utf8_validate_sz (l_large, NULL))
    {
	throw InternalException ();
    }

    const Unichar double_quote_uc = utf8_valid_to_unichar (double_quote),
		  l_large_uc = utf8_valid_to_unichar (l_large);

    unsigned long total_len = 0;

    Unichar uc;
    UnicharStream::UnicharResult ures;
    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc == l_large_uc) {
	ures = unichar_stream->getNextUnichar (&uc);
	if (ures != UnicharStream::UnicharNormal)
	    return 0;

	if (uc != double_quote_uc)
	    return 0;

	total_len += 2;
    } else
    if (uc == double_quote_uc) {
	total_len ++;
    } else
	return 0;

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();
    unsigned long schar_seq_len = matchSCharSequence (unichar_stream);
    unichar_stream->setPosition (pmark);
    unichar_stream->skip (schar_seq_len);
    total_len += schar_seq_len;

    ures = unichar_stream->getNextUnichar (&uc);
    if (ures != UnicharStream::UnicharNormal)
	return 0;

    if (uc != double_quote_uc)
	return 0;

    total_len ++;

    return total_len;
}

const unsigned long num_keywords = 63;
static const char *keywords [num_keywords] = {
    "asm",        "do",           "if",               "return",      "typedef",
    "auto",       "double",       "inline",           "short",       "typeid",
    "bool",       "dynamic_cast", "int",              "signed",      "typename",
    "break",      "else",         "long",             "sizeof",      "union",
    "case",       "enum",         "mutable",          "static",      "unsigned",
    "catch",      "explicit",     "namespace",        "static_cast", "using",
    "char",       "export",       "new",              "struct",      "virtual",
    "class",      "extern",       "operator",         "switch",      "void",
    "const",      "false",        "private",          "template",    "volatile",
    "const_cast", "float",        "protected",        "this",        "wchar_t",
    "continue",   "for",          "public",           "throw",       "while",
    "default",    "friend",       "register",         "true",
    "delete",     "goto",         "reinterpret_cast", "try"
};

namespace Scruffy_PreprocessorUtil_priv {

#if 0
    typedef Map< Ref<String>,
		 AccessorExtractor< String,
				    MemoryDesc,
				    &String::getMemoryDesc >,
		 MemoryComparator<>,
		 SimplyReferenced >
	    KeywordMap;

    Ref<KeywordMap> keyword_map;

    static void
    fill_keyword_map ()
    {
      helperLock ();

	if (keyword_map.isNull ()) {
	    keyword_map = grab (new KeywordMap);
	    for (Size i = 0; i < num_keywords; i++)
		keyword_map->add (String::forData (keywords [i]));
	}

      helperUnlock ();
    }
#endif

//    boost::unordered_set<string> keywords_set;

    class KeywordEntry : public M::HashEntry<>
    {
    public:
	Ref<String> keyword;
    };

    typedef M::Hash< KeywordEntry,
		     MemoryDesc,
		     MemberExtractor< KeywordEntry,
				      Ref<String>,
				      &KeywordEntry::keyword,
				      MemoryDesc,
				      AccessorExtractor< String,
							 MemoryDesc,
							 &String::getMemoryDesc > >,
		     MemoryComparator<> >
	    KeywordHash;

    KeywordHash keyword_hash;

    static void
    fill_keywords ()
    {
      // TODO Make this part of initialization process.
      // helperLock() / helperUnlock() won't be necessary then.

	if (keyword_hash.isEmpty ()) {
	    for (Size i = 0; i < num_keywords; i++) {
//		keywords_set.insert (keywords [i]);
		// TODO Global cleanup (keyword_hash entries are never deleted
		//      currently).
		KeywordEntry * const keyword_entry = new KeywordEntry;
		keyword_entry->keyword = grab (new String (keywords [i]));
		keyword_hash.add (keyword_entry);
	    }
	}
    }

    static bool is_keyword (ConstMemoryDesc const &mem)
    {
//	errf->print ("--- is_keyword: ").print (mem).pendl ();

//	fill_keyword_map ();
	fill_keywords ();
//	abortIf (keyword_map.isNull ());

#if 0
	// TODO Make this more effective
	// FIXME Downright ugly length limit!
	char sz_str [512];
	memcpy (sz_str, mem.getMemory (), mem.getLength ());
	sz_str [mem.getLength ()] = 0;
#endif

//	if (!keyword_map->lookup (mem).isNull ()) {
//	if (keywords_set.count (sz_str) > 0) {
	if (keyword_hash.lookup (mem)) {
//	    errf->print ("--- is_keyword: true").pendl ();
	    return true;
	}

	return false;
    }

}

/*
unsigned long
matchKeyword (Token *token)
{
    if (token == NULL)
	return NULL;

    unsigned long i;
    for (i = 0; i < num_keywords; i++) {
	if (compareStrings (token->str->getData (), keywords [i]))
	    break;
    }

    if (i >= num_keywords)
	return 0;

    return 1;
}
*/

const unsigned long num_ops_puncs = 70;
static const char *ops_puncs [num_ops_puncs] = {
    "{",   "}",      "[",      "]",     "#",     "##",   "(",      ")",
    "<:",  ":>",     "<%",     "%>",    "%:",    "%:%:", ";",      ":",   "...",
    "new", "delete", "?",      "::",    ".",     ".*",
    "+",   "-",      "*",      "/",     "%",     "^",    "&",      "|",    "~",
    "!",   "=",      "<",      ">",     "+=",    "-=",   "*=",     "/=",  "%=",
    "^=",  "&=",     "|=",     "<<",    ">>",    ">>=",  "<<=",    "==",  "!=",
    "<=",  ">=",     "&&",     "||",    "++",    "--",   ",",      "->*", "->",
    "and", "and_eq", "bitand", "bitor", "compl", "not",  "not_eq",
    "or",  "or_eq",  "xor",    "xor_eq"
};

const unsigned long num_operators = 42;
static const char *operators [num_operators] = {
    "new", "delete", "new[]", "delete[]",
    "+",   "-",  "*",  "/",  "%",  "^",   "&",   "|",   "~",
    "!",   "=",  "<",  ">",  "+=", "-=",  "*=",  "/=",  "%=",
    "^=",  "&=", "|=", "<<", ">>", ">>=", "<<=", "==",  "!=",
    "<=",  ">=", "&&", "||", "++", "--",  ",",   "->*", "->",
    "()",  "[]"
};

/* (see 2.12) */
unsigned long
matchPreprocessingOpOrPunc (UnicharStream *unichar_stream)
    throw (InternalException,
	   ParsingException)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    const char *best_fit = NULL;
    unsigned long best_fit_len = 0;

    const char **matched = ops_puncs;
    const char *matched_arr [num_ops_puncs];

    unsigned long nmatched = num_ops_puncs;
    unsigned long len = 1;

    for (;;) {
	Unichar uc;
	UnicharStream::UnicharResult ures;
	ures = unichar_stream->getNextUnichar (&uc);
	if (ures != UnicharStream::UnicharNormal) {
	    if (best_fit == NULL)
		return 0;

	    return best_fit_len;
	}

	unsigned long new_nmatched = 0;
	unsigned long i;
	for (i = 0; i < nmatched; i++) {
	    const char *candidate = matched [i];

	    if (*candidate &&
		utf8_valid_to_unichar (candidate) == uc)
	    {
		matched_arr [new_nmatched] = utf8_valid_next (candidate);
		if (!*matched_arr [new_nmatched]) {
		    best_fit = candidate;
		    best_fit_len = len;
		} else
		    new_nmatched ++;
	    }
	}

	if (new_nmatched == 0)
	    break;

	matched = matched_arr;
	nmatched = new_nmatched;
	len ++;
    }

    if (best_fit == NULL) {
      DEBUG_INT (
	errf->print ("Scruffy.matchPreprocessingOpOrPunc: "
		     "returning 0")
	     .pendl ();
      );

	return 0;
    }

  DEBUG_INT (
    errf->print ("Scruffy.matchPreprocessingOpOrPunc: "
		 "returning ")
	 .print (countStrLength (best_fit))
	 .print (" (best fit: ")
	 .print (best_fit)
	 .print (")")
	 .pendl ();
  );

    return best_fit_len;
}

/* Header names only appear within a #include preprocessing directive,
 * hence this funcion is separate from matchPreprocessingToken().
 *
 * header-name:
 *     <h-char-sequence>
 *     "q-char-sequence"
 *
 * h-char-sequence:
 *     h-char
 *     h-char-sequence h-char
 *
 * h-char:
 *     any member of the source character set except
 *         new-line and >
 *
 * q-char-sequence:
 *     q-char
 *     q-char-sequence q-char
 *
 * q-char:
 *     any member of the source character set except
 *         new-line and "
 */
Size
matchHeaderName (UnicharStream *unichar_stream,
		 PpToken_HeaderName::HeaderNameType *ret_hn_type,
		 Ref<String> *ret_header_name)
    throw (InternalException,
	   ParsingException)
{
    if (ret_hn_type != NULL) {
	// Setting to any valid value
	*ret_hn_type = PpToken_HeaderName::HeaderNameType_Q;
    }

    if (ret_header_name != NULL)
	*ret_header_name = NULL;

    Size total_len = 0;

    Unichar uc;
    UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
    if (res != UnicharStream::UnicharNormal)
	return 0;

    total_len ++;

    Ref<String> res_str;
    Unichar term_char;
    PpToken_HeaderName::HeaderNameType header_name_type;
    if (uc == 0x3c /* '<' */) {
	term_char = 0x3e /* '>' */;
	header_name_type = PpToken_HeaderName::HeaderNameType_H;
    } else
    if (uc == 0x22 /* '"' */) {
	term_char = 0x22 /* '"' */;
	header_name_type = PpToken_HeaderName::HeaderNameType_Q;
    } else
	return 0;

    Size num_chars = 0;
    for (;;) {
	res = unichar_stream->getNextUnichar (&uc);
	if (res != UnicharStream::UnicharNormal)
	    return 0;

	total_len ++;

	if (uc == term_char) {
	    if (num_chars == 0)
		return 0;

	    break;
	}

	if (isNewline (uc))
	    return 0;

	num_chars ++;

	Byte str_utf8 [6];
	Size str_utf8_len = unichar_to_utf8 (uc, str_utf8);

	if (res_str == NULL)
	    res_str = grab (new String (ConstMemoryDesc (str_utf8, str_utf8_len)));
	else
	    res_str = String::forPrintTask (Pt (res_str) (ConstMemoryDesc (str_utf8, str_utf8_len)));
    }

    if (ret_header_name != NULL)
	*ret_header_name = res_str;

    if (ret_hn_type != NULL)
	*ret_hn_type = header_name_type;

    return total_len;
}

/* preprocessing-token:
 *     header-name
 *     identifier
 *     pp-number
 *     character-literal
 *     string-literal
 *     preprocessing-op-or-punc
 *     each non-white-space character that cannot be one of the above
 */
// mt_throws ((InternalException, ParsingException))
unsigned long
matchPreprocessingToken (UnicharStream *unichar_stream,
			 PpTokenType   *ret_pp_token_type)
{
    if (unichar_stream == NULL)
	abortIfReached ();

    if (ret_pp_token_type != NULL)
	*ret_pp_token_type = PpTokenInvalid;

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

    unsigned long identifier_len = matchIdentifier (unichar_stream);

    unichar_stream->setPosition (pmark);
    unsigned long pp_number_len = matchPpNumber (unichar_stream);

    unichar_stream->setPosition (pmark);
    unsigned long character_literal_len = matchCharacterLiteral (unichar_stream);

    unichar_stream->setPosition (pmark);
    unsigned long string_literal_len = matchStringLiteral (unichar_stream);

    unichar_stream->setPosition (pmark);
    unsigned long preprocessing_op_or_punc_len = matchPreprocessingOpOrPunc (unichar_stream);

    const unsigned long nnumbers = 5;
    unsigned long number_set [nnumbers] = {
	identifier_len,
	pp_number_len,
	character_literal_len,
	string_literal_len,
	preprocessing_op_or_punc_len
    };

    if (numbersAreZero (number_set, nnumbers)) {
	unichar_stream->setPosition (pmark);
	if (matchWhitespace (unichar_stream, NULL) == 0) {
	    unichar_stream->setPosition (pmark);

	    Unichar uc;
	    UnicharStream::UnicharResult ures;
	    ures = unichar_stream->getNextUnichar (&uc);
	    if (ures == UnicharStream::UnicharNormal) {
	      DEBUG_INT (
		errf->print ("Scruffy.matchPreprocessingToken: "
			     "single non-whitespace character: ")
		     .print ((char) uc).print (", 0x").printHex ((unsigned long) uc)
		     .pendl ();
	      );

		if (ret_pp_token_type != NULL)
		    *ret_pp_token_type = PpTokenSingleChar;

		return 1;
	    }
	}

	if (ret_pp_token_type != NULL)
	    *ret_pp_token_type = PpTokenInvalid;

	return 0;
    }

    PpTokenType pp_token_type = PpTokenInvalid;

    /* Alternative tokens are not identifiers => 'preprocessing-op-or-punc'
     * should be matched before 'identifier'. */
    if (numberIsMaxOf (preprocessing_op_or_punc_len, number_set, nnumbers)) {
      DEBUG_INT (
	errf->print ("Scruffy.matchPreprocessingToken: preprocessing-op-or-punc")
	     .pendl ();
      );

	pp_token_type = PpTokenPpOpOrPunc;
    } else
    /* Alternative tokens are not identifiers => 'preprocessing-op-or-punc'
     * should be matched before 'identifier'. */
    if (numberIsMaxOf (identifier_len, number_set, nnumbers)) {
      DEBUG_INT (
	errf->print ("Scruffy.matchPreprocessingToken: identifier")
	     .pendl ();
      );

	pp_token_type = PpTokenIdentifier;
    } else
    if (numberIsMaxOf (pp_number_len, number_set, nnumbers)) {
      DEBUG_INT (
	errf->print ("Scruffy.matchPreprocessingToken: pp-number")
	     .pendl ();
      );

	pp_token_type = PpTokenPpNumber;
    } else
    if (numberIsMaxOf (character_literal_len, number_set, nnumbers)) {
      DEBUG_INT (
	errf->print ("Scruffy.matchPreprocessingToken: character-literal")
	     .pendl ();
      );

	pp_token_type = PpTokenCharacterLiteral;
    } else
    if (numberIsMaxOf (string_literal_len, number_set, nnumbers)) {
      DEBUG_INT (
	errf->print ("Scruffy.matchPreprocessingToken: string-literal")
	     .pendl ();
      );

	pp_token_type = PpTokenStringLiteral;
    }

    if (ret_pp_token_type != NULL)
	*ret_pp_token_type = pp_token_type;

    return maxNumberOf (number_set, nnumbers);
}

/* decimal-literal:
 *     nonzero-digit
 *     decimal-literal digit
 */
Size
matchDecimalLiteral (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    bool first_digit = true;
    for (;;) {
	Unichar uc;
	UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
	if (res != UnicharStream::UnicharNormal)
	    return total_len;

	if (first_digit) {
	    first_digit = false;
	    if (!isNonzeroDigit (uc))
		break;
	} else {
	    if (!isDigit (uc))
		break;
	}

	total_len ++;
    }

    return total_len;
}

/* octal-literal:
 *     0
 *     octal-literal octal-digit
 */
Size
matchOctalLiteral (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    bool first_digit = true;
    for (;;) {
	Unichar uc;
	UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
	if (res != UnicharStream::UnicharNormal)
	    return total_len;

	if (first_digit) {
	    first_digit = false;
	    if (uc != 0x30 /* '0' */)
		break;
	} else {
	    if (!isOctalDigit (uc))
		break;
	}

	total_len ++;
    }

    return total_len;
}

/* hexadecimal-literal:
 *     0x hexadecimal-digit
 *     0X hexadecimal-digit
 *     hexadecimal-literal hexadecimal-digit
 */
Size
matchHexadecimalLiteral (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    Size digit_number = 0;
    for (;;) {
	Unichar uc;
	UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
	if (res != UnicharStream::UnicharNormal) {
	    if (digit_number > 2)
		return total_len;

	    return 0;
	}

	if (digit_number > 1) {
	    digit_number ++;
	    if (!isHexadecimalDigit (uc))
		break;
	} else
	if (digit_number > 0) {
	    digit_number ++;
	    if (uc != 0x058 /* 'X' */ &&
		uc != 0x078 /* 'x' */)
	    {
		return 0;
	    }
	} else {
	    digit_number ++;
	    if (uc != 0x30 /* '0' */)
		return 0;
	}

	total_len ++;
    }

    return total_len;
}

/* unsigned-suffix: one of
 *     u U
 */
bool
isUnsignedSuffix (Unichar uc)
{
    if (uc == 0x75 /* 'u' */ ||
	uc == 0x55 /* 'U' */)
    {
	return true;
    }

    return false;
}

/* long-suffix: one of
 *     l L
 */
bool
isLongSuffix (Unichar uc)
{
    if (uc == 0x6c /* 'l' */ ||
	uc == 0x4c /* 'L' */)
    {
	return true;
    }

    return false;
}

/* integer-suffix:
 *     unigned-suffix long-suffix_{opt}
 *     long-suffix unsigned-suggix_{opt}
 */
Size
matchIntegerSuffix (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    Unichar uc;
    UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
    if (res != UnicharStream::UnicharNormal)
	return 0;

    if (isUnsignedSuffix (uc)) {
	total_len ++;
	res = unichar_stream->getNextUnichar (&uc);
	if (res != UnicharStream::UnicharNormal)
	    return total_len;

	if (isLongSuffix (uc))
	    total_len ++;
    } else
    if (isLongSuffix (uc)) {
	total_len ++;
	res = unichar_stream->getNextUnichar (&uc);
	if (res != UnicharStream::UnicharNormal)
	    return total_len;

	if (isUnsignedSuffix (uc))
	    total_len ++;
    }

    return total_len;
}

/* integer-literal:
 *     decimal-literal integer-suffix_{opt}
 *     octal-literal integer-suffix_{opt}
 *     hexadecimal-literal integer-suffix_{opt}
 */
Size
matchIntegerLiteral (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
  // TODO It would be nice to get a value for the number immediately
  // and store it in a subclass of class Token.

    Size total_len = 0;

    Ref<UnicharStream::PositionMarker> pos = unichar_stream->getPosition ();

    Size decimal_literal_len = matchDecimalLiteral (unichar_stream);

    unichar_stream->setPosition (pos);
    Size octal_literal_len = matchOctalLiteral (unichar_stream);

    unichar_stream->setPosition (pos);
    Size hexadecimal_literal_len = matchHexadecimalLiteral (unichar_stream);

    const Size num_numbers = 3;
    unsigned long number_set [num_numbers] = {
	decimal_literal_len,
	octal_literal_len,
	hexadecimal_literal_len
    };

    if (numbersAreZero (number_set, num_numbers)) {
	DEBUG (
	    errf->print ("Scruffy.Preprocessor.matchIntegerLiteral: "
			 "numbers are zero").pendl ();
	)
	return 0;
    }

    if (numberIsMaxOf (decimal_literal_len, number_set, num_numbers)) {
	// TODO Parse decimal number
	total_len += decimal_literal_len;
    } else
    if (numberIsMaxOf (octal_literal_len, number_set, num_numbers)) {
	// TODO Parse octal number
	total_len += octal_literal_len;
    } else
    if (numberIsMaxOf (hexadecimal_literal_len, number_set, num_numbers)) {
	// TODO Parse hexadecimal number
	total_len += hexadecimal_literal_len;
    } else
	abortIfReached ();

    unichar_stream->setPosition (pos);
    unichar_stream->skip (total_len);
    total_len += matchIntegerSuffix (unichar_stream);

    return total_len;
}

/* digit-sequence:
 *     digit
 *     digit-sequence digit
 */
Size
matchDigitSequence (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    for (;;) {
	Unichar uc;
	UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
	if (res != UnicharStream::UnicharNormal)
	    return total_len;

	if (!isDigit (uc))
	    break;

	total_len ++;
    }

    return total_len;
}

/* fractional-constant:
 *     digit-sequence_{opt} . digit-sequence
 *     digit-sequence .
 */
Size
matchFractionalConstant (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

    Size digit_sequence_len = matchDigitSequence (unichar_stream);
    // True is fractional-constant starts with a digit sequence.
    bool start_ds = digit_sequence_len > 0;
    total_len += digit_sequence_len;

    unichar_stream->setPosition (pmark);
    unichar_stream->skip (digit_sequence_len);

    Unichar uc;
    UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
    if (res != UnicharStream::UnicharNormal)
	return 0;

    if (uc != 0x2e /* '.' */)
	return 0;

    total_len ++;

    if (start_ds)
	total_len += matchDigitSequence (unichar_stream);

    return total_len;
}

/* exponent-part:
 *     e sign_{opt} digit-sequence
 *     E sign_{opt} digit-sequence
 */
Size
matchExponentPart (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    Unichar uc;
    UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
    if (res != UnicharStream::UnicharNormal)
	return 0;

    if (uc != 0x65 /* 'e' */ ||
	uc != 0x45 /* 'E' */)
    {
	return 0;
    }

    total_len ++;

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();
    res = unichar_stream->getNextUnichar (&uc);
    if (isSign (uc))
	total_len ++;
    else
	unichar_stream->setPosition (pmark);

    Size digit_sequence_len = matchDigitSequence (unichar_stream);
    if (digit_sequence_len == 0)
	return 0;

    total_len += digit_sequence_len;

    return total_len;
}

/* floating-suffix: one of
 *     f l F L
 */
Size
matchFloatingSuffix (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Unichar uc;
    UnicharStream::UnicharResult res = unichar_stream->getNextUnichar (&uc);
    if (res != UnicharStream::UnicharNormal)
	return 0;

    if (uc == 0x66 /* 'f' */ ||
	uc == 0x6c /* 'l' */ ||
	uc == 0x46 /* 'F' */ ||
	uc == 0x4c /* 'L' */)
    {
	return 1;
    }

    return 0;
}

/* floating-literal:
 *     fractional-constant exponent-part_{opt} floating-suffix_{opt}
 *     digit-sequence exponent-part floating_suffix_{opt}
 */
Size
matchFloatingLiteral (UnicharStream *unichar_stream)
    throw (ParsingException,
	   InternalException)
{
    Size total_len = 0;

    Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

    Size fractional_constant_len = matchFractionalConstant (unichar_stream);
    if (fractional_constant_len > 0) {
	total_len += fractional_constant_len;

	unichar_stream->setPosition (pmark);
	unichar_stream->skip (fractional_constant_len);

	pmark = unichar_stream->getPosition ();

	Size exponent_part_len = matchExponentPart (unichar_stream);
	total_len += exponent_part_len;

	unichar_stream->setPosition (pmark);
	unichar_stream->skip (exponent_part_len);

	total_len += matchFloatingSuffix (unichar_stream);

	return total_len;
    }

    unichar_stream->setPosition (pmark);
    Size digit_sequence_len = matchDigitSequence (unichar_stream);
    if (digit_sequence_len > 0) {
	total_len += digit_sequence_len;

	unichar_stream->setPosition (pmark);
	unichar_stream->skip (digit_sequence_len);

	pmark = unichar_stream->getPosition ();

	Size exponent_part_len = matchExponentPart (unichar_stream);
	if (exponent_part_len == 0)
	    return 0;

	total_len += exponent_part_len;

	unichar_stream->setPosition (pmark);
	unichar_stream->skip (exponent_part_len);

	total_len += matchFloatingSuffix (unichar_stream);
    }

    return total_len;
}

// TODO We call this a lot of times for the same pp-number-token. Cache the result.
Ref<Token>
ppNumberToken_to_Token (PpToken *pp_number_token)
    throw (ParsingException)
{
    abortIf (pp_number_token == NULL ||
	     pp_number_token->pp_token_type != PpTokenPpNumber);

    DEBUG (
	errf->print ("Scruffy.Preprocessor.ppNumberToken_to_Token: "
		     "\"").print (pp_number_token->str).print ("\"").pendl ();
    )

    Ref<Token> token;

    Size total_len = 0;

    try {
	Ref<DirectArrayFile> file = grab (new DirectArrayFile (pp_number_token->str->getMemoryDesc ()));
	Ref<FileByteStream> byte_stream = grab (new FileByteStream (file));
	Ref<Utf8UnicharStream> unichar_stream = grab (new Utf8UnicharStream (byte_stream));

	Ref<UnicharStream::PositionMarker> pmark = unichar_stream->getPosition ();

	Size integer_literal_len = matchIntegerLiteral (unichar_stream);
	if (integer_literal_len > 0) {
	    unichar_stream->setPosition (pmark);
	    unichar_stream->skip (integer_literal_len);
	    if (unichar_stream->getNextUnichar (NULL) != UnicharStream::UnicharEof) {
		DEBUG (
		    errf->print ("Scruffy.Preprocessor.ppNumberToken_to_Token: "
				 "integer_literal_len (").print (integer_literal_len).print (") not at eof").pendl ();
		)
		integer_literal_len = 0;
	    }
	}

	unichar_stream->setPosition (pmark);
	Size floating_literal_len = matchFloatingLiteral (unichar_stream);
	if (floating_literal_len > 0) {
	    unichar_stream->setPosition (pmark);
	    unichar_stream->skip (floating_literal_len);
	    if (unichar_stream->getNextUnichar (NULL) != UnicharStream::UnicharEof) {
		DEBUG (
		    errf->print ("Scruffy.Preprocessor.ppNumberToken_to_Token: "
				 "floating_literal_len (").print (floating_literal_len).print (") not at eof").pendl ();
		)
		floating_literal_len = 0;
	    }
	}

	const Size num_numbers = 2;
	unsigned long number_set [num_numbers] = {
	    integer_literal_len,
	    floating_literal_len
	};

	if (numbersAreZero (number_set, num_numbers)) {
	    DEBUG (
		errf->print ("Scruffy.Preprocessor.ppNumberToken_to_Token: numbers are zero").pendl ();
	    )
	    throw ParsingException (pp_number_token->fpos);
	}

	if (numberIsMaxOf (integer_literal_len, number_set, num_numbers)) {
	    total_len += integer_literal_len;
	    token = grab (new Literal (LiteralInteger, pp_number_token->str, pp_number_token->fpos));
	} else
	if (numberIsMaxOf (floating_literal_len, number_set, num_numbers)) {
	    total_len += integer_literal_len;
	    token = grab (new Literal (LiteralFloating, pp_number_token->str, pp_number_token->fpos));
	} else
	    abortIfReached ();
    } catch (ParsingException &exc) {
	exc.raise ();
    } catch (InternalException &exc) {
	printException (errf, exc);
	abortIfReached ();
    }

    return token;
}

#if 0
/* (see 2.11) */
bool
isKeyword (const char *str)
{
    return false;
}

/* integer-literal:
 *     decimal-literal ingeter-suffix_{opt}
 *     octal-literal integer-suffix_{opt}
 *     hexadecimal-literal integer-suffix_{opt}
 */
bool
isIntegerLiteral (const char *str)
{
    return false;
}

/* floating-literal:
 *     fractional-constant exponent-part_{opt} floating-suffix_{opt}
 *     digit-sequence exponent-part floating_suffix_{opt}
 */
bool
isFloatingLiteral (const char *str)
{
    return false;
}

/* boolean-literal:
 *     false
 *     true
 */
bool
isBooleanLiteral (const char *str)
{
    return false;
}

/* literal:
 *     integer-literal
 *     character-literal
 *     floating-literal
 *     string-literal
 *     boolean-literal
 */
bool
isLiteral (const char *str)
{
/*
    if (isIntegerLiteral (str)   ||
	isCharacterLiteral (str) ||
	isFloatingLiteral (str)  ||
	isStringLiteral (str)    ||
	isBooleanLiteral (str))
    {
	return true;
    }
*/

    return false;
}

/* token:
 *     identifier
 *     keyword
 *     literal
 *     operator
 *     punctuator
 */
bool
isToken (const char *str)
{
/*
    if (isIdentifier (str) ||
	isKeyword (str)    ||
	isLiteral (str)    ||
	isOperator (str)   ||
	isPunctuator (str))
    {
	return true;
    }
*/
    return false;
}

/* (see 2.11) */
bool
isAlternativeRepresentation (const char *str)
{
    return false;
}

/* decimal-literal:
 *     nonzero-digit
 *     decimal-literal digit
 */
bool
isDecimalLiteral (const char *str)
{
    return false;
}

/* octal-literal:
 *     0
 *     octal-literal octal-digit
 */
bool
isOctalLiteral (const char *str)
{
    return false;
}

/* hexadecimal-literal:
 *     0x hexadecimal-digit
 *     0X hexadecimal-digit
 *     hexadecimal-literal hexadecimal-digit
 */
bool
isHexadecimalLiteral (const char *str)
{
    return false;
}

/* nonzero-digit: one of
 *     1 2 3 4 5 6 7 8 9
 */
bool
isNonzeroDigit (const char *str)
{
    return false;
}

/* integer-suffix:
 *     unigned-suffix long-suffix_{opt}
 *     long-suffix unsigned-suggix_{opt}
 */
bool
isIntegerSuffix (const char *str)
{
    return false;
}

/* unsigned-suffix: one of
 *     u U
 */
bool
isUnsignedSuffix (const char *str)
{
    return false;
}

/* long-suffix: one of
 *     1 L
 */
bool
isLongSuffix (const char *str)
{
    return false;
}

/* fractional-constant:
 *     digit-sequence_{opt} . digit-sequence
 *     digit-sequence .
 */
bool
isFractionalConstant (const char *str)
{
    return false;
}

/* exponent-part:
 *     e sign_{opt} digit-sequence
 *     E sign_{opt} digit-sequence
 */
bool
isExponentPart (const char *str)
{
    return false;
}

/* digit-sequence:
 *     digit
 *     digit-sequence digit
 */
bool
isDigitSequence (const char *str)
{
    return false;
}

/* floating-suffix: one of
 *     f l F L
 */
bool
isFloatingSuffix (const char *str)
{
    return false;
}

#endif

bool
compareReplacementLists (List< Ref<PpItem> > *left,
			 List< Ref<PpItem> > *right)
{
    if (left == NULL ||
	left->first == NULL)
    {
	if (right == NULL ||
	    right->first == NULL)
	{
	    return true;
	}

	return false;
    } else {
	if (right == NULL ||
	    right->first == NULL)
	{
	    return false;
	}
    }

    List< Ref<PpItem> >::Element *cur_left  = left->first,
				     *cur_right = right->first;
    while (cur_left  != NULL &&
	   cur_right != NULL)
    {
	Ref<PpItem> &left_pp_item  = cur_left->data,
		    &right_pp_item = cur_right->data;

      /* DEBUG
	if (left_pp_item->type == PpItemWhitespace)
	    errf->print ("--- left: whitespace").pendl ();
	else
	if (left_pp_item->type == PpItemPpToken)
	    errf->print ("--- left: pp_token").pendl ();
	else
	    abortIfReached ();

	if (right_pp_item->type == PpItemWhitespace)
	    errf->print ("--- right: whitespace").pendl ();
	else
	if (right_pp_item->type == PpItemPpToken)
	    errf->print ("--- right: pp_token").pendl ();
	else
	    abortIfReached ();
       (DEBUG) */

	if (left_pp_item->type  == PpItemWhitespace &&
	    right_pp_item->type == PpItemWhitespace)
	{
	    /* No-op */
	} else
	if (left_pp_item->type  == PpItemPpToken &&
	    right_pp_item->type == PpItemPpToken)
	{
	    PpToken *left_pp_token  = static_cast <PpToken*> (left_pp_item.ptr ()),
		    *right_pp_token = static_cast <PpToken*> (right_pp_item.ptr ());

	    if (!compareStrings (left_pp_token->str->getData (),
				 right_pp_token->str->getData ()))
	    {
		break;
	    }
	} else
	    break;

	cur_left  = cur_left->next;
	cur_right = cur_right->next;
    }

  /* DEBUG
    if (cur_left != NULL)
	errf->print ("--- left is non-null").pendl ();

    if (cur_right != NULL)
	errf->print ("--- right is non-null").pendl ();
   (DEBUG) */

    if (cur_left  != NULL ||
	cur_right != NULL)
    {
	return false;
    }

    return true;
}

Ref<String>
ppItemsToString (List< Ref<PpItem> > *pp_items)
    throw (ParsingException,
	   InternalException)
{
    abortIf (pp_items == NULL);

    Ref<String> str = grab (new String);

    List< Ref<PpItem> >::DataIterator pp_item_iter (*pp_items);
    while (!pp_item_iter.done ()) {
	Ref<PpItem> &pp_item = pp_item_iter.next ();

	// TODO This is ineffective
	str = String::forPrintTask (Pt (str) (pp_item->str));
    }

    return str;
}

Ref<String>
spellPpItems (List< Ref<PpItem> > *pp_items)
    throw (InternalException,
	   ParsingException)
{
    if (pp_items == NULL)
	return grab (new String ("\"\""));

    const char *backslash    = "\\",
	       *double_quote = "\"",
	       *whitespace   = " ";

    if (!utf8_validate_sz (backslash, NULL)    ||
	!utf8_validate_sz (double_quote, NULL) ||
	!utf8_validate_sz (whitespace, NULL))
    {
	throw InternalException ();
    }

    unsigned long backslash_len    = countStrLength (backslash),
		  double_quote_len = countStrLength (double_quote),
		  whitespace_len   = countStrLength (whitespace);

    const Unichar backslash_uc    = utf8_valid_to_unichar (backslash),
		  double_quote_uc = utf8_valid_to_unichar (double_quote);

    unsigned long len = double_quote_len + double_quote_len;

    List< Ref<PpItem> >::Element *cur = pp_items->first;
    while (cur != NULL) {
	Ref<PpItem> &pp_item = cur->data;
	if (pp_item->type == PpItemWhitespace) {
	    if (cur->previous != NULL &&
		cur->next != NULL)
	    {
		len ++;
	    }
	} else
	if (pp_item->type == PpItemPpToken) {
	    PpToken *pp_token = static_cast <PpToken*> (pp_item.ptr ());

	    if (!utf8_validate_sz (pp_token->str->getData (), NULL))
		throw InternalException ();

	    len += pp_token->str->getLength ();

	    if (pp_token->pp_token_type == PpTokenCharacterLiteral ||
		pp_token->pp_token_type == PpTokenStringLiteral)
	    {
		unsigned long len_bonus = 0;

		const char *uchar = pp_token->str->getData ();
		while (*uchar) {
		    const Unichar uc = utf8_valid_to_unichar (uchar);

		    if (uc == backslash_uc)
			len_bonus += backslash_len;
		    else
		    if (uc == double_quote_uc)
			len_bonus += double_quote_len;

		    uchar = utf8_valid_next (uchar);
		}

		len += len_bonus;
	    }
	} else
	    abortIfReached ();

	cur = cur->next;
    }

    Ref<String> spelling = grab (new String ());
    spelling->allocate (len);

    char *dst_str = spelling->getData ();
    unsigned long pos = 0;

    copyMemory (MemoryDesc (dst_str + pos, double_quote_len), ConstMemoryDesc (double_quote, double_quote_len));
    pos += double_quote_len;

    cur = pp_items->first;
    while (cur != NULL) {
	Ref<PpItem> &pp_item = cur->data;
	if (pp_item->type == PpItemWhitespace) {
	    if (cur->previous != NULL &&
		cur->next != NULL)
	    {
		Whitespace * const whsp = static_cast <Whitespace*> (pp_item.ptr());
		if (whsp->has_newline) {
		    dst_str [pos] = '\n';
		    ++pos;
		} else {
		    copyMemory (MemoryDesc (dst_str + pos, whitespace_len), ConstMemoryDesc (whitespace, whitespace_len));
		    pos += whitespace_len;
		}
	    }
	} else
	if (pp_item->type == PpItemPpToken) {
	    PpToken *pp_token = static_cast <PpToken*> (pp_item.ptr ());

	    if (pp_token->pp_token_type == PpTokenCharacterLiteral ||
		pp_token->pp_token_type == PpTokenStringLiteral)
	    {
		const char *uchar = pp_token->str->getData ();
		while (*uchar) {
		    unsigned long uchar_len = utf8_valid_char_len (uchar);
		    const Unichar uc = utf8_valid_to_unichar (uchar);

		    if (uc == backslash_uc) {
			copyMemory (MemoryDesc (dst_str + pos, backslash_len), ConstMemoryDesc (backslash, backslash_len));
			pos += backslash_len;
		    } else
		    if (uc == double_quote_uc) {
			copyMemory (MemoryDesc (dst_str + pos, backslash_len), ConstMemoryDesc (backslash, backslash_len));
			pos += backslash_len;
		    }

		    copyMemory (MemoryDesc (dst_str + pos, uchar_len), ConstMemoryDesc (uchar, uchar_len));
		    pos += uchar_len;

		    uchar = utf8_valid_next (uchar);
		}
	    } else {
		copyMemory (MemoryDesc (dst_str + pos, pp_token->str->getLength ()),
			    ConstMemoryDesc (pp_token->str->getData (), pp_token->str->getLength ()));
		pos += pp_token->str->getLength ();
	    }
	} else
	    abortIfReached ();

	cur = cur->next;
    }

    copyMemory (MemoryDesc (dst_str + pos, double_quote_len), ConstMemoryDesc (double_quote, double_quote_len));
    pos += double_quote_len;

    // FIXME ? remove this 'if' block altogether?
    if (pos != len) {
	errf->print ("--- pos: ")
	     .print (pos)
	     .print (", len: ")
	     .print (len)
	     .pendl ();
	abortIfReached ();
    }

    dst_str [len] = 0;

    return spelling;
}

// TODO What about boolean literals?
Ref<Token>
ppTokenToToken (PpToken *pp_token)
    throw (ParsingException)
{
    if (pp_token == NULL)
	abortIfReached ();

    Ref<Token> token;

    if (pp_token->pp_token_type == PpTokenSingleChar) {
	throw ParsingException (pp_token->fpos);
    } else
    if (pp_token->pp_token_type == PpTokenHeaderName) {
	throw ParsingException (pp_token->fpos);
    } else
    if (pp_token->pp_token_type == PpTokenPpOpOrPunc) {
	if (matchStrings (pp_token->str->getData (), operators, num_operators))
	    token = grab (new Token (TokenOperator, pp_token->str, pp_token->fpos));
	else
	    token = grab (new Token (TokenPunctuator, pp_token->str, pp_token->fpos));
    } else
    if (pp_token->pp_token_type == PpTokenIdentifier) {
	if (compareStrings (pp_token->str->getData (), "true") ||
	    compareStrings (pp_token->str->getData (), "false"))
	{
	    token = grab (new Literal (LiteralBoolean, pp_token->str, pp_token->fpos));
	} else
	if (is_keyword (pp_token->str->getMemoryDesc ()))
	    token = grab (new Token (TokenKeyword, pp_token->str, pp_token->fpos));
	else
	    token = grab (new Token (TokenIdentifier, pp_token->str, pp_token->fpos));
    } else
    if (pp_token->pp_token_type == PpTokenPpNumber) {
	token = ppNumberToken_to_Token (pp_token);
    } else
    if (pp_token->pp_token_type == PpTokenCharacterLiteral) {
	token = grab (new Literal (LiteralCharacter, pp_token->str, pp_token->fpos));
    } else
    if (pp_token->pp_token_type == PpTokenStringLiteral) {
	token = grab (new Literal (LiteralString, pp_token->str, pp_token->fpos));
    } else
    if (pp_token->pp_token_type == PpTokenInvalid) {
	throw ParsingException (pp_token->fpos);
    } else {
	/* Unreachable */
	abortIfReached ();
    }

    return token;
}

void
ppTokensToTokens (List< Ref<PpToken> > * const pp_tokens  /* non-null */,
		  List< Ref<Token> >   * const ret_tokens /* non-null */)
{
    List< Ref<PpToken> >::DataIterator iter (*pp_tokens);
    while (!iter.done ()) {
	Ref<PpToken> &pp_token = iter.next ();
	ret_tokens->append (ppTokenToToken (pp_token));
    }
}

void
ppItemsToTokens (List< Ref<PpItem> > * const pp_items   /* non-null */,
		 List< Ref<Token> >  * const ret_tokens /* non-null */)
{
    List< Ref<PpItem> >::DataIterator iter (*pp_items);
    while (!iter.done ()) {
	Ref<PpItem> &pp_item = iter.next ();
	if (pp_item->type == PpItemPpToken)
	    ret_tokens->append (ppTokenToToken (static_cast <PpToken*> (pp_item.ptr ())));
    }
}

}
