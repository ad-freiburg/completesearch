// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <fstream>
#include "./OutputWriter.h"
#include "../codebase/semantic-wikipedia-utils/File.h"

namespace ad_semsearch
{
// _____________________________________________________________________________
void OutputWriter::setOntologyNamesMapping(const string& fileName)
{
  std::cout << "Reading the map from file " << fileName
      << "...\t" << std::flush;
  ad_utility::File map(fileName.c_str(), "r");
  string line;
  char buf[1024];
  _nameMap.clear();
  while (map.readLine(&line, buf, 1024))
  {
    size_t indexOfTab = line.find('\t');
    assert(indexOfTab != string::npos);
    _nameMap[line.substr(0, indexOfTab)] = line.substr(indexOfTab + 1);
  }
  std::cout << "Read " << _nameMap.size() << " pairs."
        << std::endl << std::flush;
}
// ____________________________________________________________________________
void OutputWriter::readStringStringMap(
    const std::string& mapFileName,
    ad_utility::HashMap<string, string>* targetMap)
{
  std::cout << "Reading the map from file " << mapFileName
      << "...\t" << std::flush;

  // Read the title - entity mapping.
  std::ifstream fs(mapFileName.c_str(), std::ios::in);
  // Read line-wise.
  std::string line;
  while (std::getline(fs, line))
  {
    // Separate titles and entity by tabs.
    std::string::size_type indextofTab = line.find('\t');
    // assert(indextofTab > 0 && indextofTab != line.npos);
    std::string key = line.substr(0, indextofTab);
    std::string value = line.substr(indextofTab + 1);
    targetMap->operator[](key) = value;
  }
  fs.close();
  std::cout << "Read " << targetMap->size() << " pairs."
      << std::endl << std::flush;
}
}
