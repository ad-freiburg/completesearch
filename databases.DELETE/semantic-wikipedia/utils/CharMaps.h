// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_UTILS_CHARMAPS_H_
#define SEMANTIC_WIKIPEDIA_UTILS_CHARMAPS_H_

namespace ad_utility
{
// Map to determine the type of any char.
// The comments can be used to see which character is mapped to which class.
// 's' corresponds to spaces, F to wiki-formatting characters,
// '0' to special chars, 'w' to word chars and '['+']'+'{' + '}' to brackets.
// Here are the characters used as indices
static const char
    W_CHAR_MAP[257] =
        "sssssssssssssssssssssssssssssssss00l000F00l00000wwwwwwwwww00<F>00wwwwwwwwwwwwwwwwwwwwwwwwww[0]000wwwwwwwwwwwwwwwwwwwwwwwwww[|]00wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww"; //NOLINT

// Map to find separators. Speparators are maked as "y"
static const char
    S_CHAR_MAP[257] =
        "0000000000000000000000000000000000000000yy00y0000000000000yy0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"; //NOLINT

// Used for the excerpts. Chars marked by X are not written.
static const char
    DOCS_W_CHAR_MAP[257] =
        "sssssssssssssssssssssssssssssssss00X000X00X00000wwwwwwwwww00XXX00wwwwwwwwwwwwwwwwwwwwwwwwwwXXXX00wwwwwwwwwwwwwwwwwwwwwwwwwwXXX00wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww"; //NOLINT
}

#endif  // SEMANTIC_WIKIPEDIA_UTILS_CHARMAPS_H_
