// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_UTILS_GLOBALS_H_
#define SEMANTIC_WIKIPEDIA_UTILS_GLOBALS_H_

#include <stdint.h>
#include <string>

using std::string;

namespace ad_semsearch
{
// typedef uint64_t Id;
typedef uint32_t Id;
typedef uint8_t Score;
typedef uint16_t Position;

static const char PREFIX_CHAR = '*';

static const char ENTITY_PREFIX[] = ":e:";
static const char RELATION_PREFIX[] = ":r:";
// static const char REVERSED_RELATION_PREFIX[] = ":rr:";
// static const char CLASS_PREFIX[] = ":c:";
static const char TYPE_PREFIX[] = ":t:";
static const char VALUE_PREFIX[] = ":v:";

static const char IS_A_RELATION_FROM_YAGO[] = "is-a.entity.class";
static const char IS_A_RELATION[] = "is-a";
static const char HAS_RELATIONS_RELATION[] = "has-relations";

static const int BUFFER_SIZE_WORDNET_CATEGORIES = 512;
static const int BUFFER_SIZE_ONTOLOGY_WORD = 4092;
static const int BUFFER_SIZE_WORD = 4092;

static const int BUFFER_SIZE_WORDSFILE_LINE = 4092;
static const int BUFFER_SIZE_ONTOLOGY_LINE = 4092;
static const int BUFFER_SIZE_ASCII_LINE = 4092;

static const int STXXL_MEMORY_TO_USE = 1024 * 1024 * 1024;
static const int STXXL_DISK_SIZE_INDEX_BUILDER = 60000;
static const int STXXL_DISK_SIZE_INDEX_BUILDER_TESTS = 10;

static const char INDEX_FILE_EXTENSION[] = ".index";
static const char VOCABULARY_FILE_EXTENSION[] = ".vocabulary";
}

#endif  // SEMANTIC_WIKIPEDIA_UTILS_GLOBALS_H_
