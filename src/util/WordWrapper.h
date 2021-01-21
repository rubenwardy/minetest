#pragma once

#include <functional>
#include <rect.h>
#include "enriched_string.h"
#include "irrlichttypes.h"

class WordWrapper
{
	const std::function<s32(const std::wstring &)> getTextWidth;

public:
	/**
	 * @param getTextWidth Function to calculate the length of a string in pixels.
	 */
	explicit WordWrapper(
			const std::function<s32(const std::wstring &)> &getTextWidth) :
			getTextWidth(getTextWidth)
	{
	}

	/**
	 * Wraps `text` to fit into `bounds`, splitting at whitespace or unicode soft
	 * hyphens.
	 *
	 * If `text` exceeds the bounds vertically, it will be truncated with an ellipsis.
	 *
	 * @param wrappedText Output, should be empty.
	 * @param text Text to wrap.
	 * @param bounds Text target bounds.
	 * @param initialLineRect Bounds of the initial line of text, relative to
	 * `bounds`.
	 */
	void wrap(std::vector<EnrichedString> &wrappedText, const EnrichedString &text,
			const core::rect<s32> &bounds,
			core::rect<s32> initialLineRect) const;
};
