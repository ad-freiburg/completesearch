// GLOBAL VARIABLES

// NOTE: The configuration is in a separate file config.js

class Global {
  constructor() {
    // Origin and port of the web page.
    this.origin = window.location.origin.replace(/:\d+$/, "");
    this.port = parseInt(window.location.port) + 1;
    // console.log("Origin: " + this.origin + ", port: " + this.port);
 
    // Global query count, so that each query gets a unique ID.
    this.num_queries = 0;
    // Keep track of where we are in the results and how many.
    this.first_hit = 0;
    this.last_total = 0;
    // Keep track of the selected facets.
    this.facets_selected = new Set();
  }

  // Get time from JSON returned by CompleteSearch. Will return the time in
  // milliseconds as integer, for example: 34ms
  time_from_json(json) {
    return Math.round(parseFloat(json.result.time.text)) + "ms";
  }
}

var global = new Global();
var PAGE_UP = 34;
var PAGE_DOWN = 33;


// CLASSES (code for the web page comes at the end).

// A query for the CompleteSearch backend.
class Query {

  // New query. The default is to take the query from the input field and
  // prepend the selected facets. Only when a query string is given, do we take
  // exactly that query string.
  //
  // TODO: There is more rewriting which can and probably should happen here +
  // it should be configurable.
  constructor(query_string = null) {

    this.query_string = query_string;
    this.query_id = global.num_queries + 1;
    global.num_queries += 1;

    if (this.query_string == null) {

      // Get value of input field.
      this.query_string = $("div#search input").val();

      // Add a * after every query part that has length at least
      // config.min_prefix_length_to_append_star.
      var regex = new RegExp("(" + "\\w".repeat(
        Math.max(config.min_prefix_length_to_append_star, 1)) + "\\b)", "g");
      this.query_string = this.query_string.replace(regex, "$1*");
      // var parts = this.query_string.split(/[ \.]+/);
      // var last_word_long_enough_to_append_star =
      //   parts.length > 0 && parts[parts.length - 1].length
      //                         >= config.min_prefix_length_to_append_star;
      // if (last_word_long_enough_to_append_star
      //       && !this.query_string.endsWith(" ")
      //       && !this.query_string.endsWith("*"))
      //   this.query_string = this.query_string + "*";

      // Prepend the selected facets. Escape with "..." because they may contain
      // separator like . or others. If facetids are available in the index, use
      // these because it's much more efficient (for each facet, the
      // :facet:<facet_name> words form one big block, so that operations on a
      // single of these words tend to be expensive).
      //
      // TODO: It would be easy to add a special :info: word for this. It
      // facetids are available, we should always use them, otherwise we can't.
      var facetid_prefix = config.facetids_available ? ":facetid:" : ":facet:"
      this.query_string = Array.from(global.facets_selected)
            .map(facet_word => "\""
                  + facet_word.replace(/^:facet:/, facetid_prefix)
                  + "\"").join(" ")
              + " " + this.query_string;

      // Normalize, see below. Do this after the code above because the final
      // character might be a space. Then we don't want to add a *, but we want to
      // remove the space for the query.
      this.query_string = this.normalized(this.query_string);
    }

    console.log("QUERY #" + this.query_id + ": \"" + this.query_string + "\"");
  }

  // Compute normalized version of query string to avoid backend errors because
  // of trivial things. For now: memove whitespace in the beginning and compress
  // any sequence of whitespaces to a single whitespace and any sequences of
  // more than one * into a single *.
  normalized(query_string) {
    return query_string
             .replace(/\s+/g, " ")       // single spaces only
             .replace(/^\w+/, "")        // clip leading non-word chars
             .replace(/[ ,;.-]+$/, "")   // clip trailing separator chars
             .replace(/\*+$/, "*");      // at most one * 
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

    // If the query_string was changed, we need to reset the facets and the
    // first hit, but only then! We have to pay attention that we do not reset
    // when we page or when we added or removed a facet.
    this.query_has_changed = 
      key_code != null && key_code != PAGE_UP && key_code != PAGE_DOWN;
    if (this.query_has_changed) {
      // global.facets_selected.clear();
      global.first_hit = 0;
    }

    // If the query is too short according to the config, we don't launch a
    // query. Exception: the empty query.
    var regex = new RegExp("\\w".repeat(
      config.min_prefix_length_to_launch_query) + "[\"*]?$");
    if (this.query_string.length > 0 && !this.query_string.match(regex)) {
      console.log("No query launched, last query word is shorter than "
                    + config.min_prefix_length_to_launch_query);
      return;
    }

    // Don't ask for word completions if the last query word is a special word
    // (starting with a colon). For example, this happens when selecting a
    // facet for the empty query.
    var num_completions = config.completions_per_facet_box;
    if (this.query_string.match(/:\S+$/)) num_completions = 0;

    // Figure out the ranking of hits and completions from the config. The || is
    // taken if there is no entry for "word" in config.how_to_rank_completions.
    // Note that config.how_to_rank_hits is just a fixed string.
    var how_to_rank_hits = config.how_to_rank_hits;
    var how_to_rank_completions = config.how_to_rank_completions["word"]
                                    || config.how_to_rank_completions["default"];

    // Get hits and completions.
    var url = global.origin + ":" + global.port + "/?q="
                + this.query_string
                + "&h=" + config.hits_per_page
                + "&c=" + num_completions
                + "&rd=" + how_to_rank_hits
                + "&rw=" + how_to_rank_completions
                + "&f=" + global.first_hit
                + "&format=json";
    // console.log("URL for hits and completion: " + url);
    var _this = this;
    fetch(url)
      .then(response => response.json())
      .then(data => new QueryHits(data, _this.query_id))
      .then(data => new QueryCompletions(data, "word", _this.query_id));

    // Get facets
    global.facet_names.forEach(function(facet_name) {

      // Figure out the ranking this facet from the config.
      var how_to_rank_completions = 
        config.how_to_rank_completions[facet_name]
          || config.how_to_rank_completions["default"]

      var facet_word = ":facet:" + facet_name.toLowerCase() + ":*";
      var query_string_with_facets = _this.normalized(
        _this.query_string + " " + facet_word);
      var url = global.origin + ":" + global.port + "/?q="
                  + query_string_with_facets
                  + "&h=0"
                  + "&rw=" + how_to_rank_completions
                  + "&c=" + config.completions_per_facet_box
                  + "&format=json";
      // console.log("URL for facet \"" + facet_name + "\": " + url);
      fetch(url)
        .then(response => response.json())
        .then(data => new QueryCompletions(data, facet_name,
                                             _this.query_id));
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
//         hit: [ { @score: ..., @id: ..., url: ..., excerpt: ...,
//           info: { ...fields according to --show option } } ] }
//     }
// }

// The HITS for a query.
class QueryHits {
  // Construct from JSON result (see above) from CompleteSearch backend.
  constructor(json, query_id) {

    this.query_id = query_id;
    console.log("Hits #" + this.query_id + " in "
      + global.time_from_json(json) + ":", json);

    // The "hits" property should always be there, even if there are no hits.
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
        hit => Object.assign({ "excerpt": hit.excerpt,
                               "@sc": hit["@score"],
                               "@id": hit["@id"] },
                             hit.info));
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
      ? "<p class=\"header\">Number of hits: " 
          + this.num_hits_total.toLocaleString()
          + ", showing: " + this.num_hits_first + " &minus; "
          + (this.num_hits_first + this.num_hits_sent - 1) + "</p>"
      : ""; // <p>No hits</p>";
    $("div#results").html(header + "\n" + this.html.join("\n"));
  }
}

// The COMPLETIONS for a query.
class QueryCompletions {

  // Construct from JSON result (see above) from CompleteSearch backend.
  constructor(json, facet_name, query_id) {

    this.facet_name = facet_name;
    this.query_id = query_id;
    console.log("\"" + facet_name + "\" #" + query_id + " in "
                     + global.time_from_json(json) + ":", json);

    // Note that result.completions always exists, even if there are no
    // completions.
    var completions = json.result.completions;
    this.num_completions_total = parseInt(completions["@total"]);
    this.num_completions = parseInt(completions["@sent"]);

    // Figure out the key of the score to be displayed from the config. This is
    // one of "@sc", "@dc", "@oc", "@id".
    var completion_scores_displayed_key =
      config.completion_scores_displayed[this.facet_name]
        || config.completion_scores_displayed["default"]

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
        completion => [completion["text"],
                       completion[completion_scores_displayed_key]]);
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
    var table_selector = "div#facets table." + this.facet_name;
    // console.log("Table selector: " + table_selector);

    // First clear all rows. TODO: probably not too efficient.
    $(table_selector + " tbody td").html("");

    // Show the number of completions in the table footer.
    var table_footer_html = this.num_completions == 0 ? ""
      : "1 &minus; " + this.num_completions + " of "
                     + this.num_completions_total.toLocaleString();
    $(table_selector + " tfoot td").html(table_footer_html);

    // Fill the rows with our completions (as many as we have, but not more than
    // config.completions_per_facet_box).
    var n = Math.min(this.completions.length, config.completions_per_facet_box);
    for (var i = 0; i < n; i++) {
      var table_row_selector = table_selector
                                 + " tbody tr:nth-child(" + (i + 1) + ")";
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
      // console.log("#COMPLETIONS: ", this.num_completions);
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

    // Reset first hit (we are getting new results now).
    global.first_hit = 0;

    // Launch new query with facets prepended (is done by the Query class
    // automatically when constructor is called without arguments).
    var query = new Query();
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
    this.create_box("word");

    // If config.facet_names is undefined, add facet boxes for each facet from
    // the special :info:facet:<name> words in the index. 
    // TODO: There is some ugly code duplication here, which should be avoided.
    var _this = this;
    if ("facet_names" in config) {
      global.facet_names = config.facet_names;
      global.facet_names.forEach(
        function(facet_name) { _this.create_box(facet_name); });
      new Initialization();
    } else {
      fetch(global.origin + ":" + global.port
              + "/?q=:info:facet:*&h=0&c=999&format=json")
        .then(response => response.json())
        .then(data => global.facet_names =
          data.result.completions.c.map(c => c.text.replace(/^.*:/, "")))
        .then(() => console.log("Facet names found: ", global.facet_names))
        .then(() => global.facet_names.forEach(
          function(facet_name) { _this.create_box(facet_name); }))
        .then(() => (new Initialization()))
    }
  }

  // Add box with given facet name.
  create_box(facet_name) {
    var table_header_html =
      "<tr><th colspan=\"3\">Refine by " + facet_name.toUpperCase() + "</th></tr>";
    var table_footer_html = "<tr><td colspan=\"3\"></td></tr>";
    var table_rows_html = new Array(config.completions_per_facet_box);
    table_rows_html.fill("<tr><td></td><td></td><td></td></tr>");
    var table_class = facet_name.toLowerCase();
    var table_html =
      "<div class=\"wrapper\">\n"
        + "<div class=\"scroll\">\n"
          + "<table class=\"" + table_class + "\">"
              + "<thead>" + table_header_html + "</thead>\n"
              + "<tbody>" + table_rows_html.join("\n") + "</tbody>"
              + "<tfoot>" + table_footer_html + "</tfoot>\n"
          + "</table></div></div>";
    // console.log(table_html);
    $("div#facets").append(table_html);
  }
}

// MAIN CODE for the web page.
$(document).ready(function() {

  // Create the facet boxes.
  global.facet_boxes = new FacetBoxes();

  // Action when something happens in the search field.
  $("div#search input").focus();
  $("div#search input").keyup(function(e) {
    var query = new Query();
    query.get_result(e.keyCode);
  });
});
