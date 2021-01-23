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
	bool is_last_line = false;

	for (s32 i = 0; i < size; ++i) {
		wchar_t c = text.getString()[i];

		// New lines
		bool is_newline = false;
		if (c == L'\r' || c == L'\n') {
			if (allow_newlines)
				is_newline = true;
			else
				c = L' ';
		}

		bool is_whitespace = c == L' ' || c == 0 || c == SOFT_HYPHEN;
		if (!is_whitespace && !is_newline)
			word.addChar(text, i);

		if (!is_whitespace && !is_newline && i != size - 1)
			continue;

		// Finish word
		if (!word.empty()) {
			const s32 whitespace_width = getTextWidth(whitespace.getString());
			const s32 word_width = getTextWidth(word.getString());

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
					whitespace = EnrichedString(L"-") +
						     whitespace.substr(1,
								     whitespace.size() -
										     1);

				// break to next line
				line += whitespace;
				output.push_back(line);
				if (line_starts)
					line_starts->push_back(last_line_start);

				if (is_whitespace)
					last_line_start = i - (s32)word.size();
				else
					last_line_start = i - (s32)word.size() + 1;

				line_width = word_width;
				line = word;

				word.clear();
				whitespace.clear();

				line_bounds.UpperLeftCorner.Y += line_height;
				line_bounds.LowerRightCorner.Y += line_height;
				is_last_line = line_bounds.LowerRightCorner.Y +
							       line_height >
					       bounds.LowerRightCorner.Y;
				if (is_last_line)
					elWidth -= getTextWidth(L"…");

				is_newline = false;
			} else if (!is_newline) {
				// add word to line
				line += whitespace;
				line += word;
				line_width += whitespace_width + word_width;

				word.clear();
				whitespace.clear();
			}
		}

		if (is_newline) {
			if (c == L'\r' && i < size - 1 &&
				text.getString()[i + 1] == L'\n')
				i++;

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
			last_line_start = i + 1;

			line.clear();
			line_width = 0;

			line_bounds.UpperLeftCorner.Y += line_height;
			line_bounds.LowerRightCorner.Y += line_height;
			is_last_line = line_bounds.LowerRightCorner.Y +
				line_height >
				bounds.LowerRightCorner.Y;
			if (is_last_line)
				elWidth -= getTextWidth(L"…");
		} else if (is_whitespace) {
			whitespace.addChar(text, i);
		}
	}

	if (!whitespace.empty() || !word.empty()) {
		line += whitespace;
		line += word;
	}

	if (!line.empty()) {
		output.push_back(line);
		if (line_starts)
			line_starts->push_back(last_line_start);
	}
}
