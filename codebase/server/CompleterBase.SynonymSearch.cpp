#include "CompleterBase.h"
#include "HYBCompleter.h"
#include "synonymsearch/SynonymDictionary.h"

extern SynonymDictionary synonymDictionary;

template <unsigned char MODE>
void CompleterBase<MODE>::processSynonymSearchQuery
                           (const QueryResult& inputList, 
                            const Query&       firstPartOfQuery, 
                            const Query&       lastPartOfQuery,
                            const Separator&   separator,
                                  QueryResult& result)
{
  Query newLastPartOfQuery = lastPartOfQuery;
  newLastPartOfQuery.removeLastCharacter();
  bool isPrefixQuery = newLastPartOfQuery.getLastCharacter() == '*';
  if (isPrefixQuery) newLastPartOfQuery.removeLastCharacter();
  vector<uint32_t> synonymGroupIds;
  log << "! NEW SYN: looking for synonym group of \""
      << newLastPartOfQuery.getQueryString() << "\"" << endl;
  synonymDictionary.getSynonymGroupIds(newLastPartOfQuery.getQueryString(),
                                       synonymGroupIds);
  ostringstream orOfSynonymGroupIdWords;
  if (synonymGroupIds.size() > 0)
  {
    log << "! NEW SYN: found synonym ids ";
    for (size_t i = 0; i < synonymGroupIds.size(); ++i)
    {
      if (i > 0) log << ", ";
      log << synonymGroupIds[i];
      if (!synonymDictionary.stripAsteriskBitFromGroupId(&synonymGroupIds[i]))
        orOfSynonymGroupIdWords << "|S:" << synonymGroupIds[i]
                                << wordPartSep << "*";
    }
    log << endl;
  }
  if (isPrefixQuery) newLastPartOfQuery.append("*");
  log << "! NEW SYN: appending disjunction of synonym group id words \""
      << orOfSynonymGroupIdWords.str() << "\"" << endl;
  newLastPartOfQuery.append(orOfSynonymGroupIdWords.str());
  log << "! NEW SYN: replacing \"" << lastPartOfQuery.getQueryString() << "\""
      << " by \"" << newLastPartOfQuery.getQueryString() << "\"" << endl;
  processBasicQuery(inputList,
                    firstPartOfQuery,
                    newLastPartOfQuery,
                    separator,
                    result);
}

// Explicit instantiation (so that actual code gets generated).
template void CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>
                ::processSynonymSearchQuery
                  (const QueryResult& inputList, 
                   const Query&       firstPartOfQuery, 
                   const Query&       lastPartOfQuery,
                   const Separator&   separator,
                         QueryResult& result);
