// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Authors: Jens Hoffmann <hoffmaje>, Hannah Bast <bast>.

#include <errno.h>
#include <time.h>
#include <iomanip>
#include <utility>
#include <vector>
#include <sstream>
#include <string>
#include "./CsvParser.h"

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

// _____________________________________________________________________________
CsvParser::CsvParser() : ParserBase()
{
}

// _____________________________________________________________________________
void CsvParser::parseCommandLineOptions(int argc, char** argv)
{
  ParserBase::parseCommandLineOptions(argc, argv);
  if (ParserBase::_fileNameBase == "")
  {
    cerr << "! No base name specified, use --base-name" << endl << endl;
    printUsage();
    cout << endl;
    exit(1);
  }
  _options.parseFromCommandLine(argc, argv,
      ParserBase::_fileNameBase + ParserBase::_csvFileNameSuffix);
  _fieldOptions = _options.getFieldOptions();
  _stringConversion.setEncoding(_encoding == ParserBase::UTF8
                                ? StringConversion::UTF_8
                                : StringConversion::ISO_8859_1);
}


// _____________________________________________________________________________
void CsvParser::writeConcatToWordsFile(const string& field,
                                       const string& prefix,
                                       const string& suffix,
                                       unsigned docID, unsigned score)
{
  size_t wordStart = 0;
  size_t wordEnd = 0;
  string word;
  string wordToIndex;
  string wordToLower;

  StringConverter::Encoding encoding = StringConverter::ENCODING_ISO8859_1;
  if (_encoding == ParserBase::UTF8)
  {
    encoding = StringConverter::ENCODING_UTF8;
  }

  // The simplest way to add UTF8-support to SimpleTextParser seems to
  // me to leave SimpleTextParser unchanged and to replace all
  // UTF8-delimiter characters by an ASCII delimiter character before
  // using SimpleTextParser::parseText().
  //
  // If the string is UTF8-encoded, we have to keep in mind that the
  // replaced character may consist of more than one byte. Therefore the
  // resulting string can have fewer bytes than the input string.
  // We can avoid this by calling replaceNonAlphanum() with the
  // bKeepByteNumber parameter set to true.
  // We do this in this case because extendToUserDefinedWord() needs to
  // work on the original string and the parser has to deliver wordStart
  // with respect to the original string and not with respect to the
  // replacement string.
  //
  // TODO(bartsch) look for another way to init vnSaveCodepoints
  // (perhaps this should be controllable by a parser option because
  // different application may wish to remain different characters
  // (perhaps even in different fields)???
  // The codepoints of SPACE and LOW LINE are equal for ISO and UTF8
  // so we can use the same vnSaveCodepoints vector for both cases:
  vector<size_t> vnSaveCodepoints;
  vnSaveCodepoints.push_back(95);  // save LOW LINE _
  string fieldCopy;
  fieldCopy = _stringConverter.replaceNonAlphanum(field, encoding, ' ', true,
                                                  vnSaveCodepoints);

  // As long there are substrings in the string field.
  while (wordEnd < fieldCopy.size())
  {
    // Get the next substring.
    _simpleTextParser.parseText(fieldCopy, &wordStart, &wordEnd);

    // Extend word to possible user defined word, iff _mode in ParserBase is set
    // accordingly.
    if (_readUserDefinedWords)
    {
      // We have to use field here instead of fieldCopy to find the
      // user defined words because in the UTF8 case special characters
      // may have been repaced in fieldCopy and the user defined word
      // will not match.
      ParserBase::extendToUserDefinedWord(field, &wordStart, &wordEnd);
    }

    if (wordStart == wordEnd) break;

    // We normalize or lowercase only the word, not prefixes and suffixes
    // NOTE(Hannah, 26-01-2017): it said so but actually did include the suffix
    // here, now remove it here and add it below, so that it indeed does not get
    // normalized or lowercased.
    word = field.substr(wordStart, wordEnd - wordStart);
    // word = field.substr(wordStart, wordEnd - wordStart) + suffix;
    wordToLower = _stringConverter.convert(word, encoding,
                      StringConverter::CONVERSION_TO_LOWER);

    // We don't want empty words to be stored.
    if (wordToLower.empty()) continue;

    if (_normalizeWords)
    {
      // Normalization includes lowercasing
      string wordNorm = _stringConverter.convert(word, encoding,
                            StringConverter::CONVERSION_TO_NORM);
      // To save the orginal word, add e.g. :filter:field:qury:query AND
      // :filter:field:query for mapping issues.
      if (wordNorm != wordToLower)
      {
        wordToIndex = prefix + wordNorm + _wordPartSep + wordToLower;
        ParserBase::writeToWordsFile(wordToIndex, docID, score, _position);
      }
    }

    // NOTE(Hannah, 26-01-2017): see comment above.
    wordToIndex = prefix + wordToLower + suffix;
    // wordToIndex = prefix + wordToLower;

    // And write the word to <basename>.words file.
    ParserBase::writeToWordsFile(wordToIndex, docID, score, _position);
    ++_position;
  }
}


/*
// _____________________________________________________________________________
void CsvParser::writeConcatToWordsFile(const string& field,
                                       const string& prefix,
                                       const string& suffix,
                                       unsigned docID, unsigned score)
{
  size_t wordStart = 0;
  size_t wordEnd = 0;
  string wordToIndex;

  // As long there are substrings in the string field.
  while (wordEnd < field.size())
  {
    // Get the next substring.
    _simpleTextParser.parseText(field, &wordStart, &wordEnd);
    // Extend word to possible user defined word, iff _mode in ParserBase is set
    // accordingly.
    if (_readUserDefinedWords)
    {
      ParserBase::extendToUserDefinedWord(field, &wordStart, &wordEnd);
    }

    if (wordStart == wordEnd) break;
    wordToIndex = prefix + field.substr(wordStart, wordEnd - wordStart)
                  + suffix;
    _stringConversion.toLower(&wordToIndex);
    // cout << "Index word: \""
    //      << wordToIndex << "\" [" << field << "]" << endl;
    // And write the word to <basename>.words file.
    ParserBase::writeToWordsFile(wordToIndex, docID, score, _position);
    ++_position;
  }
}
*/

// _____________________________________________________________________________
void CsvParser::writeFulltextToWordsFile(const string& field,
                                         unsigned docID,
                                         unsigned score)
{
  writeConcatToWordsFile(field, string(""), string(""), docID, score);
}

// _____________________________________________________________________________
void CsvParser::writePhraseCompletionToWordsFile(const string& field,
                                                 unsigned docID,
                                                 unsigned score)
{
  // Create a phrase entry only if it's a real phrase.
  // That means: for 'costa rica' we create: costa:costa_rica and
  // rica:costa_rica but for 'vietnam' we do not create: vietnam:vietnam
  //
  string field2 = _wordPartSep + field;
  bool bRealPhrase = false;

  for (size_t n = 0; n < field2.length(); n++)
  {
    if (field2[n] == ' ')
    {
      field2[n] = '_';
      bRealPhrase = true;
    }
  }
  if (bRealPhrase == false)
  {
    field2 = "";
  }

  // Write to .words file.
  writeConcatToWordsFile(field, string(""), field2, docID, score);
}


// _____________________________________________________________________________
void CsvParser::writeFilterToWordsFile(const string& field,
                                       const string& fieldName,
                                       unsigned docID,
                                       unsigned score,
                                       bool appendFieldContents)
{
  // NEW (baumgari) 18Apr13: We don't need to write empty filter words into the
  // words and vocabulary file.
  if (field.empty()) return;

  string field2;
  if (appendFieldContents)
  {
    // NEW(bast, 02-02-2017): always append the field name, even if it consists
    // just of a single word. This is not redundant, because of variants in the
    // capitalization in the field name (e.g. bok, Bok, BOK).
    // bool bRealPhrase = false;
    field2 = _wordPartSep + field;

    // Replace any ' ' by '_' in field2.
    for (size_t n = 0; n < field2.length(); n++)
    {
      if (field2[n] == ' ')
      {
        field2[n] = '_';
        // bRealPhrase = true;
      }
    }
    // if (bRealPhrase == false) { field2 = ""; }
  }


  string prefix = _wordPartSep + string("filter") + _wordPartSep
                                + fieldName + _wordPartSep;
  writeConcatToWordsFile(field, prefix, field2, docID, score);
}


void CsvParser::removeDelimChars(string* word)
{
  assert(word);
  string delims = _simpleTextParser.getSeparators();
  for (unsigned i = 0; i < word->length(); ++i)
  {
    for (unsigned j = 0; j < delims.length(); ++j)
    {
      if (word->operator[](i) == delims[j] &&
          word->operator[](i) != ' ' &&
          word->operator[](i) != '_')
        word->erase(i, 1);
    }
  }
}

// _____________________________________________________________________________
void CsvParser::writeFacetToWordsFile(const string& facetWord,
                                      const string& field,
                                      const string& fieldName,
                                      unsigned docID,
                                      unsigned score)
{
  string prefix = _wordPartSep + facetWord + _wordPartSep + fieldName
                     + _wordPartSep;
  string suffixOrg = field;
  size_t pos;
  while ((pos = suffixOrg.find(' ')) != string::npos) suffixOrg[pos] = '_';
  // NEW (baumgari) 18Apr13: We don't need to write empty facets into the words
  // and vocabulary file.
  if (suffixOrg.empty()) return;

  //
  // Start old code:
  // Makes
  // :facet:Nation:St_Lucia:St._Lucia
  // out of
  // "St. Lucia"
  //
  // string suffixNorm = suffixOrg;
  // removeDelimChars(&suffixNorm);
  // ParserBase::writeToWordsFile(prefix +
  //                              suffixNorm +
  //                              _wordPartSep +
  //                              suffixOrg, docID, score, _position);
  // End old code:
  //

  // Start new code:
  // Makes
  // :facet:Nation:St%2e_Lucia
  // out of
  // "St. Lucia"
  // instead of
  // :facet:Nation:St_Lucia:St._Lucia
  //
  /*string suffixNorm =  _stringConverter.urlEncode(suffixOrg);

  string sToWrite = prefix + suffixNorm;
  ParserBase::writeToWordsFile(sToWrite, docID, score, _position);
  */

  // Start very new code:
  // NEW baumgari 21Feb14: Why do we encode the characters? It's not necessary
  // at all since all query strings are decoded on the server. Thus, if we write
  // encoded words to the index, they cannot be found. Just don't encode.
  string sToWrite = prefix + suffixOrg;
  ParserBase::writeToWordsFile(sToWrite, docID, score, _position);

  ++_position;
}

// _____________________________________________________________________________
void CsvParser::writeFacetInOldWordsFormatToWordsFile(const string& field,
                                                      const string& fieldName,
                                                      unsigned docID,
                                                      unsigned score)
{
  string suffixOrg = field;
  size_t pos;
  while ((pos = suffixOrg.find(' ')) != string::npos) suffixOrg[pos] = '_';
  // NEW (baumgari) 18Apr13: We don't need to write empty facets into the words
  // and vocabulary file.
  if (suffixOrg.empty()) return;

  string suffixNorm =  _stringConverter.urlEncode(suffixOrg);
  StringConverter::Encoding encoding = StringConverter::ENCODING_ISO8859_1;
  if (_encoding == ParserBase::UTF8)
  {
    encoding = StringConverter::ENCODING_UTF8;
  }

  string suffixLower = suffixNorm;
  if (_normalizeWords)
  {
    // Normalization includes lowercasing
    suffixLower = _stringConverter.convert(suffixLower,
                                           encoding,
                                           StringConverter::CONVERSION_TO_NORM);
  }
  else
  {
    // Only lowercasing:
    suffixLower = _stringConverter.convert(suffixLower,
                                          encoding,
                                          StringConverter::CONVERSION_TO_LOWER);
  }
  suffixNorm = suffixLower + _wordPartSep + suffixNorm;
  string prefix = string("ct") + _wordPartSep + fieldName + _wordPartSep;
  string sToWrite = prefix + suffixNorm;
  ParserBase::writeToWordsFile(sToWrite, docID, score, _position);
  prefix = string("ce") + _wordPartSep + fieldName + _wordPartSep;
  sToWrite = prefix + suffixNorm;
  ParserBase::writeToWordsFile(sToWrite, docID, score, _position);
  prefix = string("cn") + _wordPartSep + fieldName + _wordPartSep;
  sToWrite = prefix + suffixNorm;
  ParserBase::writeToWordsFile(sToWrite, docID, score, _position);
  ++_position;
}


// _____________________________________________________________________________
void CsvParser::writeOrderingDateToWordsFile(const string& field,
                                             const string& fieldName,
                                             unsigned docID,
                                             unsigned score)
{
  string prefix = _wordPartSep + string("ordering") + _wordPartSep
                                + fieldName + _wordPartSep;
  string date;
  date.resize(8);

  string fieldConverted = _stringConverter.convertDate(field);

  // Check the dates format.
  if (fieldConverted.size() != 10 ||
      fieldConverted[2] != '-' ||
      fieldConverted[5] != '-')
  {
    fprintf(stderr, "Date has wrong format. DocID %u.\n", docID);
    fprintf(stderr, "Format of date is: DD-MM-YYYY\n");
    fprintf(stderr, "Output format is:  YYYYMMDD\n");
    exit(1);
  }
  // Map given date to csearch format.
  date[0] = fieldConverted[6];
  date[1] = fieldConverted[7];
  date[2] = fieldConverted[8];
  date[3] = fieldConverted[9];
  date[4] = fieldConverted[3];
  date[5] = fieldConverted[4];
  date[6] = fieldConverted[0];
  date[7] = fieldConverted[1];

  // Flush date.
  ParserBase::writeToWordsFile(prefix + date, docID, score, _position);
  ++_position;
}


// _____________________________________________________________________________
void CsvParser::writeOrderingLiteralToWordsFile(const string& field,
    const string& fieldName,
    unsigned docID,
    unsigned score)
{
  // NEW (baumgari) 18Apr13: We don't need to write empty ordering words into the
  // words and vocabulary file.
  if (field.empty()) return;

  string prefix =   _wordPartSep + string("ordering")
                  + _wordPartSep + fieldName
                  + _wordPartSep + field;

  ParserBase::writeToWordsFile(prefix, docID, score, _position);
  ++_position;
}


// _____________________________________________________________________________
void CsvParser::writeOrderingPrecisionToWordsFile(const string& field,
    const string& fieldName,
    unsigned docID,
    unsigned score,
    const CsvField::Precision& precision)
{
  // NEW (baumgari) 18Apr13: We don't need to write empty ordering words into the
  // words and vocabulary file.
  if (field.empty()) return;

  string number, prefix;
  size_t pos;

  // Init variables.
  prefix = _wordPartSep + string("ordering") + _wordPartSep
                         + fieldName + _wordPartSep;
  number.resize(precision.first + precision.second, '0');


  // To allow things like "12,-" or "12,99 Euro" etc.
  string fieldCopy;
  for (size_t n = 0; n < field.length(); n++)
  {
    if (isdigit(field[n]) == true ||
        field[n] == ',' ||
        field[n] == '.')
    {
      fieldCopy += field[n];
    }
  }

  pos = fieldCopy.find('.');
  if (pos == string::npos)
  {
    pos = fieldCopy.find(',');
  }
  if (pos == string::npos)
  {
    pos = fieldCopy.size();
  }

  // pos now gives the number of digits before the commata.
  if (pos > precision.first)
  {
    fprintf(stderr, "Too many leading digits: docID %u.\n", docID);
    exit(1);
  }
  if (pos == string::npos)
  {
    fprintf(stderr, "Wrong format of number: docID %u.\n", docID);
    exit(1);
  }

  assert(precision.first >= pos);
  size_t numLeadingZeroes = (size_t)(precision.first - pos);
  for (size_t i = 0; i < pos; ++i)
  {
    number[numLeadingZeroes + i] = fieldCopy[i];
  }
  for (size_t i = 0; pos + 1 + i < fieldCopy.size(); ++i)
  {
    number[numLeadingZeroes + pos + i] = fieldCopy[pos + 1 + i];
  }
  ParserBase::writeToWordsFile(prefix + number, docID, score, _position);
  ++_position;
}


// _____________________________________________________________________________
void CsvParser::writeFieldItemToWordsFile(unsigned docID, unsigned colID,
                                          string* fieldItem)
{
  const CsvField& csvField = _fieldOptions[colID];
  bool facetAllowed = true;
  // In case this is a facet string and it starts by the no-show-facets char,
  // erase this char and stop it from being added as facet.
  if (csvField.getFacet() && fieldItem->size() > 0
      && fieldItem->at(0) == _options.getNoShowPrefix())
  {
    fieldItem = new string(fieldItem->substr(1));
    facetAllowed = false;
  }

  // Option fulltext set?
  if (csvField.getFulltext())
  {
    writeFulltextToWordsFile(*fieldItem,
                             docID,
                             csvField.getScore());
  }

  // Option phrase completion set?
  if (csvField.getPhraseCompletion())
  {
    writePhraseCompletionToWordsFile(*fieldItem,
                                     docID,
                                     csvField.getScore());
  }

  // NOTE(bast, 19-03-2017): now two separate filters, one which does not add
  // the field contents (filter), and one which does (filterPlus) . In an older
  // version of the code, this was controlled via the --phrase argument, which
  // also adds words like <word>:<field-content>. The two feature are now
  // independent, as they should have been in the first place.
  // Also note that the filterPlus words can be a problem for applications that
  // do not need them, since they can blow up the vocabulary enormously (for
  // example, think of :filter:title:<word-from-title>:<whole-title>). That's
  // why there is a choice between filter and filterPlus.

  // Option filter set?
  if (csvField.getFilter())
  {
    writeFilterToWordsFile(*fieldItem,
                           csvField.getName(),
                           docID,
                           csvField.getScore(),
                           false);
                           // csvField.getPhraseCompletion());
  }

  // Option filterPlus set?
  if (csvField.getFilterPlus())
  {
    writeFilterToWordsFile(*fieldItem,
                           csvField.getName(),
                           docID,
                           csvField.getScore(),
                           true);
  }

  // Option facet set?
  if (csvField.getFacet() && facetAllowed)
  {
    // NEW 17May12 (Ina): Added functionality for old words format (ct:, cn:
    // , ce:), since the new format is right now not working with autocomplete.
    // So instead of writing :facet:* to the words file, write ct:*, etc. to it.
    if (!_options.isOldWordsFormat())
    {
      writeFacetToWordsFile("facet",
                            *fieldItem,
                            csvField.getName(),
                            docID,
                            csvField.getScore());
    }
    else
    {
      writeFacetInOldWordsFormatToWordsFile(*fieldItem,
                                            csvField.getName(),
                                            docID,
                                            csvField.getScore());
    }
  }

  // Option facetIds set?
  if (csvField.getFacetId() && facetAllowed)
  {
    if (!_options.isOldWordsFormat())
    {
      writeFacetToWordsFile("facetid",
                            *fieldItem,
                            csvField.getName(),
                            docID,
                            csvField.getScore());
    }
  }

  // Option ordering set?
  if (csvField.getOrderingDate())
  {
    writeOrderingDateToWordsFile(*fieldItem,
                                 csvField.getName(),
                                 docID,
                                 csvField.getScore());
  }

  // Option ordering literal set?
  if (csvField.getOrderingLiteral())
  {
    writeOrderingLiteralToWordsFile(*fieldItem,
                                    csvField.getName(),
                                    docID,
                                    csvField.getScore());
  }

  // Option ordering precission set?
  if (csvField.getOrderingPrecision())
  {
    writeOrderingPrecisionToWordsFile(*fieldItem,
                                       csvField.getName(),
                                       docID,
                                       csvField.getScore(),
                                       csvField.getPrecision());
  }
}


// _____________________________________________________________________________
void CsvParser::writeFieldsToWordsAndDocsFile(unsigned docID,
                                              vector<FieldItem>* fields)
{
  _position = 1;
  // Add items to words file.
  for (unsigned i = 0; i < fields->size(); ++i)
    writeFieldItemToWordsFile(docID,
                              fields->operator[](i).fieldIndex,
                              &fields->operator[](i).fieldContent);
  // Add items to docs file.
  writeFieldsToDocsFile(docID, fields);
}
// _____________________________________________________________________________
void CsvParser::writeFieldsToDocsFile(unsigned docID, vector<FieldItem>* fields)
{
  if (ParserBase::_writeDocsFile)
  {
    assert(_docs_file);
    assert(fields);
    string toShow;
    string excerpt;
    // NEW 10Jan13 (baumgari): We can now define multiple titles, whereas
    // showList defines the matrix, which stores which field should be printed
    // in which title.
    const vector<pair<FormatEnum, vector<bool> > >& showLists
      = CsvField::getStaticShowList();
    // There might be some multiple fields with no show items.
    // No show items should be erased (or replaced by an empty field, if there
    // is no other item representing the field - necessary for json output).
    vector<FieldItem> cleanedFields;
    cleanedFields.reserve(fields->size());
    for (size_t i = 0; i < fields->size(); i++)
    {
      string& field = (*fields)[i].fieldContent;
      const size_t& fieldIndex = (*fields)[i].fieldIndex;
      if (!field.empty() && field[0] == _options.getNoShowPrefix())
      {
        // There is already an item representing the field or the next one might
        // be one.
        if ((!cleanedFields.empty()
              && cleanedFields.back().fieldIndex == fieldIndex)
            || ((i + 1 < fields->size())
              && (*fields)[i + 1].fieldIndex == fieldIndex))
          continue;
        // Otherwhise add the field with empty content.
        cleanedFields.push_back((*fields)[i]);
        cleanedFields.back().fieldContent.clear();
      }
      else
      {
        cleanedFields.push_back((*fields)[i]);
      }
    }

    // Generate show field.
    // For each title ...
    for (size_t i = 0; i < showLists.size(); i++)
    {
      FormatEnum outputFormat = showLists[i].first;
      if (outputFormat == JSON) toShow.append("{");
      const CsvField* lastField = NULL;
      // For each field ...
      for (size_t j = 0; j < cleanedFields.size(); j++)
      {
        string& field = cleanedFields[j].fieldContent;
        const size_t fieldIndex = cleanedFields[j].fieldIndex;
        const CsvField* csvField = &_fieldOptions[fieldIndex];

        // Replace all info-delimiter (if defined) within the field by a space.
        for (size_t k = 0; k < field.size() && showLists.size() > 1; k++)
        {
          if (field[k] == _infoDelim) field[k] = ' ';
        }

        // Append to toShow if it should be printed in this title.
        if (showLists[i].second[fieldIndex])
        {
          // Write fields as XML.
          if (outputFormat == XML && !field.empty())
          {
            toShow.append("<" + csvField->getName() + ">");

            if (csvField->getFormat() == TEXT)
              toShow.append(escapeXml(field));
            else if (csvField->getFormat() == XML)
              toShow.append(field);
            else if (csvField->getFormat() == JSON)
              toShow.append(escapeXml(field));

            toShow.append("</" + csvField->getName() + ">");
          }
          // Write fields as JSON.
          else if (outputFormat == JSON)
          {
            // If multiples are allowed for this field, we want to treat it like
            // an array, e.g. "z": ["a","b","c"].

            // CASE 1: This field is no multiple field.
            if (!csvField->getMultipleItems())
            {
              // CASE 1.1. The last field was a multiple field.
              // Close the array by adding a ].
              if (lastField != NULL && lastField->getMultipleItems())
                toShow.append("]");

              // Now go on like always: Add a , to indicate the next element and
              // add it's name.
              if (lastField != NULL) toShow.append(",");
              toShow.append("\"" + csvField->getName() + "\":");
            }
            // CASE 2: This field is a multiple field.
            else
            {
              // CASE 2.1. The last field was a multiple field.
              if (lastField != NULL && lastField->getMultipleItems())
              {
                // CASE 2.1.1. The last field was the same field like this one.
                // Just add a ,
                if (lastField->getName() == csvField->getName())
                  toShow.append(",");
                // CASE 2.1.2. The last field was another multiple field.
                // Close the last one and open the new one.
                else
                {
                  toShow.append("],");
                  toShow.append("\"" + csvField->getName() + "\":[");
                }
              }
              // CASE 2.2. The last field was no multiple field.
              // This is the first one, so open it.
              else
              {
                if (lastField != NULL) toShow.append(",");
                toShow.append("\"" + csvField->getName() + "\":[");
              }

              // If the fieldContent is empty, just continue. Otherwhise we
              // would append "" which doesn't make sense for arrays.
              if (field.empty())
              {
                lastField = csvField;
                continue;
              }
            }

            if (csvField->getFormat() == TEXT)
              toShow.append(escapeJson(field));
            else if (csvField->getFormat() == XML)
              toShow.append(escapeJson(field));
            else if (csvField->getFormat() == JSON)
              toShow.append(field);
          }
          lastField = csvField;
        }
      }
      if (outputFormat == JSON) toShow.append("}");
      // Else every title is printed.
      if (i < showLists.size() - 1) toShow += _infoDelim;
    }

    for (size_t i = 0; i < fields->size(); i++)
    {
      const CsvField& csvField =
        _fieldOptions[(*fields)[i].fieldIndex];
      if (csvField.getExcerpt())
      {
        // TODO: Why is the . needed here? Note that in the previous version of
        // the code there was ". " after every fieldContent, even if there was
        // only one where getExcerpt is true.
        if (excerpt.size() > 0) excerpt += string(". ");
        excerpt += (*fields)[i].fieldContent;
      }
    }
    fprintf(_docs_file,
            "%d\t%s:%s%d\t%s:%s\t%s:%s\n",
            docID,
            "u",
            "URL#",
            docID,
            "t",
            toShow.c_str(),
            "H",
            excerpt.c_str());
  }
}

// ____________________________________________________________________________
string CsvParser::escapeXml(const string& textToEscape) const
{
  //  return "<![CDATA[" + textToEscape + "]]>";
  string text;
  text.reserve(textToEscape.size());
  for (size_t i = 0; i < textToEscape.length(); i++)
  {
    if (textToEscape[i] == '&') text.append("&amp;");
    else if (textToEscape[i] == '\'') text.append("&apos;");
    else if (textToEscape[i] == '\"') text.append("&quot;");
    else if (textToEscape[i] == '<') text.append("&lt;");
    else if (textToEscape[i] == '>') text.append("&gt;");
    else
    {
      text.append(1, textToEscape[i]);
    }
  }
  return text;
}

// ____________________________________________________________________________
string CsvParser::escapeJson(const string& textToEscape) const
{
  string text;
  text.reserve(textToEscape.size() + 2);
  text.append("\"");
  for (size_t i = 0; i < textToEscape.length(); i++)
  {
    if (textToEscape[i] == '\"' || textToEscape[i] == '\\'
     || textToEscape[i] == '\n' || textToEscape[i] == '\t'
     || textToEscape[i] == '\b' || textToEscape[i] == '\f'
     || textToEscape[i] == '\r')
      text.append(1, '\\');
    text.append(1, textToEscape[i]);
  }
  text.append("\"");
  return text;
}

// _____________________________________________________________________________
bool CsvParser::containsNewLine(char* buffer)
{
  for (int i = 0; i < CSV_MAX_LINE_LENGTH; i++)
  {
    if (buffer[i] == '\n')
    {
      buffer[i] = '\0';
      return true;
    }
  }
  return false;
}

// NEW 15Nov12 (baumgari): Output predefined words for holding information
// about the index.
void CsvParser::addGlobalInformationToWordsFile()
{
  ParserBase::addGlobalInformationToWordsFile();
  // encoding
  string encoding = "iso-8859-1";
  if (_encoding == ParserBase::UTF8) encoding = "utf-8";
  writeInfoWord("encoding", encoding);
  // name
  writeInfoWord("name", getFileNameBase());
  // format of each field
  string formats = "";
  for (size_t i = 0; i < CsvField::getStaticShowList().size(); i++)
  {
    if (CsvField::getStaticShowList()[i].first == JSON) formats.append(1, 'J');
    if (CsvField::getStaticShowList()[i].first == XML)  formats.append(1, 'X');
  }
  writeInfoWord("field-formats", formats);

  // If we use multiple fields add an artificial word containing the used info
  // field delimiter.
  if (formats.size() > 1)
    writeInfoWord("field-delimiter", string(1, _infoDelim));

  // Add :info:facet:<name> for each facet and :info:multiple:<name>
  // for each field which can have multiple values.
  for (size_t i = 0; i < _fieldOptions.size(); i++)
  {
    if (_fieldOptions[i].getFacet())
      writeInfoWord("facet", _fieldOptions[i].getName());
    if (_fieldOptions[i].getMultipleItems())
      writeInfoWord("multiple", _fieldOptions[i].getName());
  }
}

// _____________________________________________________________________________
void CsvParser::parse()
{
  #ifdef DEBUG_CSV_PARSER
  cout << "CsvParser::parse ..." << flush;
  #endif

  char* buffer = new char[CSV_MAX_LINE_LENGTH];

  // Open CSV file.
  string csvFileName = ParserBase::getFileNameBase()
                        + ParserBase::getCsvFileNameSuffix();
  FILE* csvFile;
  csvFile = fopen(csvFileName.c_str(), "r");

  // Initialize parser (open output files and optionally read vocabulary, fuzzy
  // search clusters, etc.).
  ParserBase::init(ParserBase::getFileNameBase());
  addGlobalInformationToWordsFile();
  // If CSV file does not exist, write error to log-file and exit.
  if (csvFile == NULL)
  {
    perror("Can't open CSV file");
    exit(errno);
  }
  string record;
  string field;

  // How many field names did we read? In the folowing every fields has to have
  // this number of entries.
  int numberOfFields = _fieldOptions.size();
  int numberOfFieldsRead = 0;
  int docID = 1;
  vector<FieldItem> fields;

  // Read first line without doing anything with it.
  char* ret = fgets(buffer, CSV_MAX_LINE_LENGTH, csvFile);
  if (ret == NULL)
  {
    perror("Can't read from file");
    exit(errno);
  }
  cout << "Parsing " << csvFileName << " ... " << flush;
  cout.setf(std::ios::fixed);
  cout.precision(2);
  clock_t time = clock();

  const char fieldSeparator = _options.getColumnSeperator();
  const char fieldItemSeparator = _options.getFieldSeparator();

  #ifdef DEBUG_CSV_PARSER
  cout << endl
       << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"   << endl
       << "CsvParser: column-separator: '"
       << static_cast<int>(fieldSeparator) << "'"  << endl
       << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"   << endl
       << endl;
  cout << endl
       << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"       << endl
       << "CsvParser: field-item-separator: '"
       << static_cast<int>(fieldItemSeparator) << "'"  << endl
       << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"       << endl
       << endl;
  #endif


  // Walk through all the records.
  while (fgets(buffer, CSV_MAX_LINE_LENGTH, csvFile))
  {
    // Buffer must contain a '\n'.
    bool lineTooLong = !containsNewLine(buffer);
    if (lineTooLong)
    {
      cerr << "Line " << (docID + 1) << " too long, truncating to "
           << CSV_MAX_LINE_LENGTH / (1000 * 1000) << "M bytes" << endl;
      assert(CSV_MAX_LINE_LENGTH > 2);
      buffer[CSV_MAX_LINE_LENGTH - 2] = '\n';
    }
    numberOfFieldsRead = 0;
    fields.clear();

    // Go through the fields and store the entries in the vector<FieldItem>
    // fields.
    record = string(buffer);
    size_t start = 0;
    size_t pos = 0;

    // If line was too long, read in batches of CSV_MAX_LINE_LENGTH until
    // eventually a newline is found (and throw it all away). That way very long
    // lines don't lead to follow-up parse errors like in a previous version.
    while (lineTooLong)
    {
      fgets(buffer, CSV_MAX_LINE_LENGTH, csvFile);
      lineTooLong = !containsNewLine(buffer);
    }

    // Get a line from csv file.
    while (pos <= record.size())
    {
      while (pos < record.size() && record[pos] != fieldSeparator) ++pos;
      field = start < record.size() ? record.substr(start, pos - start) : "";

      // NEW 31Jan14 (baumgari): Fields, which may have multiple items, need to be
      // defined now by using --allow-multiple-items. For other fields, it's not
      // necessary to parse for the fieldItemSeparator.
      // Each field can contain multiple items. Find all of them and add them to
      // the fields vector.
      const CsvField& csvField = _fieldOptions[numberOfFieldsRead];
      size_t fieldSepPos = 0;
      size_t fieldPos = 0;

      if (csvField.getMultipleItems())
      {
        while ((fieldSepPos = field.find(fieldItemSeparator, fieldPos + 1))
               != string::npos)
        {
          const string& fieldItem =
            field.substr(fieldPos, fieldSepPos - fieldPos);
          fields.push_back(FieldItem(fieldItem, numberOfFieldsRead));
          fieldPos = fieldSepPos + 1;
        }
      }
      const string& fieldItem = field.substr(fieldPos);
      fields.push_back(FieldItem(fieldItem, numberOfFieldsRead));

      #ifdef DEBUG_CSV_PARSER
      cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl
           << "CsvParser:452: Column: " << field << endl
           << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl
           << endl;
      #endif
      // cout << "Field: \"" << field << "\"" << endl;
      numberOfFieldsRead++;
      ++pos;
      start = pos;
    }
    // If we read more or less fields than defined in the first line: log and
    // continue.
    if (numberOfFieldsRead != numberOfFields)
    {
      cerr << "Skipping line " << (docID + 1) << " with "
           << numberOfFieldsRead << " fields instead of "
           << numberOfFields << endl;

      fputs(record.c_str(), _log_file);
      fputs("\n", _log_file);
      fields.clear();
      numberOfFieldsRead = 0;
      docID++;
      continue;
    }
    // Pass fields to <basename>.docs and <basename>.words.
    writeFieldsToWordsAndDocsFile(docID, &fields);

    docID++;
  }
  size_t usecs = clock() - time;
  cout << "done in " << static_cast<double>(usecs) / 1000000
       << " seconds" << endl;

  // Done. Closes output files and optionally writes the vocabulary file.
  ParserBase::done();

  delete[] buffer;
}


