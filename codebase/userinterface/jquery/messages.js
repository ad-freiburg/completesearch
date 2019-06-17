function Messages()
{
  this.errors = {
    "NO_HITS":
    {
      "de": "Leider nichts gefunden. "
    },
  };

  this.infos = {
    "INSTRUCTIONS":
    {
      "en": "  <h2>Features</h2>" +
            "  <ul>" +
            "    <li>fast search as you type:</b> a search is triggered after each keystroke, with instant response times; if not, your network connection is lame ;-)</li>" +
            "    <li>prefix search:</b> e.g., <a href=\"index.html#q=sig\">sig</a> matches SIGIR (the search is always case-insensitive, too); no prefix search for single letters!</li>" +
            "    <li>error-tolerant search:</b> append a tilda, e.g., <a href=\"index.html#q=probabi~\">probabi~</a> will also match \"probalistic\"</li>" +
            "    <li>faceted search:</b> at any time, the right sidebar gives a breakdown of the current result set (initially: all entries) by various categories; click an item to refine</li>" +
            "    <li>exact word match:</b> end word with a dollar, e.g.  <a href=\"index.html#q=graph$\">graph$</a></li>" +
            "    <li>previous/next query:</b> just use the browser back/forward buttons; typically Alt-left/right work for this, too!</li>" +
            "    <li>boolean or:</b> put a pipe between words, e.g., <a href=\"index.html#q=graph|network\">graph|network</a></li>" +
            "    <li>boolean not:</b> put a minus before a word, e.g., <a href=\"index.html#q=-venue:corr kernelization\">-venue:corr kernelization</a></li>" +
            "    <li>phrase search:</b> put a dot between words, e.g., <a href=\"index.html#q=information.retrieval\">information.retrieval</a></li>" +
            "    <li>specify number of authors:</b> add \"na:1:\" to your query to restrict your search to single-authored papers, or \"na:2:\" etc.</li>" +
            "    <li>only first-authored papers:</b> prepend \"first\" to the author token, as in <a href=\"index.html#q=firstauthor:Michael_Stonebraker\">firstauthor:Michael_Stonebraker</a></li>" +
            "  </ul>" 
    }
  }

  this.warnings = {};
}

Messages.prototype.printInfo = function(type, language)
{
  return this.infos[type][language];
}

Messages.prototype.printError = function(type, language)
{
  return this.errors[type][language];
}
