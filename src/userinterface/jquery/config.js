function Config()
{
  /* Specifies, which part of the completesearch result must be taken to get the
   * html version. */
  this.param4html = {"p": 1};
  /*  Specifying the existing facets and their properties */
  this.facets = {
    "word":   { "isFacet": false,  "title": "words"},
    "author": { "isFacet": true,   "title": "author or coauthor"},
    "type":   { "isFacet": true,   "title": "type"},
    "venue":  { "isFacet": true,   "title": "venue"},
    "year":   { "isFacet": true,   "title": "year"} 
  };
  this.apis = ["xml", "json", "jsonp"];
  this.numOfCompletionsDefault = 4;
}
