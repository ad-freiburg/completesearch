// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_EXCERPTOFFSETCOLLECTION_H_
#define SEMANTIC_WIKIPEDIA_SERVER_EXCERPTOFFSETCOLLECTION_H_

#include <vector>
#include <string>
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
//! Hack-ish class to provide early access to excerpts
class ExcerptOffsetCollection
{
  public:
    ExcerptOffsetCollection();
    virtual ~ExcerptOffsetCollection();

    void createFromDocsfile(const string& docsfile);
    void writeToFile(const string& outputFile);
    void readFromFile(const string& offsetsFile);

    off_t getExcerptOffsetForContextId(const Id contextId) const
    {
      if (contextId >= _offsets.size())
      {
        AD_THROW(Exception::CHECK_FAILED, "Access with invalid context Id.");
      }
      return _offsets[contextId];
    }

  private:
    vector<off_t> _offsets;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_EXCERPTOFFSETCOLLECTION_H_
