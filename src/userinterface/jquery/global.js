var urlBase = "";
var filterPrefix = ":filter:";
var facetPrefix = ":facet:";
var config = new Config();
var query = new Query(config.facets, facetPrefix, filterPrefix);
var result = new Result(config.facets, facetPrefix, filterPrefix);
var messages = new Messages();

//! Stores current num of completions for each facet box.
var numOfCompletionsArray = [];
//! Time to wait for checking for the next keystroke. This is done to avoid
//! sending requests for each typed letter.
var delay = 200;
//! Stores if the button is pressed right now.
var ctrlDown = false;
var altDown = false;
var fnKeyDown = false;
//! Stores which item is selected if the facetboxes are gone through by using
//! the arrow keys.
var selectedQuery = -1;

//! Width of the scrollbar. Helps to compute correct box sizes.
var scrollbarWidth;

//! Selectors
var idQuery = "#query";
var idApisSelector = "#apis select";
var idApis = "#apis";
var idErrors = "#errors";
var idReset = "#reset";
var idHits = "#hitsContent";
var idHitsContainer = "#hits";
var idHeader = "#headerContent";
var idHitsAndCompletions = "#results";
var idFboxesContainer = "#fboxes";
var classFboxes = ".fbox";
var classFboxContent = ".fboxContent";
var classFboxTitle = ".fboxTitle";
var classFboxInfo = ".fboxInfo";

//! Initialize page.
$(document).ready(function() {
  init();
  addEventHandlers(); 
});

//! Initialize.
function init() {
  // For each facet generate a facet box.
  jQuery.each(config.facets, function(id) {
    // Fbox.
    $('<div/>',{
        'class' : classFboxes.substr(1),
        'id'    : id,
    }).appendTo(idFboxesContainer);
    // Title div of the fbox.
    $('<span/>', {
        'class'    : classFboxTitle.substr(1),
        'id'       : id + "Title"
    }).appendTo('#' + id);
    // Content div of fbox.
    $('<div/>', {
        'class'    : classFboxContent.substr(1),
        'id'       : id + "Content"
    }).appendTo('#' + id);
    // Info div of fbox.
    $('<span/>', {
        'class'    : classFboxInfo.substr(1),
        'id'       : id + "Info"
    }).appendTo('#' + id);
    // Hide at the beginning, since no content is available.
    $(classFboxes).hide();
    // Set the default num of completions, which should be requested.
    numOfCompletionsArray[id] = config.numOfCompletionsDefault;
  });

  // Generate api select options.
  if (config.apis.length > 0)
  {
    $('<option/>', {
      'text': "Get result as ...",
      'disabled': true,
      'selected': true,
      'defaultSelected': true,
    }).appendTo(idApis);
    jQuery.each(config.apis, function(i) {
      $(idApis).append(new Option(config.apis[i]));;
    });
  }

  // Add default text to hit container.
  $(idHits).html(messages.printInfo("INSTRUCTIONS", "en"));

  // Specify urlBase.
  urlBase = window.location.protocol+"//"+window.location.host;

  // Compute width of scrollbar.
  scrollbarWidth = getScrollbarWidth();

  // If the url already specifies a query by using a hash (#q=queryString), search for it.
  loadQueryFromHash();

  // The width of the hits container is computed according the the width of the
  // fbox container.
  recomputeHitsContainerWidth();
}

//! Extract query from hash and load it.
function loadQueryFromHash() {
  var hash = window.location.hash;
  // Query is everything but #q=
  query.newQuery = hash.substr(3);
  // Put query in input field.
  $(idQuery).val(query.newQuery);
  // Focus to input field.
  $(idQuery).focus();
  // Look up query.
  lookupQuery(query.queriesToLoad_onClick);
}

//! Event handler.
function addEventHandlers() {
  // HISTORY
  $(window).hashchange(function(e) {
    // Load the query given by the hash.
    loadQueryFromHash();
  });

  // RESET button
  $(idReset).bind("click", function(e) {
    removeHash();
    window.location.reload();
  });

  // WINDOW
  $(window).resize(function(event) {
    // TODO: Forgot to note, what exactly happens, if I don't block the event.
    // Find out!
    if ($(event.target).hasClass('ui-resizable'))
      return;
    recomputeHitsContainerWidth();
  });

  // QUERY INPUT
  $(idQuery).bind({
    'keyup': function(e) {
      var code = (e.keyCode ? e.keyCode : e.which);
      // Non of the special keys is pressed (anymore).
      if (code == 9 || (code >= 16 && code <=45)) {
        fnKeyDown = false;
        if (code == 17) ctrlDown = false;
      // On <Return>
      } else if (code == 13) {
        // Get new query.
        query.newQuery = $(idQuery).val();
        // Set hash to new query. If it's empty, clear hash.
        if (query.newQuery.length != 0) window.location.hash = 'q=' + query.newQuery;
        else removeHash();
        // Look up query.
        lookupQuery(query.queriesToLoad_onClick);
        // TODO: Why should we do this here? Uncomment it. Maybe I am going to
        // notice the reason.
        // e.preventDefault();
      // Relookup the query. 
      } else {
        // TODO Why triggering this event? Uncommented it.
        // $(idQuery).change();
        // Get query from input.
        query.newQuery = $(idQuery).val();
        // Check for a change of the query every <delay> milliseconds. We don't
        // want to handle every key stroke.
        window.setTimeout(function() {
          var queryTmp = $(idQuery).val();
          // Nothing changed.
          if (query.newQuery != queryTmp) return false;
          // Query changed. Look it up.
          query.newQuery = queryTmp;
          lookupQuery(query.queriesToLoad_whileTyping);
        }, delay);
      }
    },
    // Capture id pressings like ctrl.
    'keydown': function(e) {
      var code = (e.keyCode ? e.keyCode : e.which);
      // functional keys, like strg, alt, shift, tab, etc.
      if (code == 9 || (code >= 16 && code <=45))
      {
        fnKeyDown = true;
        if (code == 17) ctrlDown = true;
        // down arrow: Go through facet boxes (downward)
        if (code == 40 && result.completionsList.length > selectedQuery + 1)
        {
          selectedQuery += 1;
          $(idQuery).val(result.completionsList[selectedQuery]);
        }
        // up arrow: Go throught facet boxes (upward)
        if (code == 38 && selectedQuery != -1)
        {
          selectedQuery -= 1;
          $(idQuery).val(result.completionsList[selectedQuery]);
        }
      }
    }});

  // SELECT API (xml, json, jsonp)
  $(idApis).live("change", function (selected) {
    // Get selected option.
    var format = this.options[this.selectedIndex].text;
    // Open url.
    var url = "http://" + window.location.host + "/?q=" + query.newQuery + "&format=" + format + "&h=" + query.queriesToLoad_onClick;
    window.open(url, query.newQuery + " - " + format);
    // Deselect option.
    $(this).val($(this).find('option[selected]').val());
  });

  // HITS scroll
  $(idHitsContainer).bind({
    'mousewheel DOMMouseScroll scroll': function(e) {
      // If we are near the bottom of the hits container and we are scrolling
      // downwards, load more content.
      if (scrolledToTheBottom(e, $(this), 50) && getScrollingDirection(e) < 0)
        loadContent();
    }
  });

  // FBOXES scroll
  $(classFboxContent).bind({
    'mousewheel DOMMouseScroll scroll': function(e) {
      // If we are at the bottom of the fbox div and we are scrolling
      // downwards, load more content.
      var scrollingDirectiion = getScrollingDirection(e);
      if (scrolledToTheBottom(e, $(this), 0) && scrollingDirectiion < 0) {
        // Get the id of the content of the scrolled fbox.
        var id = $(this).attr('id');
        id = id.substr(0, id.length - "Content".length);
        // If not all completions were already loaded, load 10 more.
        if (!result.facets[id].allCompletionsLoaded) {
          numOfCompletionsArray[id] += 10;
          var params = {
           "q": query.generateFacetQuery(id),
           "h": 0,
           "c": numOfCompletionsArray[id],
           "format": "json"
          }
          result.print(urlBase, params, "completions", id, result.requestNo, false);
        }
        // Do this to prevent the fbox container to scroll too.
        e.preventDefault();
        e.stopPropagation();
      // If scrolled to the bottom, do nothing. Otherwise this would cause the
      // facetbox container to scroll.
      } else if ($(this).scrollTop() == 0 && scrollingDirectiion > 0) {
        e.preventDefault();
        e.stopPropagation();
      }
    }
  });

  // FBOXES resize
  $(classFboxes).resizable({
    // Allow resizing to the east and south.
    handles: "s, e",
    // While resizing.
    resize: function (event, ui) {
      // East
      // If the ui width changed.
      if (ui.originalSize.width != ui.size.width) {
        // Set width of the fboxContainer to the new width too.
        $(idFboxesContainer).css("width", ui.size.width + "px");
        // Reset the size of the fboxes by including the reserved size of the scrollbar.
        $(classFboxes).css("width", ui.size.width - 2 * scrollbarWidth + "px");
        // Recompute size of the hits container.
        recomputeHitsContainerWidth();
        // Recompute the height of the fboxes.
        jQuery.each(config.facets, function(id) {
          recomputeFboxSize(id);
        });
      // South
      } else {
        var id = $(this).attr('id');
        // Compute how many items fit into the content div.
        var numOfCompletions = computeNumOfCompletions(id);
        // Get the current items.
        var listItems = $('#' + id + "Content li");
        // If the content div got smaller, erase the items from the list.
        if (numOfCompletions < listItems.length)
        {
          // Create new list with the first numOfCompletions-th items.
          var newhtml = "<ul>";
          listItems.each(function(i, item) {
            var content = $(item).html();
            if (i < numOfCompletions)
            {
              newhtml += "<li>" + content + "</li>";
            }
          });
          newhtml += "</ul>";
          $('#' + id + "Content").html(newhtml);
          // Replace info message.
          var oldInfoHtml = $('#' + id + "Info").html();
          var regex = /1 - \d+ of/;
          $('#' + id + "Info").html(oldInfoHtml.replace(regex, "1 - " + numOfCompletions + " of"));
          // It's obvious that not all completions are loaded.
          result.facets[id].allCompletionsLoaded = false;
        }
      }
    },
    // On stop.
    stop: function(event, ui) {
      var id = $(this).attr('id');
      // If the fbox got bigger, recompute the items.
      // TODO: find out, where numOfCompletionsArray[id] is set to the correct
      // size.
      if (ui.originalSize.height < ui.size.height) {
        var params = {
          "q": query.generateFacetQuery(id),
          "h": 0,
          "c": numOfCompletionsArray[id],
          "format": "json"
        }
        result.print(urlBase, params, "completions", id, result.requestNo, true);
      // If the fbox got smaller, adjust the size.
      } else if (ui.originalSize.height > ui.size.height) {
        recomputeFboxSize(id);
      }
    }
  });

  // Make fboxes sortable.
  $(idFboxesContainer).sortable({
    items: classFboxes,
    // Don't allow a dragging on the fbox content div.
    cancel: classFboxContent
  });
}

function lookupQuery(numOfHits)
{
  if (query.lastQuery == query.newQuery && numOfHits == result.hitsLoaded) return true;
  $(errors).empty();
  query.lastQuery = query.newQuery;

  var words = query.newQuery.split(" ");
  for (var i = 0; i < words.length; i++) {
    if (words[i].length < 3 && words[i].length > 0) return;
  }
  result.requestNo++;

  query.newQuery = query.convertQuery();
  var hitParameters = "&h=" + numOfHits + "&" + config.param4html + "&c=0";
  var facetParameters = "&h=0";
  result.completionsList = [];
  // Hits
  if (query.newQuery.length != 0)
  {
    var hitParams = {
      "q": query.newQuery,
      "h": numOfHits,
      "c": 0,
      "format": "json"
    }
    hitParams = $.extend(config.param4html, hitParams)
    result.print(urlBase, hitParams, "hits", idHits, result.requestNo, false);
    $(idApis).show();
  }
  else
  {
    $(idHits).html(messages.printInfo("INSTRUCTIONS", "en"));
    $(idApis).hide();
  }
  // Facet Boxes
  jQuery.each(config.facets, function(id){
    numOfCompletions = numOfCompletionsArray[id];
    var facetQuery = query.generateFacetQuery(id);
    var facetParams = {
      "q": facetQuery,
      "h": 0,
      "c": numOfCompletions,
      "format": "json"
    }
    if (facetQuery.length != 0)
    {
      result.print(urlBase, facetParams, "completions", id, result.requestNo, true);
    }
    else
      $("#" + id).hide();
  });
}

function getScrollingDirection(e) {
  // Stores whether it's scrolled downwards or upwards. This is detected
  // differently depending on the browser.
  if (e.type == 'mousewheel') {
    return -1 * e.originalEvent.deltaY;
  } if (e.type == 'DOMMouseScroll') {
    return -1 * e.originalEvent.detail;
  }
}

function scrolledToTheBottom(e, element, offset) {
  // If scrolled to the bottom.
  if (element.scrollTop() + element.innerHeight() >= element[0].scrollHeight - offset)
     return true;
  return false;
}

function reloadCompletions(id) {
  //numOfCompletionsArray[id] += 10;
}

function reloadHits(id) {
  var hitParams = {
    "q": query.newQuery,
    "h": query.queriesToLoad_onClick,
    "f": result.hitsLoaded,
    "c": 0,
    "format": "json"
  }
  hitParams = $.extend(config.param4html, hitParams)
  result.print(urlBase, hitParams, "hits", idHits, result.requestNo, false);
}

function computeNumOfCompletions(id)
{
  var elementHeight = $("#" + id).innerHeight()
                      - $("#" + id + "Title").outerHeight(true);
                      - $("#" + id + "Info").outerHeight(true);
                      - $(".ui-resizable-s").outerHeight(true);
  var lineHeight = $('li').outerHeight(true);
  if (elementHeight < lineHeight) return config.numOfCompletionsDefault;
  if (!elementHeight || elementHeight == null || elementHeight == 0) return config.numOfCompletionsDefault;
  if (!lineHeight || lineHeight == null || lineHeight == 0) return config.numOfCompletionsDefault;
  var num = Math.floor(elementHeight / lineHeight);
  if (num < config.numOfCompletionsDefault) return config.numOfCompletionsDefault;
  numOfCompletionsArray[id] = num;
  return num;
}

function callFacetLink(word)
{
  if (ctrlDown) word = "-" + word;
  var wordLower = word.toLowerCase();
  if (query.lastQuery.length == 0)
    window.location.hash = 'q=' + word.replace(' ', '_');
  else if (query.lastQuery.toLowerCase().search(wordLower) != -1)
    window.location.hash = 'q='+ query.lastQuery;
  else {
    var queryWords = query.lastQuery.split(" ");
    jQuery.each(queryWords, function(i, queryWord) {
      if (wordLower.search(queryWord.toLowerCase()) != -1) {
        queryWords.splice(i, 1);
        return false;
      }
    });
    window.location.hash = 'q='+ (queryWords.join(" ") + " " + word.replace(' ', '_')).trim();
  }
  return false;
}

function callFuzzyLink(word)
{
  window.location.hash = 'q='+ word.trim();
  return false;
}
function recomputeFboxSize(id)
{
  var fbox = jQuery("#" + id);
  var contentDiv = jQuery("#" + id + "Content");
  var itemList = jQuery("#" + id + "Content > ul");
  var numOfExistingItems = jQuery("#" + id + "Content > ul > li").size();
  if (numOfExistingItems < numOfCompletionsArray[id])
    numOfCompletionsArray[id] = config.numOfCompletionsDefault;
  // Recompute fbox size.
  //  - if content div > itemList, minimize content div and item list
  //  - if fbox div big enough to hold itemList, maximise content div.
  if (contentDiv.outerHeight(true) >= itemList.outerHeight(true)
      || itemList.outerHeight(true) < fbox.innerHeight()
      || numOfCompletionsArray[id] <= config.numOfCompletionsDefault) {
    // Update content div size. Since This does minimize the height of the
    // content div.
    contentDiv.innerHeight(itemList.outerHeight(true)
                         + jQuery(".ui-resizable-s").outerHeight(true));
    // Update fbox div size.
    fbox.innerHeight(contentDiv.outerHeight(true)
                               + jQuery("#" + id + "Title").outerHeight(true)
                               + jQuery("#" + id + "Info").outerHeight(true));
  }

  // Hide scrollbar if all completions are loaded and the content div already
  // shows all items.
  if (config.facets[id].allCompletionsLoaded && contentDiv.height() >= itemList.height())
    contentDiv.css("overflow-y", "hidden");
  // Otherwhise show scrollbar.
  else 
    contentDiv.css("overflow-y", "scroll");
}

function loadContent() {
  var params = {
      "q": query.newQuery,
      "h": query.queriesToLoad_onClick,
      "f": result.hitsLoaded,
      "c": 0,
      "format": "json"
  }
  params = $.extend(params, config.param4html)

  if (query.newQuery.length != 0)
    result.print(urlBase, params, "hits", idHits);
}

function removeHash () { 
    var scrollV, scrollH, loc = window.location;
    if ("pushState" in history)
        history.pushState("", document.title, loc.pathname + loc.search);
    else {
        // Prevent scrolling by storing the page's current scroll offset
        scrollV = document.body.scrollTop;
        scrollH = document.body.scrollLeft;

        loc.hash = "";

        // Restore the scroll offset, should be flicker free
        document.body.scrollTop = scrollV;
        document.body.scrollLeft = scrollH;
    }
}

function updateScrollbar(id) {
  $(id).mCustomScrollbar("update");
}

(function($) {
  $.fn.hasScrollBar = function() {
    return this.get(0).scrollHeight > this.get(0).clientHeight;
  }
})(jQuery);

function stopEvent(e){
  if(!e){ /* IE7, IE8, Chrome, Safari */ 
    e = window.event; 
          }
  if(e.preventDefault) { /* Chrome, Safari, Firefox */ 
    e.preventDefault(); 
  } 
  e.returnValue = false; /* IE7, IE8 */
}

var getScrollbarWidth = function() {
  var div, width = getScrollbarWidth.width;
  if (width === undefined) {
    div = document.createElement('div');
    div.innerHTML = '<div style="width:50px;height:50px;position:absolute;left:-50px;top:-50px;overflow:auto;"><div style="width:1px;height:100px;"></div></div>';
    div = div.firstChild;
    document.body.appendChild(div);
    width = getScrollbarWidth.width = div.offsetWidth - div.clientWidth;
    document.body.removeChild(div);
  }
  return width;
};

function recomputeHitsContainerWidth() {
  $(idHitsContainer).width($(idHitsAndCompletions).innerWidth() - $(idFboxesContainer).outerWidth(true));
}


