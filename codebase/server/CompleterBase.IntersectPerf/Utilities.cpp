// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#include "./Utilities.h"
#include<string>

/*/ArrayWithSize::~ArrayWithSize()
{
  //if(array != NULL)
  //  delete array;
  //array = NULL;
}*/
// function for displaying a green-blue progress bar in the terminal
void displayProgressBar(float progressPerCent, int widthOfProgressBar)
{
  // Print Progress bar
  printf("Progress: ");

  // should be between 0 and 10, 10 means completely done...
  int progress = static_cast<int> (progressPerCent * widthOfProgressBar);
  // for the done parts draw this
  printf("\033[42m\033[37m");
  // which column the cursor is on
  int cursorColumn = 1;
  // firstdigit of the progress in %, either 0 or 1
  int firstDigitProgress = static_cast<int> (progressPerCent);
  int secondDigitProgress = static_cast<int> ((progressPerCent * 10)) %
          10;
  int thirdDigitProgress = static_cast<int> ((progressPerCent * 100)) %
          10;
  while (cursorColumn <= progress)
  {
    // in the middle of the display draw progress in percent else draw
    // colored space (green background)...
    // draw each digit individually to preserve coloring...
    if (cursorColumn == (widthOfProgressBar / 2) - 1 &&
            firstDigitProgress != 0)
      printf("%d", firstDigitProgress);
    else if (cursorColumn == (widthOfProgressBar / 2) &&
            (firstDigitProgress != 0 || secondDigitProgress != 0))
      printf("%d", secondDigitProgress);
    else if (cursorColumn == widthOfProgressBar / 2 + 1)
      printf("%d", thirdDigitProgress);
    else if (cursorColumn == widthOfProgressBar / 2 + 2)
      printf("%%");
    else
      printf(" ");
    cursorColumn++;
  }

  // draw still to do parts with blue background spaces
  printf("\033[44m");
  while (cursorColumn <= widthOfProgressBar)
  {
    // in the middle of the display draw progress in percent else draw
    // colored space (blue background)...
    // draw each digit individually to preserve coloring...
    if (cursorColumn == (widthOfProgressBar / 2) - 1 &&
            firstDigitProgress != 0)
      printf("%d", firstDigitProgress);
    else if (cursorColumn == (widthOfProgressBar / 2) &&
            (firstDigitProgress != 0 || secondDigitProgress != 0))
      printf("%d", secondDigitProgress);
    else if (cursorColumn == widthOfProgressBar / 2 + 1)
      printf("%d", thirdDigitProgress);
    else if (cursorColumn == widthOfProgressBar / 2 + 2)
      printf("%%");
    else
      printf(" ");
    cursorColumn++;
  }
  // turn off formatting
  printf("\033[0m");
}

// _____________________________________________________________________________
// Determine the type of geven query normal or artifical
// (0 normal; 1 random; 2 pseudo random)
queryTypes getQueryType(const std::string& query)
{
  if ((query.length() >= 5) && (query.c_str()[0] == '$'))
    {
      if (query.c_str()[1] == 'R')
        return RANDOMLISTS;
      if (query.c_str()[1] == 'Q')
        return PSEUDORANDOM;
    }
  return REALWORDS;
}

  // ___________________________________________________________________________
  // Devide target and baseName of a given path
  void parsePath(const std::string &path,
                 std::string *baseName,
                 std::string *target)
  {
    size_t found = path.find_last_of("/");
    if (found != std::string::npos)
    {
      *target = path.substr(0, found + 1);
      *baseName = path.substr(found + 1);
    }
    else
    {
      *baseName = path;
      *target = "";
    }
    // cout << "path " << path << endl;
    // cout << "target " << *target << endl;
    // cout << "baseName " << *baseName << endl;
  }

  // //TODO(Robin): how to make it so that array size can be unknown to caller
  // of function?
void transformQueryListToNX4Arrays(const QueryResult& list, int* array)
{
  // get the lists
  const DocList&      docIds    = list._docIds;
  const WordList&     wordIds   = list._wordIds;
  const PositionList& positions = list._positions;
  const ScoreList&    scores    = list._scores;

  // enter them sequentially into array
  size_t i = 0;
  size_t length = docIds.size();
  while (i < length)
  {
    array[4 * i] = docIds[i];
    array[4* i + 1] = wordIds[i];
    array[4 * i + 2] = positions[i];
    array[4 * i + 3] = scores[i];
    i++;
  }
}

void transformNx4ArrayIntoQueryList(int* array,
                                    int arrayLength,
                                    QueryResult* list)
{
  // Declare the necessary lists for the query result object
  DocList docList;
  WordList wordList;

  Vector <Position> positionList;
  Vector <Score> scoreList;
  for (size_t i = 0; i < arrayLength; i+= 4)
  {
    DocId docId;
    WordId wordId;
    Position position;
    Score score;

    docId = array[i];
    wordId = array[i + 1];
    position = array[i + 2];
    score = array[i + 3];

    docList.push_back(docId);
    wordList.push_back(wordId);
    positionList.push_back(position);
    scoreList.push_back(score);
  }
  list->setMatchingWordsWithDups(wordList);
  list->setMatchingDocsWithDups(docList);
  list->setScorelist(scoreList);
  list->setPositionlist(positionList);
}
