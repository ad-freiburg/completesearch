// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <string>
#include "./QueryParameters.h"

using std::string;

namespace ad_semsearch
{
// _____________________________________________________________________________
void QueryParameters::setToDefaultValues()
{
  _nofWordsToSend = 0;
  _firstWordToSend = 0;
  _nofInstancesToSend = 0;
  _firstInstanceToSend = 0;
  _nofRelationsToSend = 0;
  _firstRelationToSend = 0;
  _nofClassesToSend = 0;
  _firstClassToSend = 0;
  _nofHitGroupsToSend = 20;
  _firstHitGroupToSend = 0;
}
// _____________________________________________________________________________
void QueryParameters::constructFromParamMap(
    const ad_utility::HashMap<string, string>& paramsMap)
{
  ad_utility::HashMap<string, string>::const_iterator it;
  it = paramsMap.find("nofwords");
  if (it != paramsMap.end()) _nofWordsToSend = atol(it->second.c_str());
  it = paramsMap.find("firstword");
  if (it != paramsMap.end()) _firstWordToSend = atol(it->second.c_str());
  it = paramsMap.find("nofinstances");
  if (it != paramsMap.end()) _nofInstancesToSend = atol(it->second.c_str());
  it = paramsMap.find("firstinstance");
  if (it != paramsMap.end()) _firstInstanceToSend = atol(it->second.c_str());
  it = paramsMap.find("nofrelations");
  if (it != paramsMap.end()) _nofRelationsToSend = atol(it->second.c_str());
  it = paramsMap.find("firstrelation");
  if (it != paramsMap.end()) _firstRelationToSend = atol(it->second.c_str());
  it = paramsMap.find("nofclasses");
  if (it != paramsMap.end()) _nofClassesToSend = atol(it->second.c_str());
  it = paramsMap.find("firstclass");
  if (it != paramsMap.end()) _firstClassToSend = atol(it->second.c_str());
  it = paramsMap.find("nofhitgroups");
  if (it != paramsMap.end()) _nofHitGroupsToSend = atol(it->second.c_str());
  it = paramsMap.find("firsthitgroup");
    if (it != paramsMap.end()) _firstHitGroupToSend = atol(it->second.c_str());
}
}
