// Constructor
function Query(facets, facetPrefix, filterPrefix)
{
  this.facets = facets;
  this.facetPrefix = facetPrefix;
  this.filterPrefix = filterPrefix;
  this.lastQuery;
  this.newQuery = '';
  this.queriesToLoad_onClick     = 100;
  this.queriesToLoad_whileTyping = 20;
}


Query.prototype.generateFacetQuery = function(id)
{
  var facet = '';
  var query = this.newQuery;
  if (this.facets[id].isFacet) facet = this.facetPrefix + id + ":*";
  if (this.newQuery.match(this.facetPrefix) || this.newQuery.match(this.filterPrefix)) return query + " " + facet;
  /*if (this.facets[id].useFilter && this.newQuery.length != 0)
  {
    var filter = this.filterPrefix + id + ":" + query.replace(/ /g, this.filterPrefix + id + ":" + query);
    return filter + " " + facet;
  }
  else
  {*/
    if (query.length == 0)
      return facet;
    else return query + " " + facet;
  // }
}

Query.prototype.convertQuery = function()
{
  var convertedQuery = '';
  // Erase all asterix to avoid generating multiple ones.
  var query = this.newQuery.replace(/\*/g, "");
  var words = query.split(" ");
  for (var i = 0; i < words.length; i++)
  {
    var convertedWord = words[i];
    if (convertedWord.length == 0) continue;
    // FACET WORD
    if (convertedWord.search(":") != '-1')
    {
      var prefix = '';
      if (convertedWord[0] == "-") {
        prefix = "-";
        convertedWord = convertedWord.substr(1);
      }
      convertedWord = this.quoteQueryWord(convertedWord);
      convertedWord = prefix + ":facet:" + convertedWord;
    }
    // EXACT MATCH
    // NEW (baumgari) 16Mai14: The server does support the dollar sign too, to
    // look for "real exact matches". E.g. müller finds "muller, müller,
    // mueller, müeller", whereas müller$ finds only "müller".
    else if (convertedWord.search(/\$$/) != '-1')
    {
      convertedWord = convertedWord.toLowerCase();
      //convertedWord = this.quoteQueryWord(convertedWord.substr(0, convertedWord.length - 1));
    }
    else
    {
      convertedWord = convertedWord.toLowerCase();
      convertedWord += '*';
    }
    // ~* to *~
    convertedWord = convertedWord.replace(/~\*$/, "*~");
    if (convertedQuery.length != 0) convertedQuery += " ";
    convertedQuery += convertedWord;
  }
  return convertedQuery;
}

Query.prototype.quoteQueryWord = function(word)
{
  var quotedWord = '';
  for (var i = 0; i < word.length; i++)
  {
    var ch = word[i].toLowerCase();
    // Quote everything that's not a character nor a number nor _ or :
    if (   (ch >= '0' && ch <= '9')
        || (ch >= 'a' && ch <= 'z')
        ||  ch.charCodeAt(0) > 127
        ||  ch == '_'
        ||  ch == ':'
        || (ch == '~' && i == word.length - 1)
        || (ch == '-' && i == 0))
      quotedWord += word[i];
    else
      quotedWord += "\"" + word[i] + "\"";
  }
  return quotedWord;
}
