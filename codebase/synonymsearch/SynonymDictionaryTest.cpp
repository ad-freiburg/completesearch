// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright Jens Hoffmann 2010

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include "./SynonymDictionary.h"

// Test fixture.
class SynonymDictionaryTest : public ::testing::Test
{
	protected:
		virtual void SetUp()
		{
			// Define 3 records with synonyms.
  		char synGroup1[] = "VW Volkswagen *Beatle *Kaefer Passat \n";
  		char synGroup2[] = "Professor Lehrer Dozent Schulmeister Direktor Pedell "
  		                    "Studienrat \n";
  		char synGroup3[] = "Kaefer Marienkaefer Maikaefer Mistkaefer "
  		                   "Kartoffelkaefer \n";

  		// Create unique file as temporary file.
  		FILE* file;
			char tmpFileName[] = "XXXXXX";
  		if (mkstemp(tmpFileName) < 0)
			{
				perror("mkstemp failed.");
				exit(errno);
			}
			strcpy(fileName, tmpFileName);

  		file = fopen(tmpFileName, "w");

  		assert(file != NULL);

  		// Write 3 records to the filestream.
  		fputs(synGroup1, file);
  		fputs(synGroup2, file);
  		fputs(synGroup3, file);

  		// Close the filestream.
  		fclose(file);
		}

		virtual void TearDown()
		{
			// Destroy temporary file tmpFileName.
		  remove(fileName);
		}
		char fileName [1024];
};

// _____________________________________________________________________________
TEST_F(SynonymDictionaryTest, readFromFile_test1)
{

  // Create an instance of the class SynonymDictionary.
  SynonymDictionary sd;

  // Now let the object sd read the synonym groups inside the temporary file.
  sd.readFromFile(fileName);
  // Get the vector that holds all hash values related to the synonym group the
  // word belongs to.
  vector<uint32_t> synGroupIds;
  sd.getSynonymGroupIds("Kaefer", synGroupIds);
  ASSERT_EQ((unsigned) 2, synGroupIds.size());

  // 'Kaefer' is in group 2 with the 31. bit set (marked with '*').
  ASSERT_EQ(synGroupIds[0], 0x80000002);
  // 'Kaefer' is in group 6.
  ASSERT_EQ((unsigned) 6, synGroupIds[1]);

  // Check iff getSynonymGroups returns NULL iff we search for a word that is
  // not mapped.

  synGroupIds.resize(0);

  sd.getSynonymGroupIds("BlaBliBlub", synGroupIds);
  ASSERT_EQ(synGroupIds.empty(), true);
}

// _____________________________________________________________________________
TEST_F(SynonymDictionaryTest, getSynonymGroupIds)
{
	SynonymDictionary sd;
	vector<uint32_t> result;
	sd.readFromFile(fileName);
	sd.getSynonymGroupIds("VW", result);
	ASSERT_EQ((unsigned) 1, result.size());
	ASSERT_TRUE(!sd.stripAsteriskBitFromGroupId(&result[0]));
	sd.getSynonymGroupIds("Volkswagen", result);
	ASSERT_EQ((unsigned) 1, result.size());
	ASSERT_TRUE(!sd.stripAsteriskBitFromGroupId(&result[0]));
	sd.getSynonymGroupIds("Beatle", result);
	ASSERT_EQ((unsigned) 1, result.size());
	ASSERT_TRUE(sd.stripAsteriskBitFromGroupId(&result[0]));
	sd.getSynonymGroupIds("Kaefer", result);
	ASSERT_EQ((unsigned) 2, result.size());
	ASSERT_TRUE(sd.stripAsteriskBitFromGroupId(&result[0]));
	sd.getSynonymGroupIds("Passat", result);
	ASSERT_EQ((unsigned) 1, result.size());
	ASSERT_TRUE(!sd.stripAsteriskBitFromGroupId(&result[0]));
}

// _____________________________________________________________________________
TEST_F(SynonymDictionaryTest, readFromFile_test2)
{
  int line = 10000;  // Number of lines in testfile
  int column;
  int wLength;
  int character;
  unsigned int seed = 0;

  // Creating testfile.
  FILE* testFile = fopen("synonyme.dic", "w+");

  assert(testFile != NULL);

  // Write two words at the begin of file.
  fprintf(testFile, "%s", "Bohne\n");
  fprintf(testFile, "%s", "Pest\n");

  // This shall be a shortened ASCII table. When words are created with only
  // this set, it's a little probable that words come twice and more often.
  char randomizer[] = "qwertzuiop";

  // Init random function.
  srand(time(NULL));

  // Write a lot of random data.
  for (int i = 0; i < line; i++)
  {
    // Select one column between 0..99.
    column = rand_r(&seed) % 100;

    for (int j = 0; j < column; j++)
    {
      // Select word length between 3..10.
      wLength = (rand_r(&seed) % 8) + 3;


      // Write random word of length wLength in testfile.
      for (int k = 0; k < wLength; k++)
      {
        // Select random character out of shortened ASCII table (see above).
        character = rand_r(&seed) % 10;

        assert(character >= 0 && character <= 9);

        fprintf(testFile, "%c", randomizer[character]);
      }

      // Space after the word.
      fprintf(testFile, " ");
    }

    // New line in file.
    fprintf(testFile, "\n");
  }

  // Close test file.
  fclose(testFile);

  // Create an instance of the class SynonymDictionary.
  SynonymDictionary sd;

  // Now read created testFile
  sd.readFromFile("synonyme.dic");

  vector<uint32_t> synGroupIds;

  // Check iff the word 'bohne' still belongs to the groups 2.
  sd.getSynonymGroupIds("Bohne", synGroupIds);

  synGroupIds.clear();

  // Check iff the word 'pest' still belongs to the groups 4.
  sd.getSynonymGroupIds("Pest", synGroupIds);
  ASSERT_EQ((unsigned) 4, synGroupIds[0]);
}

// _____________________________________________________________________________
TEST_F(SynonymDictionaryTest, stripAsteriskBitFromGroupId)
{
  uint32_t groupId = 0;
  SynonymDictionary dictionary;
  bool result;

  // Asterisk bit set.
  groupId =  groupId | 0x80000003;
  result = dictionary.stripAsteriskBitFromGroupId(&groupId);
  ASSERT_EQ(true, result);
  ASSERT_EQ((unsigned) 3, groupId);

  // Asterisk bit not set.
  result = dictionary.stripAsteriskBitFromGroupId(&groupId);
  ASSERT_FALSE(result);
  ASSERT_EQ((unsigned) 3, groupId);
}

