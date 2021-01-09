<?php

  // The path to the autocomplete folder (relative to web server's document root, with leading and trailing slash!)
  $config->autocomplete_path = "/autocomplete-php.NEW_MARKUS/autocomplete/";

?>
<html>
<head>
  <title>Access-Log-Auswertung</title>
  <style>
    body { font-family: arial; color: rgb(50, 100, 100); }
    h2 { font-size: normal; }
    td { font-size: small; background-color: rgb(220, 220, 250); padding: 5px; width: 150px; vertical-align: top; }
    .title { font-size: small; background-color: rgb(200, 200, 250); padding: 5px; width: 150px; vertical-align: top; }
  </style>
  
  <script src="date.js"></script>
  <script src="<?php echo get_full_url($config->autocomplete_path); ?>logging.js"></script>
  <link rel=stylesheet type="text/css" href="<?php echo get_full_url($config->autocomplete_path); ?>logging.css">

</head>

<body onload="init();">

  <h1>access_log</h1>
  
<?php

class access
{
  var $date;
  var $collection;
  var $query_string;
  var $query_parameters;
  var $query_type;
  var $ip;
  var $host;
  var $time;
  var $time_msecs;
}

$t_start = microtime_float();
$acs = array();
$logfile = "/var/opt/completesearch/log/completesearch.access_log";
$getHost = false;   // If true the host name of a ip address is tried to resolve
$warning = "";

$handle = fopen($logfile, "r");
if ((bool)$handle === false)
{
  echo "\nLog file '" . $logfile . "' not found";
} 
else {
  // Seek to the end
  fseek($handle, -100000, SEEK_END);
  $content = fread($handle, 100000);
  $i = strpos($content, "\n");
  $n = 0;
  
  foreach (split("\n", substr($content, $i)) as $line) 
  {
    if (empty($line)) continue;

    $p = explode("|", $line);
    
    $t = new access();
    sscanf(trim($p[0]), "%d.%d.%d %d:%d:%d ", $d, $m, $y, $h, $mm, $s);
    $t->date = mktime($h, $mm, $s, $m, $d, $y);
    $t->collection = trim($p[1]);
    $t->query_string = trim($p[2]);
    $tp = explode(" ", trim($p[3]));
    $t->query_parameters = trim($tp[0]);
    $t->query_type = trim($tp[4]);
    // IP address
    sscanf(trim($p[4]), "%s ", $t->ip);
    if ($getHost) {
//      try {
        $t->host = gethostbyaddr($t->ip);
//      } catch (Exception $e) {
//        $t->host = "Konnte nicht aufgelöst werden: " . $t->ip;
//      }
    }
    sscanf(trim($p[5]), "%s msecs", $t->time_msecs);
    $t->time = trim($p[5]);

    $n++;
    
    if (empty($t->collection)) {
      $warning = "Möglicherweise läuft eine Anwendung mit Log-Level > 2. Entsprechende EInträge wurden ignoriert ('" . substr($line, 1, 100) + "')";
    } else {
      $acs[] = $t;
    }
  }
  
  echo "<h3>" . date("Y-m-d H:i:s", $acs[0]->date) . "&nbsp;-&nbsp;" . date("H:i:s", $acs[sizeof($acs) - 1]->date) . "</h3>";
  echo "$n Zeilen ausgewertet in " . round((microtime_float() - $t_start) * 1000) . " ms";
  echo "<br>Quelle: $logfile";
  
  if (! empty($warning)) {
    echo "<br>Warnung: $warning";
  }
}


function microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}


/**
	 * Construct the full URL for the relative path given by $relative_path
	 *
	 * @param string $relative_path the relative path
	 * @return string the full URL
	 */
	function get_full_url ($relative_path)
	{
		return "http://" . $_SERVER['HTTP_HOST'] . $relative_path;
	}

	

?>

<script type="text/javascript">
Access = function(date, collection, query_string, query_parameters, query_type, ip, host, time, time_msecs)
{
  this.date = date;
  this.collection = collection;
  this.query_string = query_string;
  this.query_parameters = query_parameters;
  this.query_type = query_type;
  this.ip = ip;
  this.host = host;
  this.time = time;
  this.time_msecs = time_msecs;
  
  
  if (!acs_g[this.collection]) {
    acs_g[this.collection] = new Array();
  }
  if (!acs_g[this.collection][this.ip]) {
    acs_g[this.collection][this.ip] = new Array();
  }
  acs_g[this.collection][this.ip].push(this);
}

var acs_g = new Array();
var AC = new Object();
AC.log_level = 4;
var notes = new Notes();

<?php
$dt = ($acs[sizeof($acs) - 1]->date - $acs[0]->date);
echo "var dt=$dt;";
foreach ($acs as $ac) {
    echo "\nnew Access('{$ac->date}','{$ac->collection}','{$ac->query_string}','{$ac->query_parameters}','{$ac->query_type}','{$ac->ip}','{$ac->host}','{$ac->time}','{$ac->time_msecs}');";
}
?>

function init()
{
//  notes.clear();
  var stat = new Array();
  
  notes.add("Anwendungen", 0, true)

    
  for (var i in acs_g) {

    stat[i] = new Array();
    stat[i]["ip"] = 0;
    stat[i]["access"] = 0;
    stat[i]["max_time"] = 0;
    
    notes.add(i, 1)
    for (var j in acs_g[i]) {
      stat[i]["ip"]++;
      notes.add(j, 2)
      for (var k in acs_g[i][j]) {
        stat[i]["access"]++;
        if (acs_g[i][j][k].time_msecs > stat[i]["max_time"]) {
          stat[i]["max_time"] = acs_g[i][j][k].time_msecs;
        }
        notes.add("Anfrage: <b>" + acs_g[i][j][k].query_string + "</b>", 3)
        notes.add("<i>Anwendung:</i> " + acs_g[i][j][k].collection, 4)
        notes.add("<i>Zeitpunkt:</i> " + new Date(acs_g[i][j][k].date * 1000), 4)
        notes.add("<i>Parameter:</i> " + acs_g[i][j][k].query_parameters, 4)
        notes.add("<i>Typ:</i> " + acs_g[i][j][k].query_type, 4)
        notes.add("<i>Dauer:</i> " + acs_g[i][j][k].time, 4)
      }
    }
  }
  
  var table = document.getElementById("tab");
      
  for (var i in stat) {
    tr = document.createElement("tr");
    table.appendChild(tr);
    tr.innerHTML = "<td class=\"title\"><b>" + i + "</b><br>" + "</td><td class=\"title\">Gesamt</td><td class=\"title\">Pro Minute</td>";

    tr = document.createElement("tr");
    table.appendChild(tr);
    tr.innerHTML = "<td>Zugriffe</td><td>" + stat[i]["access"] + "</td><td>" + Math.round(stat[i]["access"] / dt * 60) + "</td>";

    tr = document.createElement("tr");
    table.appendChild(tr);
    tr.innerHTML = "<td>IP's</td>" + "<td>" + stat[i]["ip"] + "</td><td>" + Math.round(stat[i]["ip"] / dt * 60) + "</td>";

    tr = document.createElement("tr");
    table.appendChild(tr);
    tr.innerHTML = "<td>Max. Ausführung [ms]</td>" + "<td>" + stat[i]["max_time"] + "</td><td>-</td>";
  }

  var infobox = document.getElementById("info");
  if (infobox) {
    infobox.appendChild(notes.toDOM());
  }
  
}

</script>
  <h2>Überblick</h2>
  <table><tbody id="tab"></tbody></table>

  <br><br><h2>Gruppiert</h2>
  <div id="info"></div>

  <h2>Tabellarisch</h2>
  Weiter zur <a href="stat_tab.php">tabellarischen Darstellung</a>
</body>
</html>
