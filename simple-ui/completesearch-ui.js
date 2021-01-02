// CONFIGURATION ... TODO: this should eventually go in separate file.

// Class holding the configuration.
class Config {
  constructor() {
    this.origin = window.location.origin.replace(/:\d+$/, "");
    this.port = parseInt(window.location.port) + 1;
    // console.log("Origin: " + this.origin + ", port: " + this.port);
 
    // HTML for a hit. The hit contains the key excerpt, as well as a key for
    // each field specfied via the --show option when the CompleteSearch index
    // was buit (see the Makefile in the applications folder).
    this.hit_to_html = function(hit) {
      return "<div class=\"hit\">"
               + "<p class=\"title\"><a href=\"" + hit.wikipedia + "\">" + hit.title + " (" + hit.date + ")</a></p>"
               + "<p class=\"subtitle\">" + hit.director.join(", ") + "</p>"
               + "<p class=\"excerpt\">" + hit.excerpt + "</p>"
               + "</div>";
    };

    // Number of hits per page.
    this.hits_per_page = 7;
  }
}

class Global {
  constructor() {
    this.first_hit = 0;
    this.last_total = 0;
  }
}

var config = new Config();
var global = new Global();
var PAGE_UP = 34;
var PAGE_DOWN = 33;

// CLASSES (code for the web page comes at the end).

// A query for the CompleteSearch backend.
class Query {

  // Construct from query string. Append * if query is not empty. TODO: There is
  // more rewriting which can happen here + it should be configurable.
  constructor(query_string) {
    this.query_string = query_string;
    if (query_string.length > 0 && !query_string.endsWith(" "))
      this.query_string = query_string + "*";
  }

  // Send to backend and get result. The
  get_result(key_code) {
    // console.log("Key code: " + key_code);
    if (key_code == PAGE_UP) global.first_hit += config.hits_per_page;
    if (key_code == PAGE_DOWN) global.first_hit -= config.hits_per_page;
    if (global.first_hit + config.hits_per_page > global.last_total)
      global.first_hit = global.last_total - config.hits_per_page;
    if (global.first_hit < 0) global.first_hit = 0;

    var url = config.origin + ":" + config.port + "/?q="
                + this.query_string
                + "&h=" + config.hits_per_page
                + "&f=" + global.first_hit
                + "&format=json";
    console.log("Fetch URL: " + url);
    fetch(url)
      .then(response => response.json())
      .then(data => new QueryResult(data));
  }

}

// A result for a query.
class QueryResult {
  // Construct from JSON result from CompleteSearch backend. The structure of
  // this JSON is as follows:
  //
  // {
  //   result:
  //     {
  //       completions: { @total: ..., @computed: ..., @sent: ...,
  //         c: [ { @sc: ..., @dc: ..., @oc: ..., @id: ..., text: ... }, ... ] }
  //     }
  //     {
  //       hits: { @computed: ..., @first: ..., @sent: ..., @total: ...,
  //         hit: [ { @sore: ..., @id: ..., url: ..., excerpt: ...,
  //           info: { ...fields according to --show option } } ] }
  //     }
  // }
  constructor(json) {
    console.log(json);
    // The hits key should always be there, even if there are 0 hits.
    var hits = json.result.hits;
    this.num_hits_total = parseInt(hits["@total"]);
    this.num_hits_sent = parseInt(hits["@sent"]);
    this.num_hits_first = parseInt(hits["@first"]) + 1;
    global.last_total = this.num_hits_total;
    // If there are hits, read them from the JSON and make an array of HTML
    // strings out of it using config.hit_to_html. Note that Object.assign
    // merges two dictionaries. If there are not hits, the array is empty.
    if ("hit" in hits) {
      this.hits = json.result.hits.hit.map(
      hit => Object.assign({ "excerpt": hit.excerpt }, hit.info));
      this.html = this.hits.map(hit => config.hit_to_html(hit))
    } else {
      this.html = [];
    }
    this.display();
  }

  // Display query result. TODO: not for "out-of-sync" queries.
  display() {
    var header = this.num_hits_sent > 0
      ? "<p class=\"header\">Number of hits: " + this.num_hits_total
          + ", showing: " + this.num_hits_first + " &minus; "
          + (this.num_hits_first + this.num_hits_sent - 1) + "</p>"
      : ""; // <p>No hits</p>";
    $("div#results").html(header + "\n" + this.html.join("\n"));
  }
}

// A single matching item (document) from a QueryResult.
class QueryResultItem {
  // Construct from JSON.result.hits.hit returned by CompleteSearch.
  constructor(hit) {}
}

 

// MAIN CODE for the web page.
$(document).ready(function() {

  // Action when something happens in the search field
  $("div#search input").focus();
  $("div#search input").keyup(function(e) {
    var query = new Query($(this).val());
    query.get_result(e.keyCode);
  });

});
