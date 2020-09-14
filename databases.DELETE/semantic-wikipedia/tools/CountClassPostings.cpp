// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <string>
#include <sstream>
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
using std::string;

int main(int argc, char** argv)
{
  ad_utility::HashMap<string, int> classesPerEntity;
  ad_utility::HashSet<string> personEntities;
  ad_utility::HashSet<string> substanceEntities;
  ad_utility::File isAStuff("entity-class-relation.ascii", "r");
  char buf[5000];
  string line;
  while (isAStuff.readLine(&line, buf, 5000))
  {
    size_t indexOftab = line.find('\t');
    ad_utility::HashMap<string, int>::const_iterator it =
        classesPerEntity.find(line.substr(0, indexOftab));
    if (it == classesPerEntity.end())
    {
      classesPerEntity[line.substr(0, indexOftab)] = 1;
    }
    else
    {
      ++classesPerEntity[line.substr(0, indexOftab)];
    }
    if (line.substr(indexOftab + 1) == ":e:person:Person")
    {
      personEntities.insert(line.substr(0, indexOftab));
    }
    if (line.substr(indexOftab + 1) == ":e:substance:Substance")
    {
      substanceEntities.insert(line.substr(0, indexOftab));
    }
  }
  ad_utility::File wordsFile(
      "data/semantic-wikipedia-full-sep11-full.words-by-contexts.ascii", "r");
  size_t count = 0;
  size_t personcount = 0;
  size_t substancecount = 0;
  while (wordsFile.readLine(&line, buf, 5000))
  {
    string word = line.substr(0, line.find('\t'));
    if (ad_utility::startsWith(line, ":e:"))
    {
      ad_utility::HashMap<string, int>::const_iterator it =
          classesPerEntity.find(word);
      if (it != classesPerEntity.end())
      {
        count += it->second;
      }
      else
      {
        ++count;
      }
      if (personEntities.count(word) > 0) ++personcount;
      if (substanceEntities.count(word) > 0) ++substancecount;
    }
  }
  std::cout << "Total classes count:" << count << std::endl;
  std::cout << "Person list:" << personcount << std::endl;
  std::cout << "Substance list:" << substancecount << std::endl;
  std::cout << "Total classes count" << count << std::endl;
}
