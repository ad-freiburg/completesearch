// Class holding the configuration of the UI for a particular CompleteSearch
// instance
//
// The one thing one almost certainly has to adapt is this.hit_to_html because
// that displays the hits and that depends very much on the contents of these
// hits, which depend on the data and how it was indexed.
//
// All the other options have reasonable default values. But, of course, they
// can be freely adapted. 
//
class Config {
  constructor() {
    // HTML for a hit. The variable "hit" has the following properties:
    //
    //   "excerpt": the excerpts from the fields indexed as full text
    //   "@sc": the score of the hit (can be configured via the backend)
    //   "@id": the id of the document (can be useful for ordering)
    //
    //   all fields specified via the --show option of the CSV parser
    //
    this.hit_to_html = function(hit) {
      return "<div class=\"hit\">"
               + "<p class=\"title\"><a href=\"" + hit.wikipedia + "\" target=\"_blank\">"
                    + hit.film + " (" + hit.date.replace(/-.*$/, "") + ")</a></p>"
               + "<p class=\"subtitle\">" + hit.director.join(", ") + "</p>"
               + "<p class=\"excerpt\">" + hit.excerpt + "</p>"
               + "</div>";
    };

    // Number of hits per page.
    this.hits_per_page = 7;

    // Number of completions in a facet box.
    this.completions_per_facet_box = 10;

    // How many words to show around each match in excerpt.
    this.excerpt_radius = 20;

    // How to rank hits and completions. A single digit followed by the letter
    // 'a' (ascending) or 'd' (descending). For example, 1a or 5d. The number in
    // parantheses is the name of the attribute in the JSON result, see code. I
    // am not sure what 5 and 6 mean when ranking hits.
    //
    //  0   rank hits by score (@score)
    //  1   rank hits by doc id (@id)
    //  2   rank hits by word id
    //  5   rank hits by completio scores ???
    //  6   group docs by word id ???
    //  7   rank docs by combination of score and aggregated edit distance
    //
    // How to rank hits (@score or @id). The first letter specified the ordering
    // ("a" = ascending, "d" = descending).
    this.how_to_rank_hits = "0d";

    // How to rank completions, see the explanations in the comment above. This
    // is a dictionary with one value per facet, where the word facet has key
    // "word". The value for key "default" is given to all facets that are not
    // explicitly specified. If a key is specified for which no facet exists,
    // that entry has simply no effect. For example:
    //
    //  { "default": "1d", "year": "4d", "has_no_effect": "87x" }
    //
    //  0   rank words by score (@sc)
    //  1   rank words by count of docs containing them (@dc)
    //  2   rank words by count of occurrences in those docs (@oc)
    //  3   rank words by word id (@id)
    //  4   rank words by document id
    //  5   rank words by edit distance
    //
    // Concerning option 4 (@id): with the --ordering option, the CSV Parser can
    // produce words from dates or quantities, where the lexical order
    // corresponds to the order of the dates or quantities. See
    // https://ad-wiki.informatik.uni-freiburg.de/completesearch/CsvParser
    this.how_to_rank_completions = { "default": "2d", "year": "4a" };

    // NOTE: There is no variable for the hit score. Both "@sc" and "@id" are
    // available as keys of "hit" in hit_to_html above. It's up to the
    // configuration to make use of it or not.

    // Which scores to show for each facet box (see above, one of: @sc, @dc,
    // @oc, @id).
    this.completion_scores_displayed = { "default": "@dc" };

    // User-defined facets (will be preferred to automatic ones).
    // this.facet_names = ["author", "venue", "year"];
    // this.facet_names = ["author", "venue", "year"];

    // Launch empty query on startup?
    this.launch_empty_query = true;

    // Only launch a query when the last word has at least these many
    // characters.
    this.min_prefix_length_to_launch_query = 1;

    // Only append a * when the last word has at least these many characters.
    this.min_prefix_length_to_append_star = 3;

    // Are facet ids available in the index? Crucial for efficiency for last
    // facets (that is, facets contained by many millions of documents).
    //
    // TODO: We could just ask the index to complete ":facetid:*". If we get a
    // completion, the facetid words are there.
    this.facetids_available = true;

    // Background colors of the three parts of the screen (search field, facets,
    // results). Faded color of inactive word facet box.
    this.background_facets = "F4F4F4";
    this.background_facets_faded = "#FEFEFE";
    this.foreground_facets = "black";
    this.foreground_facets_faded = "lightgray";
  }
}
