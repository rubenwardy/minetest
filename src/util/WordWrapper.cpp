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

void WordWrapper::wrap(std::vector<EnrichedString> &output, std::vector<s32> *line_starts,
		const EnrichedString &text, const core::rect<s32> &bounds,
		s32 line_height, bool allow_newlines) const
{
	s32 elWidth = bounds.getWidth();

	EnrichedString line;
	EnrichedString word;
	s32 size = text.size();
	s32 line_width = 0;
	EnrichedString whitespace;

	core::rect<s32> line_bounds = bounds;
	line_bounds.LowerRightCorner.Y = line_bounds.UpperLeftCorner.Y + line_height;

	s32 last_line_start = 0;
	s32 last_word_start = 0;
	bool is_last_line = false;

	for (s32 i = 0; i < size; ++i) {
		wchar_t c = text.getString()[i];

		// New lines
		if (c == L'\r' || c == L'\n') {
			if (!allow_newlines) {
				c = ' ';
			} else {
				bool is_crlf = c == L'\r' && i < size - 1 &&
					       text.getString()[i + 1] == L'\n';
				if (is_crlf)
					i++;

				if (line.empty())
					last_line_start = last_word_start;

				line += whitespace;
				line += word;

				word.clear();
				whitespace.clear();

				if (is_last_line) {
					if (i < size - 1)
						line.addCharNoColor(L'…');
					output.push_back(line);
					if (line_starts)
						line_starts->push_back(last_line_start);
					return;
				}

				output.push_back(line);
				if (line_starts)
					line_starts->push_back(last_line_start);

				line.clear();
				line_width = 0;

				line_bounds.UpperLeftCorner.Y += line_height;
				line_bounds.LowerRightCorner.Y += line_height;
				is_last_line = line_bounds.LowerRightCorner.Y +
							       line_height >
					       bounds.LowerRightCorner.Y;
				if (is_last_line)
					elWidth -= getTextWidth(L"…");

				continue;
			}
		}

		bool is_whitespace = c == L' ' || c == 0 || c == SOFT_HYPHEN;
		if (!is_whitespace) {
			if (word.empty())
				last_word_start = i;
			word.addChar(text, i);
		}

		if (!is_whitespace && i != size - 1)
			continue;

		// Finish word
		if (!word.empty()) {
			const s32 whitespace_width = getTextWidth(whitespace.getString());
			const s32 word_width = getTextWidth(word.getString());

			if (line.empty())
				last_line_start = last_word_start;

			if (line_width > 0 && line_width + whitespace_width + word_width >
							      elWidth) {
				if (is_last_line) {
					line.addCharNoColor(L'…');
					output.push_back(line);
					if (line_starts)
						line_starts->push_back(last_line_start);
					return;
				}

				if (!whitespace.empty() &&
						whitespace.getString()[0] == SOFT_HYPHEN)
					line.addCharNoColor(L'-');

				// break to next line
				output.push_back(line);
				if (line_starts)
					line_starts->push_back(last_line_start);
				line_width = word_width;
				line = word;

				line_bounds.UpperLeftCorner.Y += line_height;
				line_bounds.LowerRightCorner.Y += line_height;
				is_last_line = line_bounds.LowerRightCorner.Y +
							       line_height >
					       bounds.LowerRightCorner.Y;
				if (is_last_line)
					elWidth -= getTextWidth(L"…");
			} else {
				// add word to line
				line += whitespace;
				line += word;
				line_width += whitespace_width + word_width;
			}

			word.clear();
			whitespace.clear();
		}

		if (is_whitespace)
			whitespace.addChar(text, i);
	}

	if (!word.empty()) {
		if (line.empty())
			last_line_start = last_word_start;

		line += whitespace;
		line += word;
	}

	if (!line.empty()) {
		output.push_back(line);
		if (line_starts)
			line_starts->push_back(last_line_start);
	}
}
