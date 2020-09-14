// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>
#include <string>

#include "./ExcerptOffsetCollection.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::string;

namespace ad_semsearch
{
// _____________________________________________________________________________
ExcerptOffsetCollection::ExcerptOffsetCollection()
{
}
// _____________________________________________________________________________
ExcerptOffsetCollection::~ExcerptOffsetCollection()
{
}
// _____________________________________________________________________________
void ExcerptOffsetCollection::createFromDocsfile(const string& docsfileName)
{
  ad_utility::File docsFile(docsfileName.c_str(), "r");
  off_t currentOffset = 0;
  char buf[BUFFER_SIZE_DOCSFILE_LINE];
  string line;
  while (docsFile.readLine(&line, buf, BUFFER_SIZE_DOCSFILE_LINE))
  {
    Id contextId = atol(line.substr(0, line.find('\t')).c_str());
    while (_offsets.size() < contextId)
    {
      _offsets.push_back(off_t(-1));
    }
    _offsets.push_back(currentOffset);
    // One extra byte for the newline:
    currentOffset += line.size() + 1;
  }
}
// _____________________________________________________________________________
void ExcerptOffsetCollection::writeToFile(const string& outputFilename)
{
  ad_utility::File outputFile(outputFilename.c_str(), "w");
  for (size_t i = 0; i < _offsets.size(); ++i)
  {
    outputFile.write(&_offsets[i], sizeof(_offsets[i]));
  }
}
// _____________________________________________________________________________
void ExcerptOffsetCollection::readFromFile(const string& offsetsFileName)
{
  LOG(INFO) << "Reading excerpt offsets from file." << endl;

  ad_utility::File offsetsFile(offsetsFileName.c_str(), "r");
  off_t current = 0;
  _offsets.clear();
  while (!offsetsFile.isAtEof())
  {
    offsetsFile.read(&current, sizeof(off_t));
    _offsets.push_back(current);
  }

  LOG(INFO) << "Done reading excerpt offsets" << endl;
}
}
