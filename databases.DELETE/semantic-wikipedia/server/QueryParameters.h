// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_QUERYPARAMETERS_H_
#define SEMANTIC_WIKIPEDIA_SERVER_QUERYPARAMETERS_H_

#include <stdlib.h>
#include <string>

#include "../codebase/semantic-wikipedia-utils/HashMap.h"

using std::string;

namespace ad_semsearch
{
class QueryParameters
{
    public:

    //! Default ctor.
    QueryParameters()
    {
      setToDefaultValues();
    }

    //! Set all parameters to  default values.
    void setToDefaultValues();

    void constructFromParamMap(
        const ad_utility::HashMap<string, string>& paramsMap);

    // Available parameters

    size_t _nofInstancesToSend;
    size_t _firstInstanceToSend;
    size_t _nofRelationsToSend;
    size_t _firstRelationToSend;
    size_t _nofClassesToSend;
    size_t _firstClassToSend;
    size_t _nofHitGroupsToSend;
    size_t _firstHitGroupToSend;
    size_t _nofWordsToSend;
    size_t _firstWordToSend;
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_QUERYPARAMETERS_H_
