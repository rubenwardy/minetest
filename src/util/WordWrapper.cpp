/*
Minetest
Copyright (C) 2021 rubenwardy <rw@rubenwardy>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "WordWrapper.h"

#include <irrString.h>

static const wchar_t SOFT_HYPHEN = 0x00AD;

void WordWrapper::wrap(std::vector<EnrichedString> &wrappedText,
		std::vector<s32> *lineStarts, const EnrichedString &text,
		const core::rect<s32> &bounds, s32 lineHeight) const
{
	s32 elWidth = bounds.getWidth();

	EnrichedString line;
	EnrichedString word;
	s32 size = text.size();
	s32 length = 0;
	EnrichedString whitespace;

	core::rect<s32> lineBounds = bounds;
	lineBounds.LowerRightCorner.Y = lineBounds.UpperLeftCorner.Y + lineHeight;

	s32 lastLineStart = 0;
	s32 lastWordStart = 0;
	bool isLastLine = false;

	for (s32 i = 0; i < size; ++i) {
		wchar_t c = text.getString()[i];

		// New lines
		if (c == L'\r' || c == L'\n') {
			if (c == L'\r' && i < size - 1 &&
					text.getString()[i + 1] == L'\n') {
				// Skip next char
				i++;
			}

			if (line.empty())
				lastLineStart = lastWordStart;

			line += whitespace;
			line += word;

			word.clear();
			whitespace.clear();

			if (isLastLine) {
				if (i < size - 1)
					line.addCharNoColor(L'…');
				wrappedText.push_back(line);
				if (lineStarts)
					lineStarts->push_back(lastLineStart);
				return;
			}

			wrappedText.push_back(line);
			if (lineStarts)
				lineStarts->push_back(lastLineStart);

			line.clear();
			length = 0;

			lineBounds.UpperLeftCorner.Y += lineHeight;
			lineBounds.LowerRightCorner.Y += lineHeight;
			isLastLine = lineBounds.LowerRightCorner.Y + lineHeight >
				     bounds.LowerRightCorner.Y;
			if (isLastLine)
				elWidth -= getTextWidth(L"…");

			continue;
		}

		bool isWhitespace = c == L' ' || c == 0 || c == SOFT_HYPHEN;
		if (!isWhitespace) {
			if (word.empty())
				lastWordStart = i;
			word.addChar(text, i);
		}

		if (!isWhitespace && i != size - 1)
			continue;

		// Finish word
		if (!word.empty()) {
			const s32 whitespaceWidth = getTextWidth(whitespace.getString());
			const s32 wordWidth = getTextWidth(word.getString());

			if (line.empty())
				lastLineStart = lastWordStart;

			if (length > 0 &&
					length + whitespaceWidth + wordWidth > elWidth) {
				if (isLastLine) {
					line.addCharNoColor(L'…');
					wrappedText.push_back(line);
					if (lineStarts)
						lineStarts->push_back(lastLineStart);
					return;
				}

				if (!whitespace.empty() &&
						whitespace.getString()[0] == SOFT_HYPHEN)
					line.addCharNoColor(L'-');

				// break to next line
				wrappedText.push_back(line);
				if (lineStarts)
					lineStarts->push_back(lastLineStart);
				length = wordWidth;
				line = word;

				lineBounds.UpperLeftCorner.Y += lineHeight;
				lineBounds.LowerRightCorner.Y += lineHeight;
				isLastLine = lineBounds.LowerRightCorner.Y + lineHeight >
					     bounds.LowerRightCorner.Y;
				if (isLastLine)
					elWidth -= getTextWidth(L"…");
			} else {
				// add word to line
				line += whitespace;
				line += word;
				length += whitespaceWidth + wordWidth;
			}

			word.clear();
			whitespace.clear();
		}

		if (isWhitespace)
			whitespace.addChar(text, i);
	}

	if (!word.empty()) {
		if (line.empty())
			lastLineStart = lastWordStart;

		line += whitespace;
		line += word;
	}

	if (!line.empty()) {
		wrappedText.push_back(line);
		if (lineStarts)
			lineStarts->push_back(lastLineStart);
	}
}
