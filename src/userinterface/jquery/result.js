function Result(facets, facetPrefix, filterPrefix)
{
  this.facets = facets;
  this.facetPrefix = facetPrefix;
  this.filterPrefix = filterPrefix;
  this.hitsLoaded = 0;
  this.requestNo = 0;
  this.completionsList = [];
}

// If num == -1 all completions should be printed.
Result.prototype.getCompletions = function(result, id, ajaxTime)
{
  var self = this;
  var completions = result.result.completions.c;
  var completionsTotal = result.result.completions["@total"];
  var completionsSent = result.result.completions["@sent"];
  var resultDiv = jQuery("#" + id + "Content");
  var resultTitle = jQuery("#" + id + "Title");
  var resultInfo = jQuery("#" + id + "Info");
  if (typeof completions == 'undefined')
  {
    $("#" + id).hide();
    return false;
  }
  var html = "";
  if (typeof completions.length == 'undefined')
  {
    html += self.generateFacetBoxEntry(completions, id);
  }
  else
  {
    jQuery.each(completions, function(i) {
      html += self.generateFacetBoxEntry(completions[i], id);
    });
  }
  if (html.length > 0)
  {
    var time = Math.round(result.result.time.text);
    ajaxTime = Math.round(ajaxTime - time);
    html = "<ul>" + html + "</ul>";
    $(resultDiv).html(html);
    $(resultTitle).html("Refine by " + this.facets[id].title.toUpperCase() + "<br>searched " + time + " ms (+ " + ajaxTime + " ms waiting)");
    $(resultInfo).html("1 - " + completionsSent + " of " + completionsTotal);
    if (completionsSent == completionsTotal) this.facets[id].allCompletionsLoaded = true;
    else this.facets[id].allCompletionsLoaded = false;

    if ($("#" + id).is(":hidden")) {
      $("#" + id).show();
      recomputeFboxSize(id);
    }
  }
  else
  {
    $("#" + id).hide();
  }
}

Result.prototype.generateFacetBoxEntry = function(completion, id) {
  // Nice print
  var listEntry = '';
  var text = completion.text;
  if (!text) return listEntry;
  var re = new RegExp(self.facetPrefix + id + ":", "g");
  var uiText = text.replace(re, id + ":");
  // If text still contains ":facet:", ":facet:<id>:" doesn't match, which
  // means, that this has to be the word facet, which is no facet at all. Just
  // ignore it.
  if (uiText.search(self.facetPrefix) != - 1) return listEntry;
  if (id == "word") uiText += "$";
  var action = "callFacetLink(\'" + uiText + "\');";
  text = text.replace(re, '');
  text = text.replace(/_/g, ' ');
  text = text.replace(/\*/g, '');
  var num = completion['@sc'];
  if (text.length == 0) return true; // Go on with i++
  text = "<a href=\"" + uiText + "\" onclick=\"" + action + " return false;\">" + text + "</a>";
  text += " (" + num + ")";
  this.completionsList.push(uiText);
  listEntry += "<li>" + text + "</li>";
  return listEntry;
}

Result.prototype.getHits = function(result, id, ajaxTime)
{
  var hitsDiv = $(id);
  var hitsBefore = parseInt(result.result.hits["@first"]);
  var hitsTotal = parseInt(result.result.hits["@total"]);
  this.hitsLoaded = parseInt(result.result.hits["@sent"]) + hitsBefore;
  if (result.result.hits["@total"] == 0)
  {
    $("#title").html("Zoomed in on " +  this.hitsLoaded + " of " + hitsTotal + " documents. Searched " + time + " ms and waited " + ajaxTime + " ms.");
    var queryStr = query.lastQuery + "~";
    queryStr = "<a href=\"" + queryStr + "\" onclick=\"callFuzzyLink(\'" + queryStr + "\'); return false;\">" + queryStr + "</a>";
    hitsDiv.html("Sorry, couldn't find any hits for the given query. Try fuzzysearch: " + queryStr);
    return false;
  }
  var html = "<ul>";
  var hits = result.result.hits.hit;
  jQuery.each(hits, function(i)
  {
    html += "<table>" +  hits[i].info[Object.keys(hits[i].info)[0]] + "</table>";
  });
  html += "</ul>";
  // If no hits were sent before for this query, replace the html content.
  if (hitsBefore == 0)
    hitsDiv.html(html);
  // Elsewhise append the new html to the old one (but take care to remove the last line of the old html and the first line of the new html (table open/close tags))
  else
    hitsDiv.html( function(index, oldhtml) {
      oldhtml = oldhtml.substr(0, oldhtml.length - "</ul>".length);
      html = html.substr("<ul>".length);
      return oldhtml + html;
  });
  var time = Math.round(result.result.time.text);
  ajaxTime = Math.round(ajaxTime - time);
  $("#title").html("Zoomed in on " +  this.hitsLoaded + " of " + hitsTotal + " documents. Searched " + time + " ms and waited " + ajaxTime + " ms.");
}

Result.prototype.print = function(url, parameters, extractFromResult, id, requestNumber, resizeFboxes)
{
  var self = this;
  // There is already a new request. Cancel the request.
  if (self.requestNo > requestNumber) return true;
  // Trying: JSON instead of JSONP.
  var ajaxTime= new Date().getTime();
  var result = $.ajax({
   type: "post",
   url: url,
   data: parameters,
   dataType: "json",
   cache: true
  }); 
  result.success(function(response)
  {
    // There is already a new request. Cancel the request.
    if (self.requestNo > requestNumber) return true;
    ajaxTime = new Date().getTime() - ajaxTime;
    if (extractFromResult == "hits")
      self.getHits(response, id, ajaxTime);
    else if (extractFromResult == "completions") {
      self.getCompletions(response, id, ajaxTime);
      if (resizeFboxes && !$("#" + id).is(":hidden")) {
        recomputeFboxSize(id)
      }
    }
  });
  result.fail(function(data, textStatus, error)
  {
    $(errors).append("Error processing url " + url + ": " + textStatus + " " + data.status + " - " + error  + "<br>");
  });
  result.always(function()
  {
  });
}

