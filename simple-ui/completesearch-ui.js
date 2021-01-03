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
               + "<p class=\"title\"><a href=\"" + hit.wikipedia + "\" target=\"_blank\">"
                                       + hit.title + " (" + hit.date + ")</a></p>"
               + "<p class=\"subtitle\">" + hit.director.join(", ") + "</p>"
               + "<p class=\"excerpt\">" + hit.excerpt + "</p>"
               + "</div>";
    };

    // Number of hits per page.
    this.hits_per_page = 7;

    // Number of completions in a facet box.
    this.completions_per_facet_box = 5;

    // Launch empty query on startup?
    this.launch_empty_query = true;

    // Background colors of the three parts of the screen (search field, facets,
    // results). Faded color of inactive word facet box.
    this.background_left = "#E9E9E9";
    this.background_right = "#F9F9F9";
    this.background_facets = "white";
    this.background_facets_faded = "#F3F3F3";
    this.foreground_facets = "black";
    this.foreground_facets_faded = "lightgray";
  }
}


class Global {
  constructor() {
    this.first_hit = 0;
    this.last_total = 0;
    this.facets_selected = new Set();
  }
}

var config = new Config();
var global = new Global();
var PAGE_UP = 34;
var PAGE_DOWN = 33;


// CLASSES (code for the web page comes at the end).

// A query for the CompleteSearch backend.
class Query {

  // Construct from query string. Append * if query is not empty.
  //
  // TODO: There is more rewriting which can and probably should happen here +
  // it should be configurable.
  constructor(query_string) {

    this.query_string = query_string;

    // Add a * if the query does not end with a space or *.
    if (query_string.length > 0
          && !query_string.endsWith(" ")
          && !query_string.endsWith("*"))
      this.query_string = query_string + "*";

    // Normalize, see below. Do this after the code above because the final
    // character might be a space. Then we don't want to add a *, but we want to
    // remove the space for the query.
    this.query_string = this.normalized(this.query_string);
  }

  // Compute normalized version of query string to avoid backend errors because
  // of trivial things. For now: memove whitespace in the beginning and compress
  // any sequence of whitespaces to a single whitespace.
  normalized(query_string) {
    return query_string.replace(/^\s+/, "")
                       .replace(/\s+$/, "")
                       .replace(/\s+/g, " ");
  }

  // Send to backend and get result.
  //
  // NOTE: The key_code is for the case that this call was triggered by a typing
  // action in the search field. This is useful to process actions like PageUp
  // or PageDown, used for paging. This function also gets called when selecting
  // or unselecting a facet. The we call it without argument, hence the default
  // of null.
  get_result(key_code = null) {
    // console.log("Key code: " + key_code);
    if (key_code == PAGE_UP) global.first_hit += config.hits_per_page;
    if (key_code == PAGE_DOWN) global.first_hit -= config.hits_per_page;
    if (global.first_hit + config.hits_per_page > global.last_total)
      global.first_hit = global.last_total - config.hits_per_page;
    if (global.first_hit < 0) global.first_hit = 0;

    // Don't ask for word completions if the last query word is a special word
    // (starting with a colon). For example, this happens when selecting a
    // facet for the empty query.
    var num_completions = config.completions_per_facet_box;
    if (this.query_string.match(/:\S+$/)) num_completions = 0;

    // Get hits and completions.
    var url = config.origin + ":" + config.port + "/?q="
                + this.query_string
                + "&h=" + config.hits_per_page
                + "&c=" + num_completions
                + "&f=" + global.first_hit
                + "&format=json";
    console.log("URL for hits and completion: " + url);
    fetch(url)
      .then(response => response.json())
      .then(data => new QueryHits(data))
      .then(data => new QueryCompletions(data, "Word"));

    // Get facets
    var _this = this;
    global.facet_names.forEach(function(facet_name) {
      var facet_word = ":facet:" + facet_name.toLowerCase() + ":*";
      var query_string_with_facets = _this.normalized(
        _this.query_string + " " + facet_word);
      var url = config.origin + ":" + config.port + "/?q="
                  + query_string_with_facets
                  + "&h=0"
                  + "&c=" + config.completions_per_facet_box
                  + "&format=json";
      console.log("URL for facet \"" + facet_name + "\": " + url);
      fetch(url)
        .then(response => response.json())
        .then(data => new QueryCompletions(data, facet_name));
    });

    // Make sure the focus is in the search field again (we may have clicked on
    // a facet and lost it).
    $("div#search input").focus();
  }

}

// The structure of the JSON returned by CompleteSearch is as follows:
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

// The HITS for a query.
class QueryHits {
  // Construct from JSON result (see above) from CompleteSearch backend.
  constructor(json) {
    console.log("Hits: ", json);
    // The hits key should always be there, even if there are 0 hits.
    var hits = json.result.hits;
    this.num_hits_total = parseInt(hits["@total"]);
    this.num_hits_sent = parseInt(hits["@sent"]);
    this.num_hits_first = parseInt(hits["@first"]) + 1;
    global.last_total = this.num_hits_total;

    // Set subtitle according to the total number of hits.
    var subtitle = "No hits";
    if (this.num_hits_total > 0) {
      subtitle = this.num_hits_total > 1 ? "documents" : "document";
      var num_hits_formatted = this.num_hits_total.toLocaleString();
      subtitle = json.result.query == ""
        ? "Searching in " + num_hits_formatted + " " + subtitle
        : "Zoomed in on " + num_hits_formatted + " " + subtitle
    }
    $("div#search p.subtitle").html(subtitle);

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
    this.display_hits();
    // Pass on to the next .then above.
    return json;
  }

  // Display hits in the results panel. TODO: not for "out-of-sync" queries.
  display_hits() {
    var header = this.num_hits_sent > 0
      ? "<p class=\"header\">Number of hits: " + this.num_hits_total
          + ", showing: " + this.num_hits_first + " &minus; "
          + (this.num_hits_first + this.num_hits_sent - 1) + "</p>"
      : ""; // <p>No hits</p>";
    $("div#results").html(header + "\n" + this.html.join("\n"));
  }
}

// The COMPLETIONS for a query.
class QueryCompletions {
  // Construct from JSON result (see above) from CompleteSearch backend.
  constructor(json, facet_name) {
    console.log("Completions for \"" + facet_name + "\": ", json);
    this.facet_name = facet_name;
    // The hits key should always be there, even if there are 0 hits.
    var completions = json.result.completions;
    this.num_completions_total = parseInt(completions["@total"]);
    this.num_completions = parseInt(completions["@sent"]);
    // If there are hits, read them from the JSON and make an array of HTML
    // strings out of it using config.hit_to_html. Note that Object.assign
    // merges two dictionaries. If there are not hits, the array is empty.
    if ("c" in completions) {
      // TODO: If single completion, CompleteSearch currently does not return an
      // array (which is just weird and should be fixed).
      this.completions = json.result.completions.c;
      if (!Array.isArray(this.completions)) this.completions = [ this.completions ];
      // TODO: Sort by @dc, @oc, @sc, @id according to config parameter.
      this.completions = this.completions.map(
        completion => [completion["text"], completion["@sc"]]);
    } else {
      this.completions = [];
    }
    this.display_completions();
    // Pass on to the next .then above.
    return json;
  }

  // Display completions in the box corresponding to the facet name.
  //
  // NOTE: This might be called after we selected a facet. It is therefore
  // crucial that we set the value of each checkbox according to the values in
  // global.facets_selected. Otherwise a selected facet will appear again in the
  // new result, but unselected.
  //
  // TODO: not for "out-of-sync" queries.
  display_completions() {
    var table_selector = "div#facets table." + this.facet_name + " tbody";
    // console.log("Table selector: " + table_selector);
    // First clear all rows. TODO: probably not too efficient.
    $(table_selector + " td").html("");
    // Fill the rows with our completions (as many as we have, but not more than
    // config.completions_per_facet_box).
    var n = Math.min(this.completions.length, config.completions_per_facet_box);
    for (var i = 0; i < n; i++) {
      var table_row_selector = table_selector + " tr:nth-child(" + (i + 1) + ")";
      // console.log("Table row selector: " + table_row_selector);
  
      // Fill the i-th table row with the corresponding HTML.
      //
      // IMPORTANT: The "let" is crucial here because otherwise the functions
      // ("closures") passed to change below all have the same context and
      // "completion_text" will be the last value of the variable for all of
      // them. An alternative would be to use forEach instead of the for loop.
      let completion_text = this.completions[i][0];
      var completion_score = this.completions[i][1];
      $(table_row_selector).html(
        this.completion_to_html(completion_text, completion_score));

      // Make sure that the checkbox has the right status.
      if (global.facets_selected.has(completion_text))
        $(table_row_selector + " input").prop("checked", true);

      // Add action when selecting or unselecting this facet.
      // console.log("Input selector: " + table_row_selector + " input");
      var _this = this;
      $(table_row_selector + " input").change(() =>
        _this.facet_selection_changed(completion_text));
    }

    // When the WORDS box is not needed, fade it out, otherwise fade it in.
    if (this.facet_name.toLowerCase() == "word") {
      console.log("#COMPLETIONS: ", this.num_completions);
      var words_box_bg = this.num_completions > 0
        ? config.background_facets : config.background_facets_faded;
      var words_box_fg = this.num_completions > 0
        ? config.foreground_facets : config.foreground_facets_faded;
      $("div#facets table.word td").css("background-color", words_box_bg);
      $("div#facets table.word th").css("background-color", words_box_bg);
      $("div#facets table.word th").css("color", words_box_fg);
    }
  }

  // Make HTML of a table row, from a completion and a score. A row has three
  // columns: a checkbox for selection, the completion text, the score.
  completion_to_html(completion_text, completion_score) {
    var checkbox_td = "<td><input type=\"checkbox\"></td>";
    var text_td = "<td>"
      + completion_text.replace(/.*:/, "").replace(/_/g, " ") + "</td>";
    var score_td = "<td>" + completion_score + "</td>";
    return checkbox_td + text_td + score_td;
  }

  // Action when selecting a facet. The argument is the DOM element of the
  // checkbox.
  //
  // TODO: This is currently just reverting the status of the facet. Better make
  // sure that this in sync with the status of the checkbox or it could happen
  // that the behavior is the reverse of that was intended.
  facet_selection_changed(completion_text) {
    // Add the facet if it's not there, delete it if it's there.
    var facet = completion_text;
    if (global.facets_selected.has(facet)) {
      global.facets_selected.delete(facet);
    } else {
      global.facets_selected.add(facet);
    }
    // console.log("Facet selection changed for \"" + completion_text + "\"");
    console.log("Facet selection is now: ", global.facets_selected);

    // Launch new query with facets prepended.
    //
    // TODO: Is it really best to read the query from the search field again
    // here? Or should we better take the one from the JSON result? In the
    // typical situation, they should be the same.
    var query_string_with_facets = Array.from(global.facets_selected).join(" ")
      + " " + $("div#search input").val();
    console.log("Launching query with facets: " + query_string_with_facets);
    var query = new Query(query_string_with_facets);
    query.get_result();
  }
}

// Initialization after the page has loaded and the facet boxes have been
// constructed.
//
// 1. Set colors according to configuration
// 2. Launch initial query.
// 3. What else?
//
class Initialization {
  constructor() {
    // Set colors according to configuration
    $("div#search").css("background-color", config.background_left);
    $("div#facets").css("background-color", config.background_left);
    $("div#results").css("background-color", config.background_right);

    // Launch empty query. TODO: make optional depending on config.
    if (config.launch_empty_query) (new Query("")).get_result()
  }
}

 
// Class for initially constructing the (empty) facet boxes. And finding out
// which facets there are in the first place.
//
// TODO: It is awkward that we have all the initialization in the constructor, in
// a large chain of "then". However, note that there is an order to these calls:
//
// 1. First the HTML needs to be loaded
// 2. We need the name of the facets from the backend
// 3. Only after 1 and 2 have finished, can we construct the facet boxes
// 4. Only then can we ask the initial (empty) query.
//
class FacetBoxes {

  // Get all facet names and add the corresponding boxes.
  constructor()  {
    // Add facet box for WORD completions.
    this.create_box("Word");

    // Add facet boxes for each facet from the special :info:facet:<name> words
    // in the index.
    var _this = this;
    fetch(config.origin + ":" + config.port
            + "/?q=:info:facet:*&h=0&c=999&format=json")
      .then(response => response.json())
      .then(data => global.facet_names =
        data.result.completions.c.map(c => c.text.replace(/^.*:/, "")))
      .then(() => console.log("Facet names found: ", global.facet_names))
      .then(() => global.facet_names.forEach(
        function(facet_name) { _this.create_box(facet_name); }))
      .then(() => (new Initialization()))
  }

  // Add box with given facet name.
  create_box(facet_name) {
    var table_header_html =
      "<tr><th colspan=\"3\">Refine by " + facet_name.toUpperCase() + "</th></tr>";
    var table_rows_html = new Array(config.completions_per_facet_box);
    table_rows_html.fill("<tr><td></td><td></td><td></td></tr>");
    var table_html =
      "<table class=\"" + facet_name.toLowerCase() + "\">"
          + "<thead>" + table_header_html + "</thead>\n"
          + "<tbody>" + table_rows_html.join("\n") + "</tbody></table>";
    // console.log(table_html);
    $("div#facets").append(table_html);
  }
}

// MAIN CODE for the web page.
$(document).ready(function() {

  // Create the facet boxes.
  global.facet_boxes = new FacetBoxes();

  // Action when something happens in the search field. Make sure that the facet
  // selection is cleared in that case.
  $("div#search input").focus();
  $("div#search input").keyup(function(e) {
    var query = new Query($(this).val());
    global.facets_selected.clear();
    query.get_result(e.keyCode);
  });
});
