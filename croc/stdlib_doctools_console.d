/******************************************************************************
This module contains the 'doctools.console' module of the standard library.

License:
Copyright (c) 2013 Jarrett Billingsley

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software in a
	product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not
	be misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.
******************************************************************************/

module croc.stdlib_doctools_console;

import croc.ex;
import croc.types;

// ================================================================================================================================================
// Public
// ================================================================================================================================================

public:

void initDoctoolsConsoleLib(CrocThread* t)
{
	importModuleFromStringNoNS(t, "doctools.console", Code, __FILE__);
}

// ================================================================================================================================================
// Private
// ================================================================================================================================================

private:

const char[] Code =
`/**
This module defines a means of outputting docs to the console (or any console-like stream).
*/
module doctools.console

import ascii:
	isSpace,
	toUpper

import docs:
	docsOf,
	childDocs,
	metamethodDocs

import doctools.output:
	SectionOrder,
	DocOutputter,
	OutputDocVisitor,
	toHeader,
	numToLetter,
	numToRoman

local helpVisitor

/**
Get help on a program object and print it to the standard output stream. This function is useful for Croc CLIs.

If \tt{obj} is a string, it must name a Croc type, and \tt{child} must also be a string. In this case it looks up the
docs for the metamethod named \tt{child} for the type named \tt{obj}.

If \tt{obj} is not a string, and \tt{child} is null, the docs for \tt{obj} are printed.

If \tt{obj} is not a string, and \tt{child} is not null, the docs for the child named \tt{child} inside \tt{obj} are
printed.
*/
function help(obj, child: string = null)
{
	local docs

	if(isString(obj))
	{
		if(child is null)
			throw TypeError("A child name must be given when looking up docs on a builtin type's metamethod")

		docs = metamethodDocs(obj, child)
	}
	else if(child is null)
		docs = docsOf(obj)
	else
		docs = childDocs(obj, child)

	if(docs)
	{
		helpVisitor ?= OutputDocVisitor(SectionOrder(), BasicConsoleOutputter())
		helpVisitor.visitItem(docs)
	}
	else
		writeln("<no help available>")
}

local function splitAtWS(s: string, len: int)
{
	if(#s == 0)
		return null, null

	local firstWS, lastWS = 0, 0
	local foundWS = false
	local sbegin = s[..len]

	foreach(i, ch; sbegin, "reverse")
	{
		if(foundWS)
		{
			if(isSpace(ch))
				firstWS = i
			else
				break
		}
		else if(isSpace(ch))
		{
			foundWS = true
			firstWS = i
			lastWS = i
		}
	}

	if(foundWS)
	{
		if(firstWS == 0)
			return null, s.lstrip() // beginning of string is whitespace; split it there.
		else if(lastWS == #s - 1)
			return sbegin.rstrip(), s.lstrip() // whitespace either ends at or is overlapping split point.
		else
		{
			// whitespace is somewhere IN the beginning, but there's some stuff before the split point.
			return s[.. firstWS], s[lastWS + 1 ..]
		}
	}
	else
		return null, s // no whitespace found, shove it all to the next line
}

/**
A simple implementation of the \link{docs.output.DocOutputter} class which outputs docs to the console (by default, but
can be redirected).

No special terminal features are assumed; no ANSI escapes or anything are used. Furthermore the text is wrapped to a
user-settable maximum line length to avoid ugliness.

Links are not translated by this outputter. Only the text within link spans is output.
*/
class BasicConsoleOutputter : DocOutputter
{
	_output
	_listType
	_listCounters

	_lineEnd
	_lineLength = 0
	_consecNewlines = 0
	_alreadyIndented = false
	_isSpecialSection = false

	/**
	\param[lineEnd] is the number of characters to which line will be wrapped. This simply calls \link{setLineLength}
		with this parameter.
	\param[output] is the stream to which the output will be written. The only method that will be called on this is
		\tt{write} with string parameters. Defaults to \link{console.stdout}.
	*/
	this(lineEnd: int = 80, output = console.stdout)
	{
		:_output = output
		:_listType = []
		:_listCounters = []
		:setLineLength(lineEnd)
	}

	/**
	Set the number of characters to which each line will be wrapped.

	\param[lineEnd] is the number of characters. The length will be clamped to a minimum of 40 characters; anything less
		than this will simply set the length to 40.
	*/
	function setLineLength(lineEnd: int)
		:_lineEnd = math.max(lineEnd, 40)

	// =================================================================================================================
	// Item-level stuff

	override function beginModule(doctable: table) :beginItem(doctable)
	override function endModule() :endItem()
	override function beginFunction(doctable: table) :beginItem(doctable)
	override function endFunction() :endItem()

	override function beginClass(doctable: table)
	{
		:beginItem(doctable)
		:_listType.append(null)
	}

	override function endClass()
	{
		:_listType.pop()
		:endItem()
	}

	override function beginNamespace(doctable: table)
	{
		:beginItem(doctable)
		:_listType.append(null)
	}

	override function endNamespace()
	{
		:_listType.pop()
		:endItem()
	}

	override function beginField(doctable: table) :beginItem(doctable)
	override function endField() :endItem()
	override function beginVariable(doctable: table) :beginItem(doctable)
	override function endVariable() :endItem()

	function beginItem(doctable: table) :outputHeader(doctable)
	function endItem() :newline()

	function outputHeader(doctable: table)
	{
		local head = toHeader(doctable, "", true)
		local barLength = math.min(#head + 2, :_lineEnd)
		local headers

		if(doctable.dittos)
		{
			headers = []

			foreach(dit; doctable.dittos)
			{
				local header = toHeader(dit, "", true)
				barLength = math.max(barLength, math.min(#header + 2, :_lineEnd))
				headers ~= header
			}
		}

		local bar = "=".repeat(barLength)

		:outputText(bar)
		:newline()
		:outputText(" ", head)
		:newline()

		if(headers)
		{
			foreach(h; headers)
			{
				:outputText(" ", h)
				:newline()
			}
		}

		:outputText(bar)
		:newline()
	}

	// =================================================================================================================
	// Section-level stuff

	override function beginSection(name: string)
	{
		:beginParagraph()

		if(name !is "docs")
		{
			:beginBold()

			if(name.startsWith("_"))
				:outputText(toUpper(name[1]), name[2..], ":")
			else
				:outputText(toUpper(name[0]), name[1..], ":")

			:endBold()
			:outputText(" ")
		}

		:_isSpecialSection = name is "params" || name is "throws"

		if(:_isSpecialSection)
			:beginDefList()
	}

	override function endSection()
	{
		if(:_isSpecialSection)
		{
			:endDefList()
			:_isSpecialSection = false
		}
	}

	override function beginParameter(doctable: table)
	{
		:beginDefTerm()
		:outputText(doctable.name)
		:endDefTerm()
		:beginDefDef()
	}

	override function endParameter() :endDefDef()

	override function beginException(name: string)
	{
		:beginDefTerm()
		:outputText(name)
		:endDefTerm()
		:beginDefDef()
	}

	override function endException() :endDefDef()

	// =================================================================================================================
	// Paragraph-level stuff

	override function beginParagraph() {}

	override function endParagraph()
	{
		:newline()
		:newline()
	}

	override function beginCode(language: string)
	{
		:newline()
		:newline()
		:outputText("-----")
		:newline()
	}

	override function endCode()
	{
		:newline()
		:outputText("-----")
		:newline()
		:newline()
	}

	override function beginVerbatim(type: string)
	{
		:newline()
		:newline()
		:outputText("-----")
		:newline()
	}

	override function endVerbatim()
	{
		:newline()
		:outputText("-----")
		:newline()
		:newline()
	}

	override function beginBulletList()
	{
		:_listType.append("*")
		:newline()
	}

	override function endBulletList()
	{
		:_listType.pop()
		:newline()
	}

	override function beginNumList(type: string)
	{
		:_listType.append(type)
		:_listCounters.append(1)
		:newline()
	}

	override function endNumList()
	{
		:_listType.pop()
		:_listCounters.pop()
		:newline()
	}

	override function beginListItem()
	{
		assert(#:_listType > 0)
		:newline()

		local type = :_listType[-1]
		local str

		if(type == '*')
			:outputText("* ")
		else
		{
			local count = :_listCounters[-1]

			switch(type)
			{
				case '1': str = toString(count); break
				case 'A': str = numToLetter(count, false); break
				case 'a': str = numToLetter(count, true); break
				case 'I': str = numToRoman(count, false); break
				case 'i': str = numToRoman(count, true); break
				default: throw ValueError("Malformed documentation")
			}

			:outputText(str, ". ")
			:_listCounters[-1]++
		}
	}

	override function endListItem() :newline()

	override function beginDefList() :_listType.append(null)

	override function endDefList()
	{
		:_listType.pop()
		:newline()
	}

	override function beginDefTerm() :newline()

	override function endDefTerm()
	{
		:outputText(": ")
		:_listType.append(null)
		:newline()
	}

	override function beginDefDef() {}

	override function endDefDef()
	{
		:_listType.pop()
		:newline()
	}

	override function beginTable()
	{
		:newline()
		:outputText("<table>")
		:_listType.append(null)
	}

	override function endTable()
	{
		:_listType.pop()
		:newline()
	}

	override function beginRow()
	{
		:newline()
		:outputText("<row>")
		:_listType.append(null)
	}

	override function endRow()
		:_listType.pop()

	override function beginCell()
	{
		:newline()
		:outputText("<cell> ")
	}

	override function endCell() {}

	override function beginBold() :outputText("*")
	override function endBold() :outputText("*")
	override function beginEmphasis() :outputText("_")
	override function endEmphasis() :outputText("_")
	override function beginLink(link: string) {}
	override function endLink() {}
	override function beginMonospace() {}
	override function endMonospace() {}
	override function beginStrikethrough() :outputText("~");
	override function endStrikethrough() :outputText("~");
	override function beginSubscript() :outputText("_")
	override function endSubscript() {}
	override function beginSuperscript() :outputText("^")
	override function endSuperscript() {}
	override function beginUnderline() :outputText("__")
	override function endUnderline() :outputText("__")

	// =================================================================================================================
	// Raw output

	override function outputText(vararg)
	{
		for(i: 0 .. #vararg)
		{
			local s = vararg[i]

			if(#s == 0)
				continue

			:indent()

			local remaining = :_lineEnd - :_lineLength
			local firstLoop = true

			while(true)
			{
				if(#s <= remaining)
				{
					if(#s > 0)
					{
						:baseWrite(s)
						:_lineLength += #s
					}

					break
				}

				local first, second = splitAtWS(s, remaining)

				if(first !is null)
					:baseWrite(first)

				:newline()

				if(second is null)
					break
				else
				{
					if(s is second && !firstLoop)
					{
						// in this case we're looping because s can't be split any further, so just print it
						// and let the console wrap it.
						:baseWrite(s)
						break
					}

					s = second
					:indent()
					remaining = :_lineEnd - :_lineLength
					firstLoop = false
				}
			}
		}
	}

	function newline()
	{
		if(:_consecNewlines < 2)
		{
			:_consecNewlines++
			:_output.write("\n")
			:_lineLength = 0
			:_alreadyIndented = false
		}
	}

	function baseWrite(s: string)
	{
		:_consecNewlines = 0
		:indent()
		:_output.write(s)
	}

	function indent()
	{
		if(:_alreadyIndented || #:_listType == 0)
			return

		local indent = "  ".repeat(#:_listType)
		:_output.write(indent)
		:_lineLength = #indent
		:_alreadyIndented = true
	}
}
`;