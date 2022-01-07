// GLOBAL STUFF

// NOTE: The configuration is in a separate file config.js. Ideally, there
// should be no need to change anything in this file to suit an application,
// but only in config.js

var config = new Config();

// TEST: Try to figure out the path via which this JS was loaded.
// var scripts = document.getElementsByTagName("script");
// var src = scripts[scripts.length - 1].src;
// console.log("Source of JavaScript: ", src)

class Global {
  constructor() {

    // Base URL of the page.
    this.base_url = window.location;
    console.log("Base URL: " + this.base_url);
    // this.origin = window.location.origin.replace(/:\d+$/, "");
    // this.port = parseInt(window.location.port) + 0;
    // console.log("Origin: " + this.origin + ", port: " + this.port);

    // Read query after # in URL and add to input field if zero.
    // var hash_query = window.location.hash.match(/query=([^&]+)/);
    // if (hash_query) console.log("HASH query:", hash_query);
      // $("div#search input").val(hash_query[1]);
 
    // Global query count, so that each query gets a unique ID. Also count the
    // number of "rounds" = the number of times, the input in the search field
    // was actually changed.
    this.num_queries = 0;
    this.num_rounds = 0;

    // Initial excerpt radius (can be controlled with ALT-l/m).
    this.excerpt_radius = config.excerpt_radius;

    // The hit box and the facet boxes. These will be will be filled by
    // initialize_boxes below.
    this.hit_box = null;
    this.facet_boxes = {};

    // TODO: not sure if we ever need an array with the names of factes, since
    // we have this.facet_boxes
    this.facet_names = [];

    // Keep track of the selected facets and know which names there are.
    this.facets_selected = new Set();

    // Regex matching separator and word character.
    this.sep_char_regex = "[ ,;.-]";
    this.word_char_regex = "[^ ,;.-]";

    // Set colors according to configuration.
    //
    // NOTE: Make sure that the Global object is created *after* the page has
    // actually loaded, otherwise these will have no effect.
    // $("div#search").css("background-color", config.background_left);
    // $("div#facets").css("background-color", config.background_left);
    $("div#results").css("background-color", config.background_right);
  }

  // Helper function that turns the argument into an array unless it already is
  // one.
  make_array(array_or_value) {
    return Array.isArray(array_or_value) ? array_or_value : [ array_or_value ];
  }
  
  // Create and initialize the (single) hit box and the facet boxes.
  //
  // NOTE: This should be called only after the classes HitBox and FacetBox are
  // acutally defined.
  initialize_boxes() {
    console.debug("Initializing hit and facet boxes ...");
    // Initialize the hit box.
    this.hit_box = new HitBox();

    // Add facet box for WORD completions.
    this.facet_boxes["word"] = new FacetBox("word");

    // Add the other facet boxes. If facet_names are specified in the config,
    // take those, otherwise determine them automatically via the special
    // special :info:facet:<name> words in the index. 
    let _this = this;
    if ("facet_names" in config) {
      console.debug("Getting facet names from config: ", config.facet_names);
      global.facet_names = config.facet_names;
      global.facet_names.forEach(function(facet_name) {
        _this.facet_boxes[facet_name] = new FacetBox(facet_name);
      });
    } else {
      console.debug("Getting facet names from index ...");
      var url = global.base_url + "?q=:info:facet:*&h=0&c=999&format=json";
      fetch(url)
        .then(response => response.json())
        .then(data => global.facet_names =
          global.make_array(data.result.completions.c)
            .map(c => c.text.replace(/^.*:/, "")))
        .then(() => console.log("Facet names found: ", global.facet_names))
        .then(() => global.facet_names.forEach(function(facet_name) {
          _this.facet_boxes[facet_name] = new FacetBox(facet_name); }))
        .catch(error => console.error("ERROR fetching result for "
          + "facet names query with " + url + ": " + error.message));
    }
  }

  // Get time from JSON returned by CompleteSearch. Will return the time in
  // milliseconds as integer, for example: 34ms
  time_from_json(json) {
    return Math.round(parseFloat(json.result.time.text)) + "ms";
  }
}

// Declared here because it's used in the following. But must be created only
// after the page has loaded, see below.
var global;


// CLASSES (code for the web page comes at the end).

// Class for asking queries to the CompleteSearch backend.
//
// TODO: This is a stub and not yet used. Still unsure what exactly should go
// into this class and what should go in HitBox and FacetBox.
class Backend {

  // Initialize.
  constructor() {
    // Base URL of CompleteSearch backend.
    this.base_url = global.base_url;

    // The values of the various parameters. Note that the expression after ||
    // is taken if the expression before is undefined.
    this.how_to_rank_hits = config.how_to_rank_hits;
    this.how_to_rank_completions = {
      "word" : config.how_to_rank_completions["word"]
          || config.how_to_rank_completions["default"] || "1d" };

    // 2D count for query id (1 = increased by one when something changes in the
    // search field, 2 = increased by selecting facets or loading more
    // completions or hits).
    this.query_id = [0, 0];
  }

  // Get hits.
  get_hits(query_string, num_hits, first_hit) {
    // Build URL from paramters
    var url = global.base_url
                + "?q=" + query_string
                + "&h=" + num_hits
                + "&c=0"
                + "&er=" + global.excerpt_radius
                + "&rd=" + this.how_to_rank_hits
                + "&f=" + first_hit
                + "&format=json";
    console.debug("URL for hits and completions: " + url);
    this.query_id[1] += 1;
    let _this = this;
    fetch(url)
      .then(response => response.json())
      .then(json => global.hit_box.update(
                      json, _this.query_id[0], _this.query_id[1]))
      .catch(error => console.error("ERROR fetching HITS via " 
                                      + url + ": ", error));
  }

  // Get completions for given facet.
  get_completions(query_string, num_completions, facet_name) {
    // Build URL from paramters
    var url = global.base_url
                + "?q=" + query_string
                + "&h=0"
                + "&c=" + num_completions
                + "&rw=" + this.how_to_rank_completions
                + "&format=json";
    console.debug("URL for hits and completions: " + url);
    this.query_id[1] += 1;
    let _this = this;
    fetch(url)
      .then(response => response.json())
      .then(json => global.facet_boxes[facet_name].update(
                      json, _this.query_id[0], _this.query_id[1]))
      .catch(error => console.error("ERROR fetching \"" + facet_name
                                      + "\" COMPL via " + url + ": ", error));
  }

  // if (global.hit_box) global.hit_box.scroll_to_top = true;
  // if (global.facet_boxes["word"]) global.facet_boxes["word"].scroll_to_top = true;
}


// The hit box on the right. This should be used as follows:
//
// 1. Initialize once in the beginning. This is taken care of by
// Global.initialize_boxes
//
// 2. Update using the method "update" which takes as arguments the JSON from
// the CompleteSearch backend and a unique query id (later queries, larger ids).
// This is called by the class Query
//
class HitBox {

  // We keep track of the hits, some meta data, and the id of the query from
  // which those hits originated. Note that usually only a fraction of the hits
  // is shown, so num_hits_total can be much larger than hits.length . We don't
  // really need num_hits_sent, since it should always be exactly equal to
  // hits.length
  constructor() {
    console.debug("Constructor HitBox() ...");

    // Keep track of where we are in the results and how many.
    this.hits = [];
    this.num_hits_total = 0;
    this.num_hits_sent = 0;
    this.first_hit = 0;
    this.query_id = 0;
    this.round_id = 0;
    this.query_string = "";
    this.scroll_to_top = true;

    // If the hit box scrolled to the end, and the number of hits shown are
    // less than the total number of hits, reload.
    let _this = this;
    $("div#results div.hits").scroll(function(event) {
      var scroll_top = Math.round($(this).scrollTop());
      var visible_height = Math.round(parseFloat($(this).css("height")));
      var max_height = $(this).prop("scrollHeight");
      var scroll_perc = Math.round(
        100 * scroll_top / (max_height - visible_height));
      if (scroll_perc == 100 &&
            _this.hits.length < _this.num_hits_total) {
        console.assert(_this.hits.length > 0);
        var num_hits_requested = Math.min(_this.num_hits_total,
          2 * _this.hits.length);  // + config.hits_per_page);
        console.log("Reloading " + num_hits_requested + " hits for query \""
          + _this.query_string + "\" ...");
        // TODO: The CompleteSearch engine currently does not send more than
        // 1000 results. Where is this configured or configurable?
        if (num_hits_requested > 1000) console.log(
          "NOTE: Requested " + num_hits_requested + " hits"
          + ", but CompleteSearch will not return more than 1,000");
        // TODO: It is not right to launch a completely new query here (which
        // will also reload the facet boxes).
        _this.scroll_to_top = false;
        _this.query(_this.query_string, _this.round_id, num_hits_requested);
      }
    });
  }

  // Launch query for more hits (when scrolled to bottom).
  //
  // TODO: This repeats code from the Query class, just like FacetBox.query
  // repeats code. This should be refactored.
  query(query_string, round_id, num_hits_requested) {

    // Get only hits, no completions.
    var url = global.base_url
                + "?q=" + query_string
                + "&f=" + global.first_hit
                + "&h=" + num_hits_requested
                + "&rd=" + config.how_to_rank_hits
                + "&er=" + global.excerpt_radius
                + "&c=" + 0
                + "&format=json";
    console.debug("URL for loading more hits: " + url);
    let _this = this;
    fetch(url)
      .then(response => response.json())
      .then(json => global.hit_box.update(
                      json, _this.query_id, _this.round_id))
      .catch(error => console.error("ERROR fetching result for "
                                     + url + ": " + error.message));
  }

  // Update from JSON returned by CompleteSearch backend. If the query id is
  // lower than the result already shown, the query is simply discarded. This
  // can happen when a query launched earlier returns later. It is common when
  // launching queries for short prefix lengths because these are more
  // expensive; see config.min_prefix_length_to_launch_query .
  update(json, query_id, round_id) {

    // Discard if old query.
    if (query_id < this.query_id) {
      console.debug("Received hits for Query #" + query_id
        + ", but hits for Query #" + this.query_id
        + " are already displayed ... discarding the former");
      return json;
    }
    this.query_id = query_id;
    this.round_id = round_id;
    console.info("Hits #" + this.round_id + "." + this.query_id
      + " in " + global.time_from_json(json) + ":", json);

    // Read meta data from JSON. Note that result.hits always exists (at least
    // it should) even if there are zero hits or the query was malformed. Also
    // note that hits["@first"] starts from zero, but we want it to start from
    // 1.
    //
    // NOTE: There is also key "@computed", are we interested in that?
    // CompleteSearch does not always compute full results for efficiency
    // reasons.
    this.query_string = json.result.query;
    var hits = json.result.hits;
    this.num_hits_total = parseInt(hits["@total"]);
    this.num_hits_sent = parseInt(hits["@sent"]);
    this.first_hit = parseInt(hits["@first"]) + 1;

    // If hits["@sent"] is non-zero, there should be an array json.result.hits.hit
    // with entries in the following format where info contain the fields from
    // the index according to the --show option passed to the CSV parser.
    //
    //   { @score: ..., @id: ..., url: ..., excerpt: ..., info: { ... } }
    //
    // We re-arrange this to a non-nested map and rename @score to @sc (for
    // consistency with result.competions) and drop url (TODO: find out how to
    // fill this with a meaningful value with the CSV parser).
    //
    //   { @sc: ..., @id: ..., excerpt: ..., <fields from info> }
    //
    console.assert("hit" in hits || this.num_hits_sent == 0);
    this.hits = this.num_hits_sent == 0 ? [] : json.result.hits.hit;
    this.hits = this.hits.map(
      // Object.assign merges two dictionaries.
      hit => Object.assign({ "@sc": hit["@score"],
                             "@id": hit["@id"],
                             "excerpt": hit["excerpt"] },
                           hit.info));
    // Now we can display the hits in the hit box.
    this.display_hits();

    // This function is called from a .then (after the result was received from
    // the CompleteSearch backend), so we need to pass it on, in case there is
    // another .then following it.
    return json;
  }

  // Display hits in the hit box.
  display_hits() {
    // In the header, show which hits out of how many we are showing.
    var header = this.num_hits_sent > 0
      ? "<p class=\"header\">Number of hits: " 
          + this.num_hits_total.toLocaleString()
          + ", showing: " + this.first_hit + " &minus; "
          + (this.first_hit + this.num_hits_sent - 1)
          + " ... <b>scroll for more hits, Alt-m / Alt-l for more or less text</b>"
          + "</p>"
      : ""; // <p>No hits</p>";
    $("div#results div.header").html(header);

    // Highlight query words in excerpt (see function below for details).
    this.highlight_query_words();

    // Make HTML using config.hit_to_html and show it.
    var html = this.hits.map(hit => config.hit_to_html(hit))
    $("div#results div.hits").html(html.join("\n"));
 
    // Set subtitle of search field according to the total number of hits.
    var subtitle = "No hits";
    if (this.num_hits_total > 0) {
      subtitle = this.num_hits_total > 1 ? "text records" : "text record";
      var num_hits_formatted = this.num_hits_total.toLocaleString();
      subtitle = this.query_string == ""
        ? "Searching in " + num_hits_formatted + " " + subtitle
        : "Zoomed in on " + num_hits_formatted + " " + subtitle
    }
    $("div#search p.subtitle").html(subtitle);

    // Scroll to top (deactivate when reloading hits after scrolling to the
    // bottom, activated otherwise).
    // console.log("SCROLL TO TOP: ", this.scroll_to_top);
    if (this.scroll_to_top) $("div#results div.hits").scrollTop(0);
  }

  // Highlight query words in excerpts.
  //
  // NOTE: This is done to some extent by CompleteSearch's ExcerptGenerator
  // class, but this has (a) some bugs and (b) it does not color partial words.
  // Therefore, this function first *undoes* the highlighting from the
  // ExcertsGenerator (tages of the form <hl idx="..."> and </hl> and then
  // inserts its own highlight.
  highlight_query_words() {
    // console.log("HIGHLIGHT in excerpts for query:", this.query_string);
    // Break query into its parts (treating all characters with special meaning
    // as whitespace). Remove special words first. Remove empty strings.
    var query_parts = this.query_string
                             .replace(/":.*?"/g, "")
                             .replace(/[\.,;:\*|\s]+/ug, " ").split(/\s+/)
                             .filter(s => s);
    // console.log("QUERY parts:", query_parts);
    // Build regex for each non-empty query part.
    var query_part_regexes = [];
    query_parts.forEach(function(query_part) {
      query_part_regexes.push(new RegExp("(" + query_part + ")", "uig"));
    });
    // For each excerpt, first remove all highlights from excerpts that were
    // inserted by CompleteSearch and then highlight the query parts.
    console.log("Highlight regexes:", query_part_regexes);
    this.hits.forEach(function(hit) {
      // For some hits, we might have an array of excerpts, just flatten this
      // by concatenating the excerpts.
      hit.excerpt = Array.isArray(hit.excerpt)
        ? hit.excerpt.join(" ") : hit.excerpt;
      // Remove existing highlights.
      if (hit.excerpt) hit.excerpt = hit.excerpt.replace(/<\/?hl[^>]*>/g, "");
      // For each query part, add highlight.
      query_part_regexes.forEach(function(query_part_regex, i) {
        if (hit.excerpt) hit.excerpt = hit.excerpt.replace(
          query_part_regex, "<hl idx=\"" + i + "\">$1</hl>");
        if (hit.title) hit.title = hit.title.replace(
          query_part_regex, "<hl idx=\"" + i + "\">$1</hl>");
        if (hit.h1) hit.h1 = hit.h1.replace(
          query_part_regex, "<hl idx=\"" + i + "\">$1</hl>");
      });
    });
  }
}


// A single facet box (including the "word" box). For each facet, one box is
// created in the beginning, by Global.initialize_boxes (called after the page
// has loaded, see code at the very end). In the following, the method "update"
// is triggered by one of two actions:
//
// 1. When a new query is launched and all facet boxes are updated. This is done
// via Query.get_result, which is called when the user has typed something in
// the search field.
//
// 2. When a facet box is scrolled down and new completions need to be loaded.
// TODO: this is not yet implemented, but should be easy to do, now that a
// proper class design is in place.
//
class FacetBox {

  // Create box for given facet and add it to the facets panel on the left.
  constructor(facet_name) {
    console.debug("Constructor FacetBox(\"" + facet_name + "\") ...");

    // Meta data about this box. The facet word is the word contained in the
    // index for each document that has this facet.
    this.facet_name = facet_name;
    this.completions = [];
    this.num_completions_total = 0;
    this.num_completions_sent = 0;
    this.query_id = 0;
    this.round_id = 1;
    this.query_string = "";
    this.facet_word_prefix_with_star =
      ":facet:" + facet_name.toLowerCase() + ":*";
    this.is_dimmed = true;
    this.scroll_to_top = true;

    // Figure out the ranking for this facet from the config, which either 
    // specifies an explicit value for that facet or a default value. If the
    // config does not even provide a default value, we sort by document
    // count in descendig order (see config.js for all the options).
    this.how_to_rank_completions = 
      config.how_to_rank_completions[facet_name]
        || config.how_to_rank_completions["default"] || "1d";

    // Figure out which kind of scores to display for this facet from the
    // config, which either specifies an explicit value for that facet or a
    // default value. If the config does not even provide a default value, we
    // display documents counts.
    this.completion_scores_displayed_key =
      config.completion_scores_displayed[this.facet_name]
        || config.completion_scores_displayed["default"] || "@dc";
   
    // Build the HTML of the table and add it to the facets panel. The
    // subsequent call to this.display_completions will add empty rows to the
    // body.
    //
    // NOTE: Filling the table with empty rows is also needed later when a facet
    // box had results for a previous query, but has no results for a new query.
    var table_header_html =
      "<tr><th colspan=\"3\">Refine by " + facet_name.toUpperCase() + "</th></tr>";
    var table_footer_html = "<tr><td colspan=\"3\"></td></tr>";
    // var table_rows_html = new Array(config.completions_per_facet_box);
    // table_rows_html.fill("<tr><td></td><td></td><td></td></tr>");
    var table_class = facet_name.toLowerCase();
    var table_html =
      "<div class=\"" + table_class + "\">"
        + "<table class=\"" + table_class + "\">"
        + "<thead>" + table_header_html + "</thead>\n"
        + "<tbody></tbody>"
        + "<tfoot>" + table_footer_html + "</tfoot>\n"
      + "</table></div>";
    $("div#facets").append(table_html);

    // Remember the table and div selector for this table. We will use it
    // frequently in the other functions (especially the table selector).
    // NEW: There is now also a div selector.
    this.table_selector = "div#facets table." + this.facet_name.toLowerCase();
    this.div_selector = "div#facets div." + this.facet_name.toLowerCase();
    // console.log("Table selector: " + table_selector);

    // Add empty rows.
    this.display_completions();

    // Initially, all the boxes are dimmed (indicating to the user that no
    // results have been requested).
    this.dim_if_true_undim_if_false(this.is_dimmed);

    // If the table is scrolled to the end, and the number of hits shown are
    // less than the total number of hits, reload.
    let _this = this;
    $(this.table_selector + " tbody").scroll(function(event) {
      var scroll_top = Math.round($(this).scrollTop());
      var visible_height = Math.round(parseFloat($(this).css("height")));
      var max_height = $(this).prop("scrollHeight");
      var scroll_perc = Math.round(
        100 * scroll_top / (max_height - visible_height));
      console.debug("Scrolled: "
        + "scroll top = " + scroll_top
        + ", visible height = " + visible_height
        + ", max height = " + max_height
        + ", scroll perc = " + scroll_perc + "%");
      // If scrolled to the bottom and not all completions are shown yet, reload
      // completions. We always double the number of completions (if there are
      // that many left).
      if (scroll_perc == 100 &&
            _this.completions.length < _this.num_completions_total) {
        console.log("Reloading " + _this.facet_name + " completions for "
          + " query \"" + _this.query_string + "\" ...");
        console.assert(_this.completions.length > 0);
        var num_completions_requested = Math.min(
          2 * _this.completions.length, _this.num_completions_total);
        // TODO: The CompleteSearch engine currently does not send more than
        // 1000 results. Where is this configured or configurable?
        if (num_completions_requested > 1000) console.log(
          "NOTE: Requested " + num_completions_requested + " completions"
          + ", but CompleteSearch will not return more than 1,000");
        _this.scroll_to_top = false;
        _this.query(_this.query_string, _this.round_id,
          num_completions_requested);
      }
    });

    // If config.launch_empty_query, fill the facet box for the empty query. Not
    // for the "word" facet because that is already taken care of by HitBox.
    //
    // TODO: config.launch_empty_query is a misnomer here. The empty query is
    // actually used for two things: (1) getting the total number of documents
    // in the collection; (2) filling the facet boxes. Note that there are
    // neither hits nor completions for the completely empty query.
    if (facet_name != "word" && config.launch_empty_query)
      this.scroll_to_top = true;
      this.query("", this.round_id);
  }

  // Launch facet query by prepending the facet word to the query string. If the
  // number of completions is not specified, we take the number from
  // config.completions_per_facet_box
  //
  // TODO: The HitBox class has no method query, is this asymmetry justified?
  query(query_string, round_id, 
          num_completions_requested = config.completions_per_facet_box) {
    // Each query gets a unique id, including facet queries. The round_id counts
    // the number of times, the search field has actually been changed. This is
    // currently only used for bookkeeping.
    //
    // NOTE: Do *not* use this.query_id here, since this is only the id of the
    // launched query. We use this.query_id for the id of the query, the content
    // of which is actually displayed in the box. It will be set by the method
    // display_completions
    global.num_queries += 1;
    var query_id = global.num_queries;
    this.round_id = round_id;

    // Append :facet:<facet_name>:* and get completions from the CompleteSearch
    // backend.
    //
    // NOTE: Don't do this when the query_string already ends with this word.
    // This is the case when reloading when the the box is scrolled to the
    // bottom. It's weird though and points to a suboptimal logic in the code.
    //
    // TODO 1: Assuming here that query_string is already rewritten, is that
    // correct?
    //
    // TODO 2: What about the selected facets, should they not be prepended here?
    var query_string_with_facets = query_string;
    if (this.facet_name != "word"
         && !query_string.endsWith(this.facet_word_prefix_with_star))
      query_string_with_facets +=
       (query_string.length > 0 ? " " : "") + this.facet_word_prefix_with_star;
    var url = global.base_url
                + "?q=" + query_string_with_facets
                + "&h=0"
                + "&c=" + num_completions_requested
                + "&rw=" + this.how_to_rank_completions
                + "&format=json";
    console.debug("URL for facet \"" + this.facet_name + "\": " + url);
    let _this = this;
    fetch(url)
      .then(response => response.json())
      .then(data => _this.update(data, query_id, _this.round_id))
      .catch(error => console.error("ERROR fetching result for facet \""
        + this.facet_name + "\" with " + url + ": " + error.message));
  }

  // Update from JSON returned by CompleteSearch backend. If the query id is
  // lower than the result already shown, the query is simply discarded. The
  // same is done for HitBox.update above
  update(json, query_id, round_id) {
    // Discard if old query.
    if (query_id < this.query_id) {
      console.debug("Received completions for Query #" + query_id
        + ", but completions for Query #" + this.query_id
        + " are already displayed ... discarding the former");
      return json;
    }
    this.query_id = query_id;
    this.round_id = round_id;
    console.info("Completions " + this.facet_name + " #" + this.round_id + "." + this.query_id
      + " in " + global.time_from_json(json) + ":", json);

    // Read meta data from JSON. Note that result.completions always exists (at
    // least it should) even if there are zero hits or the query was malformed.
    //
    // NOTE: there is also key "@computed", are we interested in that?
    // CompleteSearch does not always compute full results for efficiency
    // reasons.
    this.query_string = json.result.query;
    var completions = json.result.completions;
    this.num_completions_total = parseInt(completions["@total"]);
    this.num_completions_sent = parseInt(completions["@sent"]);

    // If completions["@sent"] is non-zero, there should be
    // json.result.completions.c with entries in the following format, where
    // "text" is // the actual completion:
    //
    //   { @sc: ..., @dc: ..., @oc: ..., @id: ..., text: ... }
    //
    // NOTE: If there is a single completion, CompleteSearch currently does not
    // return an array (TODO: which is just weird and should be fixed). For this
    // code, we then explicitly turn it into an array.
    console.assert("c" in completions || this.num_completions_sent == 0);
    this.completions = this.num_completions_sent == 0 ? [] : json.result.completions.c;
    if (!Array.isArray(this.completions)) this.completions = [ this.completions ];
    this.completions = this.completions.map(
      completion => [completion["text"],
                     completion[this.completion_scores_displayed_key]]);

    // Now we can display the completions in the corresponding facet box.
    this.display_completions();

    // This function is called from a .then (after the result was received from
    // the CompleteSearch backend), so we need to pass it on, in case there is
    // another .then following it.
    return json;
  }

  // Display completions in the box corresponding to the facet name.
  //
  // NOTE: This might be called after we selected a facet. It is therefore
  // crucial that we set the value of each checkbox according to the values in
  // global.facets_selected. Otherwise a selected facet will appear again in the
  // new result, but unselected.
  display_completions() {
    // Show the number of completions in the table footer. NOTE: The reason for
    // the &nbsp; is that the footer is otherwise more slim than for the facet
    // boxes with a footer text (although it has the same "height" attribute).
    var table_footer_html = this.num_completions_sent == 0 ? "&nbsp;"
      : "1 &minus; " + this.num_completions_sent + " of "
                     + this.num_completions_total.toLocaleString();
    $(this.table_selector + " tfoot td").html(table_footer_html);

    // Empty the table body. In the following, we will first append rows with
    // completions. If there were less than config.completions_per_facet_box
    // results, we will fill up with empty rows. In particular, this will fill
    // up the table with rows if there are no completions at all.
    $(this.table_selector + " tbody").html("");

    // Fill the rows with the received completions (as many as we have).
    let _this = this;
    this.completions.forEach(function(completion, i) {
      // Get the completion and its score. These were stored as arrays of size 2
      // in method update above.
      //
      // IMPORTANT: The "let" is crucial here because otherwise the functions
      // ("closures") passed to "change" below all have the same context and
      // "completion_text" will be the last value of the variable for all of
      // them. An alternative would be to use forEach instead of the for loop.
      // Update: we now use a for loop. Leaving the comment as a warning.
      let completion_text = completion[0];
      var completion_score = completion[1];

      // Build table row from i-th completion. Each row has a checkbox in the
      // left column, the completion in the middle column, and the completion
      // score in the right columns.
      var checkbox_td = "<td><input type=\"checkbox\"></td>";
      var completion_td = "<td>" + completion_text
                                     .replace(/.*:/, "")
                                     .replace(/_/g, " ") + "</td>";
      var score_td = "<td>" + completion_score + "</td>";
      var table_row = "<tr>" + checkbox_td + completion_td + score_td + "</tr>";
  
      // Add row to table body and compute corresponding selector. TODO: Can we
      // get that selector directly from jQuery as a return value?
      $(_this.table_selector + " tbody").append(table_row);
      var table_row_selector =
        _this.table_selector + " tbody tr:nth-child(" + (i + 1) + ")";
      // console.log("Table row selector: " + table_row_selector);

      // Make sure that the checkbox has the right status.
      if (global.facets_selected.has(completion_text))
        $(table_row_selector + " input").prop("checked", true);

      // Add action when selecting or unselecting this facet (either by clicking
      // in the checkbox or on the completion).
      // console.log("Input selector: " + table_row_selector + " input");
      $(table_row_selector + " input").change(() =>
        _this.facet_selection_changed(completion_text));
      $(table_row_selector + " td:nth-child(2)").click(() =>
        _this.facet_selection_changed(completion_text));
    });
 
    // If there were less than config.completions_per_facet_box completions,
    // fill up with empty rows until tbody height reached.
    //
    // NOTE: The reason for the hidden checkbox and the two &nbsp; is that we
    // want to make sure that the row takes up the exact same amount of space as
    // with normal contents. It looks slightly different otherwise (the rows are
    // slightly slimmer and slightly less wide).
    //
    // NEW: Add rows until scroll height > table body height. NOTE: We can't
    // take >= because the scroll height is always at leas the table body
    // height. TODO: How do we get the height of the rows so far?
    var row_height =
          parseFloat($(this.table_selector + " tbody tr") .css("height")) +
        + parseFloat($(this.table_selector).css("border-spacing").split(" ")[1]);
    // $(this.table_selector + " tbody").css("height", (4 * row_height) + "px");
    var tbody_height = parseFloat($(this.table_selector + " tbody").css("height"));
    // console.log("TBODY HEIGHT: ", tbody_height);
    var num_empty_rows = Math.max(Math.max(0, 5 - this.completions.length),
          Math.round(tbody_height / row_height - this.completions.length));
    // console.log("Border spacing: ", border_spacing);
    // console.log("Row height: ", row_height);
    // console.log("EMPTY ROWS: ", num_empty_rows);

    var empty_checkbox = "<input style=\"visibility: hidden\" type=checkbox>";
    var empty_table_row = "<tr><td>" + empty_checkbox
                            + "</td><td>&nbsp;</td><td>&nbsp;</td></tr>"
    // var num_empty_rows = Math.max(0,
    //       config.completions_per_facet_box - this.completions.length);
    $(this.table_selector + " tbody").append(empty_table_row.repeat(num_empty_rows));
    // console.log("FILL " + this.table_selector + ": " + num_empty_rows);
    // console.log("Scroll height BEFORE: ", $(this.table_selector + " tbody").prop("scrollHeight"));
    // console.log("Scroll height AFTER : ", $(this.table_selector + " tbody").prop("scrollHeight"));
    // console.log("Table body height   : ",
    //   Math.round(parseFloat($(this.table_selector + " tbody").css("height"))));

    // Set the dim status of this facet box: dimmed if no completions, undimmed
    // otherwise.
    this.dim_if_true_undim_if_false(this.num_completions_sent == 0);

    // NEW 04.02.2021: Set max-height of div, so that we cannot resize the box beyond
    // the actual contents.
    //
    // TODO: Reload completions (if still some left) instead.
    var max_height_in_px =
      $(this.table_selector + " tbody").prop("scrollHeight")
         + parseFloat($(this.table_selector + " thead").css("height"))
         + parseFloat($(this.table_selector + " tfoot").css("height"))
    $(this.div_selector).css("max-height", max_height_in_px.toString() + "px");

    // Scroll to top (if specified, reset to default afterwards).
    //
    // NOTE: when reloading hits after scrolling to the bottom, scroll_to_top is
    // explicitly set to false.
    if (this.scroll_to_top) $(this.table_selector + " tbody").scrollTop(0);
  }

  // Action when selecting a facet. The argument is the text of the completion.
  //
  // TODO: This is currently just reverting the status of the facet. Better make
  // sure that this in sync with the status of the checkbox. Otherwise, the user
  // might end up in a situation, where the checkbox shows one thing but the
  // results are actually for the opposite.
  facet_selection_changed(completion_text) {
    // Add the facet if it's not there, delete it if it's there. If it is
    // added, remove the query words that are contained in completion_text from
    // the query.
    var facet = completion_text;
    if (global.facets_selected.has(facet)) {
      global.facets_selected.delete(facet);
    } else {
      global.facets_selected.add(facet);
      // For non-word facets (completion_text starting with :facet:...):
      //
      // Remove query words that are contained in the displayed text (the part
      // after the :facet:...: prefix) from the query. Note that we do this
      // right in the input field since new Query() below reads the query from
      // that field (there is no global variable for the query string). 
      if (completion_text.startsWith(":facet:")) {
        const completion_text_suffix = completion_text.replace(/^.*:/, "");
        const query_parts_before = $("div#search input")
                                     .val().split(/\s+/).filter(s => s);
        console.log("Completion text:", completion_text);
        // console.log("Completion text:", completion_text_suffix);
        // console.log("Query parts before:", query_parts_before);
        var query_parts_after = [];
        query_parts_before.forEach(function(query_part) {
          if (!completion_text_suffix.match(new RegExp(query_part, "i")))
            query_parts_after.push(query_part);
        });
        // console.log("Query parts after:", query_parts_after);
        $("div#search input").val(query_parts_after.join(" "));
      }
    }
    console.info("Facet selection is now: ", global.facets_selected);

    // Reset first hit (we are getting new results now). TODO: shouldn't this
    // better be handled by the Query class?
    global.hit_box.first_hit = 0;

    // Launch new query. Note that the prepending of the facets from
    // global.facets_selected is handled by the Query class automatically when
    // constructor is called without arguments. TODO: this is cryptic syntax,
    // improve it.
    (new Query()).get_result();
  }

  // Dim or undim this facet box.
  dim_if_true_undim_if_false(should_be_dimmed) {
    console.debug(this.facet_name
      + " dim_if_true_undim_if_false: ", should_be_dimmed);
    // If a facet box remains undimmed (the normal case), don't to anything.
    // Note that we cannot simply do nothing when it remains dimmed because the
    // facet box for words is sometimes updated twice for the same query (in
    // particular, when query_string is empty).
    if (!this.should_be_dimmed && this.should_be_dimmed == this.is_dimmed)
      return
    this.is_dimmed = should_be_dimmed

    // NEW: Instead of dimming, try hiding the box.
    //
    // NOTE: This makes the dimming in the code afterwas without effect (when
    // the box is hidden from view, it doesn't matter whether it's dimmed or
    // not).
    if (this.is_dimmed) {
      // console.log("Hide:", this.div_selector);
      $(this.div_selector).hide();
    } else {
      // console.log("Show:", this.div_selector);
      $(this.div_selector).show();
    }

    // If dimmed, background color of the table are more similar to the
    // background of the containing div instead of white. This can be changed in
    // the config.
    var words_box_bg = should_be_dimmed
      ? config.background_facets_faded : config.background_facets;
    var words_box_fg = should_be_dimmed
      ? config.foreground_facets_faded : config.foreground_facets;
    $(this.table_selector + " td").css("background-color", words_box_bg);
    $(this.table_selector + " th").css("background-color", words_box_bg);
    $(this.table_selector + " th").css("color", words_box_fg);
    // If dimmed, hide the scrollbar. Otherwise, it's shown.
    $(this.table_selector + " tbody").css("overflow",
      should_be_dimmed ? "hidden" : "auto");
  }
}

// A query for the CompleteSearch backend.
class Query {

  // New query. The default is to take the query from the input field and
  // prepend the selected facets. Only when a query string is given, do we take
  // exactly that query string.
  //
  // TODO: The API here is weird. In particular it's weird that in the rest of
  // the code we always do (new Query(...)).get_result and never use the actual
  // query object. It's also weird that we have a method "query" in the class
  // FacetBox (which is called when we select a facet).
  //
  // Maybe we should have this as a method query of the class HitBox. This would
  // also make these two classes more consistent. Then the only weird thing
  // would be that HitBox.query would update the contents of the word facet box.
  // But that weirdness is actually due to a disparity between the
  // CompleteSearch backend (which just knows queries, each of which has hits and
  // completions) and the UI (which has one hit box and several facet boxes).
  //
  // So maybe it is right to have a "dispatcher" class like this one, which then
  // calls HitBox.update and FacetBox.update for the various facets. Maybe it
  // should simply not be called "Query", but something more appropriate. And it
  // should probably be a member of Global, created once and then re-used for each
  // query.
  //
  // Also, it's probably cleaner to have one query only for the hits (with URL
  // parameter c=0) and one query only for the completions (with URL parameter h
  // = 0). Then we send two queries for information which we could have obtained
  // via just one query, but maybe that is not a problem and actually worth it.
  constructor(query_string = null) {

    // TODO: These should be global variables. Which they actually were before.
    // Actually, the whole Query class should be somehow "global", see the
    // comment above.
    this.PAGE_UP = 34;
    this.PAGE_DOWN = 33;

    this.query_string = query_string;
    this.query_id = global.num_queries + 1;
    this.round_id = global.num_rounds + 1;
    global.num_queries += 1;
    global.num_rounds += 1;

    // If called without query string, we take the (rewritten) query string
    // from the input field and prepend the index words for the selected facets.
    //
    // TODO: Who calls this with a query string? It seem to me: no one anymore,
    // since FacetBox has its own query method (which also consults and
    // increases global.num_queries).
    if (this.query_string == null) {

      // Get value of input field.
      this.query_string = $("div#search input").val();

      // If the config wants it, replace empty query by predefined query.
      if (this.query_string == ""
            && config.replacement_for_empty_query != undefined) {
        this.query_string = config.replacement_for_empty_query;
      }

      // Add a * after every query part that has length at least
      // config.min_prefix_length_to_append_star.
      var regex = new RegExp("(" + global.word_char_regex.repeat(
        Math.max(config.min_prefix_length_to_append_star, 1))
          + ")($|"+ global.sep_char_regex + ")", "g");
      this.query_string = this.query_string.replace(regex, "$1*$2");
      // No * for words with a $ in the end.
      this.query_string = this.query_string.replace(/\$\*/g, "");
      // No * for words that consist only of digits.
      this.query_string = this.query_string.replace(/\b(\d+)\*/g, "$1");
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
      // NOTE: Important to normalize only the query string because the
      // normalization also converts to lower case and the facet words often
      // contain letters in upper case.
      //
      // TODO: It would be easy to add a special :info: word for this. It
      // facetids are available, we should always use them, otherwise we can't.
      var facetid_prefix = config.facetids_available ? ":facetid:" : ":facet:"
      this.query_string = Array.from(global.facets_selected)
            .map(facet_word => "\""
                  + facet_word.replace(/^:facet:/, facetid_prefix)
                  + "\"").join(" ")
              + " " + this.rewrite_query(this.query_string);

      // Remove leading space if any (may have been added by the above).
      this.query_string = this.query_string.trim();
    }

    console.log("\nQuery #" + this.query_id + ": \"" + this.query_string + "\"");
  }

  // Rewrite query.
  // 
  // For now: memove whitespace in the beginning and compress any sequence of
  // whitespaces to a single whitespace and any sequences of more than one *
  // into a single *.
  rewrite_query(query_string) {
    if (config.rewrite_query != undefined) {
      query_string = config.rewrite_query(query_string);
      // console.log("Query string rewritten: ", query_string);
    }
    var leading_sep_chars = new RegExp("^" + global.sep_char_regex + "+");
    var trailing_sep_chars = new RegExp(global.sep_char_regex + "+($| )");
    query_string = query_string
      .replace(/\s+/g, " ")             // single spaces only
      .replace(/-/g, ".")               // Treat - like . (phrase query)
      .replace(leading_sep_chars, "")   // clip leading sep chars
      .replace(trailing_sep_chars, "")  // clip trailing sep chars
      .replace(/\*+$/, "*")             // at most one * 
      .toLowerCase();
    // console.log("REWRITTEN query string: ", query_string);
    return query_string;
  }

  // Send to backend and get result.
  //
  // NOTE: The key_code is for the case that this call was triggered by a typing
  // action in the search field. This is useful to process actions like PageUp
  // or PageDown, used for paging. This function also gets called when selecting
  // or unselecting a facet. The we call it without argument, hence the default
  // of null.
  get_result(event = null) {
    var key_code = event ? event.keyCode : null;
    var ctrl_pressed = event ? event.ctrlKey : false;
    var shift_pressed = event ? event.shiftKey : false;
    var alt_pressed = event ? event.altKey : false;
    console.log("Key code: " + key_code + " (alt = " + alt_pressed + ")");

    // Result paging with PAGE_UP and PAGE_DOWN.
    // if (key_code == this.PAGE_UP) global.first_hit += config.hits_per_page;
    // if (key_code == this.PAGE_DOWN) global.first_hit -= config.hits_per_page;
    if (global.first_hit + config.hits_per_page > global.last_total)
      global.first_hit = global.last_total - config.hits_per_page;
    if (global.first_hit < 0) global.first_hit = 0;

    // Controlling the excerpt radius with ALT-m (77) and ALT-l (76).
    if (key_code == 77 && alt_pressed) global.excerpt_radius *= 2;
    if (key_code == 76 && alt_pressed) global.excerpt_radius /= 2;
    global.excerpt_radius = Math.min(1024, Math.max(2, global.excerpt_radius));
    if (key_code == 77 && alt_pressed) console.log("Increased exerpt radius");
    if (key_code == 76 && alt_pressed) console.log("Decreased exerpt radius");

    // If the query_string was changed, we need to reset the facets and the
    // first hit, but only then! We have to pay attention that we do not reset
    // when we page or when we added or removed a facet.
    this.query_has_changed = key_code != null
      && key_code != this.PAGE_UP && key_code != this.PAGE_DOWN;
    if (this.query_has_changed) {
      // global.facets_selected.clear();
      global.first_hit = 0;
    }

    // If the query is too short according to the config, we don't launch a
    // query. Exception: the empty query.
    var regex = new RegExp("[^ ,;.-]".repeat(
      config.min_prefix_length_to_launch_query) + "[\"*]?$");
    if (this.query_string.length > 0 && !this.query_string.match(regex)) {
      console.log("No query launched, last query word of "
        + this.query_string + " is shorter than "
        + config.min_prefix_length_to_launch_query
        + " (can be configured in config.js)");
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
    //
    // TODO: In FacetBox.query we have the exact same code to compute
    // how_to_rank_completions, that points to a suboptimal design.
    var how_to_rank_hits = config.how_to_rank_hits;
    var how_to_rank_completions = config.how_to_rank_completions["word"]
          || config.how_to_rank_completions["default"] || "1d";

    // Get hits and completions.
    var url = global.base_url
                + "?q=" + this.query_string
                + "&h=" + config.hits_per_page
                + "&c=" + num_completions
                + "&er=" + global.excerpt_radius
                + "&rd=" + how_to_rank_hits
                + "&rw=" + how_to_rank_completions
                + "&f=" + global.first_hit
                + "&format=json";
    console.debug("URL for hits and completions: " + url);
    // TODO: Do we need let here instead of var? Maybe yes, when several queries
    // are executed and the "then" parts interleave.
    let _this = this;
    if (global.hit_box) global.hit_box.scroll_to_top = true;
    if (global.facet_boxes["word"]) global.facet_boxes["word"].scroll_to_top = true;
    fetch(url)
      .then(response => response.json())
      .then(json => global.hit_box.update(
                      json, _this.query_id, _this.round_id))
      .then(json => global.facet_boxes["word"].update(
                       json, _this.query_id, _this.round_id))
      // NOTE: An earlier version of the code printed only error.message which
      // omits the location of the error in the code, which makes it extremely
      // hard to locate the error.
      .catch(error => console.error("ERROR fetching result for "
        + "hits and completions query with " + url + ": ", error));

    // Launch query for each other facet ("word" was already dealt with above).
    global.facet_names.forEach(function(facet_name) {
      global.facet_boxes[facet_name].scroll_to_top = true;
      global.facet_boxes[facet_name].query(_this.query_string, _this.round_id);
    });

    // Make sure the focus is in the search field again (we may have clicked on
    // a facet and lost it).
    $("div#search input").focus();
  }
}


// MAIN CODE for the web page. Look how simple it is :-)
$(document).ready(function() {

  // Initialize global variables and set colors according to config.js
  global = new Global();

  // Create the hit box and the facet boxes and initialize them. This also fills
  // the facet boxes for the empty query if config.launch_empty_query
  global.initialize_boxes();

  // There are three places in the code, where a FULL query (hits and
  // completions) is launched: initially (below), when something is typed
  // (further below), or when a facet in one of the boxes is selected
  // (FacetBox.facet_selection_changed -> maybe better move that function to the
  // class Query?).
 
  // Launch empty query (checks config.replacement_for_empty_query and only does
  // something if it is defined).
  var query = new Query();
  query.get_result();

  // Action when something happens in the search field.
  $("div#search input").focus();
  $("div#search input").keyup(function(event) {
    var query = new Query();
    query.get_result(event);
  });
});
