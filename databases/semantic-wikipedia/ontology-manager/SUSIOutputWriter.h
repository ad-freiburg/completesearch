// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_SUSIOUTPUTWRITER_H_
#define SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_SUSIOUTPUTWRITER_H_

#include <string>
#include <vector>
#include <utility>
#include "./OntologyRelation.h"
#include "./OutputWriter.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"

using std::string;
using std::vector;
using std::pair;

namespace ad_semsearch
{
extern const char* WORDNET_ENTITY;

// Write two files:
// I. semantic-wikipedia.yago-paths
// II. semantic-wikipedia.yago-facts
// This writer is supposed to be deprecated soon and only
// written to maintain backwards compatibility.
// If usage should be continued, comments would have to be improved.
class SUSIOutputWriter: public OutputWriter
{
  public:
    explicit SUSIOutputWriter(const string& outputDir,
        bool pathWithSpecialCharsMode = false);
    ~SUSIOutputWriter();

    void generateOutput(vector<OntologyRelation> relations);
    typedef vector<string> Path;
    typedef pair<string, Path> PathPair;

    void setOntologyCategoryNamesMap(const string& fileName);

  private:
    string _outputDir;
    bool _pathWithSpecialCharsMode;
    ad_utility::HashMap<string, string> _ontologyCategoryNamesMap;

    string stripWordnetAndIdAccordingToSettings(const string& classString)
    {
      if (_pathWithSpecialCharsMode)
      {
        return stripWordnetAndIdLeaveUntouched(classString);
      }
      if (_ontologyCategoryNamesMap.size() > 0)
      {
        ad_utility::HashMap<string, string>::const_iterator it =
            _ontologyCategoryNamesMap.find(classString);
        if (it != _ontologyCategoryNamesMap.end())
        {
          return it->second;
        }
        else
        {
          std::cout << "!!! NO MAPPING FOUND FOR: \"" << classString << "\"!"
              << std::endl;
          return stripWordnetAndIdKeepSpecialChars(classString);
        }
      }
      // If none of the above applies, fall back to the classical
      // SUSI setup.
      return stripWordnetAndId(classString);
    }
    static string stripWordnetAndIdLeaveUntouched(const string& classString);
    static string stripWordnetAndId(const string& classString);
    static string stripWordnetAndIdKeepSpecialChars(const string& classString);

    class PathCompare
    {
      public:
        bool operator()(const Path& x, const Path& y) const;
    };

    class PathSizeCompare
    {
      public:
        bool operator()(const Path& x, const Path& y) const;
    };

    class PathPairCompare
    {
      public:
        bool operator()(const PathPair& x, const PathPair& y) const;
    };

    class FactPairCompare
    {
      public:
        bool operator()(const PathPair& x, const PathPair& y) const;
    };

    string getPathString(const Path& path) const;
    string getIdFromWN(const string& classString) const;
};
}
#endif  // SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_SUSIOUTPUTWRITER_H_
