
/*
document.write(
    '<input type="text" size="40" style="float:right;color:#888;" ' +
    'onclick="this.blur(); window.location.href=\'' + completeSearchLink + '\'" ' +
    'value="click here for facets and more..."/>'
);
*/

window.onload = init;

function init()
{
  time_E = new Date().getTime();
  getFacetBoxes();
}

// nice hack from http://javascript.internet.com/snippets/convert-html-entities.html 
function html_entity_decode(str) 
{
  var ta = document.createElement("textarea");
  ta.innerHTML = str.replace(/</g,"&lt;").replace(/>/g,"&gt;");
  return ta.value;
}

function getQueryFromUrl()
{
  var url = window.location.href;
  author = url.match(/([^\/]+)\.html$/)[1];
  
  // NEW 01Nov12 (baumgari):
  // There is a bug, which causes authors with double names and special
  // characters to fail, since entities are marked just as double names
  // by = within the url. 
  // E.g Jörg-Rüdiger Sack ---> Sack:J=ouml=rg=R=uuml=diger.html.
  // Minimized that problem by expecting an entity to consist of at least 

  // e.g., K=ouml=nig:Felix_G= -> K&ouml;nig:Felix_G=
  author = author.replace(/=([a-zA-Z]{4,}?)=/g, "&$1;");
  // e.g., K&ouml;nig:Felix_G= -> König:Felix_G=
  author = html_entity_decode(author);
  // e.g., König:Felix_G= -> könig:felix_g
  author = author.replace(/[_=]/g, "_");
  //author = author.replace(/[_=]/g, "");
  // e.g., König:FelixG -> author:felixgkönig
  var parts = author.toLowerCase().match(/^(.*?):(.*)$/);
  // 22May10: authors with middle initials have two underscores now.
  authorQuery = "author:" + parts[2] + "_" + parts[1] + ":";
  authorQuery = authorQuery.replace(/_+/g, "_");
  return authorQuery;
}

function showFacetBoxes(boxes) 
{
    time_D = new Date().getTime() - time_D;
    var numOfHits = ":H1.1000";
    var queryParameters = "&qp=W1.4:F1.4:F2.4:F3.4:F4.4" + numOfHits;
    var indexUrl = "http://www.dblp.org/search";
    //var indexUrl = "http://test.dblp.org/search";
    var tables = document.getElementsByTagName("table");
    if (!tables.length) return;
    var table = tables[0];
    table.style.overflow = "hidden";
    var sidebar = document.createElement("div");
    sidebar.id = "sidebar";
    sidebar.style.cssFloat = "right";
    sidebar.style.styleFloat = "right";
    sidebar.style.width = "300px";
    sidebar.style.marginLeft = "20px";
    // NEW 05Mar12 (Ina): 
    // Use the same hack like in getFacetBoxes to make sure the links within the
    // facetboxes are in utf-8. Else there won't be found anything in case of
    // umlauts, etc..
    boxes = boxes.replace(/(\?)(query=.+?)\">/g, function($0,$1,$2)
      {
        // In case there are already parameters (links for showing "all 12314"
        // within the facetboxes), we do nothing.
        if ($2.search("&qp") != -1)
        {
          return "#" + decodeURIComponent(encodeURIComponent($2)) + numOfHits + "\">";
        }
        return "#" + decodeURIComponent(encodeURIComponent($2)) + queryParameters + "\">";
      });
    sidebar.innerHTML = boxes;
    table.parentNode.insertBefore(sidebar, table);

    var link = document.createElement("a");
    link.style.cssFloat = "right";
    link.style.styleFloat = "right";
    // Have a space after the query (so that user can just continue typing)
    // TEMPRORAILY NO SPACE (more/less does not work, Markus is working on it - 06Mar08)
    link.href = indexUrl + "/index.php#query=" + decodeURIComponent(encodeURIComponent(authorQuery)) + queryParameters;
      //link.href = "http://dblp.mpi-inf.mpg.de/dblp-mirror/relay.php?query=" + authorQuery + "&fh=1";
      //link.href = "http://dblp.mpi-inf.mpg.de/dblp-mirror/reset.php?session_name=dblpmirror&index_url=/index.php";
    link.innerHTML = "Facets and more with CompleteSearch"; 
    // insert before text node "List of publications ..."
    var h1s = document.getElementsByTagName("h1");
    if (h1s.length < 1) return;
    var element = h1s[0];
    while (!element.data || !element.data.match(/List of/)) element = element.nextSibling;
    element.parentNode.insertBefore(link, element);

    var hrs = document.getElementsByTagName("hr");
    if (hrs.length < 2) return;
    //var hr = hrs[0];
    //hr.parentNode.insertBefore(link, hr.nextSibling);
    hr = hrs[0];
    form = document.createElement("form");
    // NEW 05Sep12 (baumgari): Replaced form.action by form.onsubmit (see below)
    // to avoid encoding/conversion problems.
      //form.action = "http://test.dblp.org/search/relay.php";
      //form.action = "http://dblp.mpi-inf.mpg.de/dblp-mirror/relay.php?fh=1";
    form.method = "GET";
    form.style.cssFloat   = "right";
    form.style.styleFloat = "right";
    form.style.width      = "300px";
    if   (navigator.appName == "Microsoft Internet Explorer") form.style.margin     = "-3px 0";
    else                                                      form.style.margin     = "0";
    form.style.padding    = "0";
    input = document.createElement("input");
    input.name = "query"
    input.value = authorQuery; // + " "; //"";  //authorQuery;
    input.style.width   = "300px";
    input.style.margin  = "0";
    input.style.padding = "0";
      //input.autocomplete = "off";
    //input.disabled = true;
    form.appendChild(input);
    form.onsubmit = function(e) {
       self.location.href = indexUrl + "/relay.php#query=" + decodeURIComponent(encodeURIComponent(input.value)) + queryParameters;
       return false;
    }

      //Button = document.createElement("input");
      //Button.type = "submit";
      //Button.value = "Search";
      //Button.style.width = "70px";
      //Button.style.border = "1";
      //Form.appendChild(button);
    hr.parentNode.insertBefore(form, hr.nextSibling);
    input.focus();
    hr = hrs[1];
    if (navigator.appName == "Microsoft Internet Explorer") hr.style.clear = "both";
    if (input.createTextRange)
    {
      var v = input.value;
      var r = input.createTextRange();
      r.moveStart('character', v.length);
      r.select();
    }

    // hack to fix padding left of hit numbers, as in "Kurt Mehlhorn (274)"
    var hit_numbers = document.getElementsByTagName("span");
    for (var i = 0; i < hit_numbers.length; ++i) 
      if (hit_numbers[i].className == "hits_number")
        hit_numbers[i].style.paddingLeft = "0.3em";

    // NEW: send client timings back to us
    time_E = new Date().getTime() - time_E;
    if (1)
    {
      var params =     "query="  + authorQuery
                    + "&types="  + "P"
                    + "&time_D=" + time_D
                    + "&time_E=" + time_E;
      if (window.XMLHttpRequest) {
          xmlhttp = new XMLHttpRequest();
      } else if (window.ActiveXObject) {
          xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
      }
      if (xmlhttp) 
      {
        //xmlhttp.open("POST", "http://dblp.mpi-inf.mpg.de/dblp-mirror/client_measurement.php", true);
        xmlhttp.open("POST", "/~ley/www.dblp.org/search/client_measurement.php", true);
        xmlhttp.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );
        xmlhttp.onreadystatechange = function() { };
        xmlhttp.send(params);
      }
    }

}

function hideFacetBoxes()
{
  document.getElementById('sidebar').style.display = "none";
}

function getFacetsURL() {
  return "/~ley/www.dblp.org/search/facetboxes.php?query=" + authorQuery;
}

function getFacetBoxes() 
{
  authorQuery = getQueryFromUrl();
  xmlhttp = null;
  if (window.XMLHttpRequest) {
      xmlhttp = new XMLHttpRequest();
  } else if (window.ActiveXObject) {
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
  }

  if (xmlhttp) {
      var url = getFacetsURL();
      // make sure it's utf8 (hack from http://ecmanaut.blogspot.com/2006/07/encoding-decoding-utf8-in-javascript.html)
      // doesn't work with IE otherwise when query contains ascii codes > 128
      if (window.ActiveXObject) { url = unescape(encodeURIComponent(url)); }
      // NEW 25Oct12 (baumgari): The new static author sites do explicitly set the charset to utf-8. For some reason
      // Chrome now needs to use decodeURIComponent, instead of unescape, while the old sites are just workung with
      // unescape. IE just works with unescape in both cases!
      if (navigator.appVersion.search("Chrome") != -1)
      {
	if (document.characterSet == "ISO-8859-1") url = unescape(encodeURIComponent(url));
        else url = decodeURIComponent(encodeURIComponent(url));
      }

      
      time_D = new Date().getTime();
      xmlhttp.open("GET", url, true);
      xmlhttp.onreadystatechange = function() {
          if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
              showFacetBoxes(xmlhttp.responseText);
          }
      }
      xmlhttp.send(null);
  }
}


