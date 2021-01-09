// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#include "server/CustomScorer.h"
#include "server/ConcurrentLog.h"
#include "server/Vocabulary.h"

// Pointer to global CustomScorer object.
CustomScorer* globalCustomScorer = NULL;

// _____________________________________________________________________________
CustomScorer::CustomScorer(const Vocabulary& vocabulary)
 : _vocabulary(vocabulary)
{
}

// _____________________________________________________________________________
void CustomScorer::readCustomScores(const string& fileName)
{
  // Try to open file for reading.
  FILE* file = fopen(fileName.c_str(), "r");
  if (file == NULL)
  {
    cout << "! ERROR: could not open custom scores file \"" << fileName << "\""
         << endl;
  }

  // Read line by line.
  const size_t MAX_LINE_SIZE = 10000;
  char line[MAX_LINE_SIZE + 1];
  while (true)
  {
    char* ret = fgets(line, MAX_LINE_SIZE, file);
    if (feof(file)) break;
    if (ret == NULL)
    {
      cout << "! WARNING(CustomScorer): problem reading line" << endl;
      continue;
    }
    // Skip comments.
    if (line[0] == '#') continue;

    char* token = strtok(line, "\t");
    if (token == NULL)
    {
      cout << "! WARNING(CustomScorer): missing TAB 1 in custom score file"
           << endl;
      continue;
    }
    Score score = atoi(token);
    token = strtok(NULL, "\n");
    if (token == NULL)
    {
      cout << "! WARNING(CustomScorer): missing NEWLINE in custom score file"
           << endl;
      continue;
    }
    string word = token;
    // cout << "Score : " << score << "; word : \"" << word << "\"" << endl;
    size_t wordId = _vocabulary.findWord(word);
    if (wordId >= _vocabulary.size() || _vocabulary[wordId] != word)
    {
      cout << "! WARNING(CustomScorer): word \"" << word << "\" not found" << endl;
      continue;
    }
    else
    {
      cout << "* NEW(CustomScorer): adding word \"" << word
           << "\" (id = " << wordId << ") with score " << score << endl;
    }
    // TODO(bast): so far simply setting all new scores to ZERO.
    _newWordScores[wordId] = score;
  }
}
