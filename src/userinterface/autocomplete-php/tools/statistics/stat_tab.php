<html>
<head>
  <title>access_log</title>
  <style>
    body { font-family: arial; color: rgb(50, 100, 100); }
    h2 { font-size: small; }
    td { font-size: small; background-color: rgb(200, 200, 250); padding: 5px; width: 150px; vertical-align: top; }
    .expand-table, .expand-table tr, .expand-table td { font-size: small; padding: 0px; margin: 0px; }
  </style>
  
  <script src="date.js"></script>
  <link rel=stylesheet type="text/css" href="stat.css">

</head>

<body onload="init();">

  <h1>access_log: tabellarisch</h1>
  

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

    if (empty($t->collection)) {
      $warning = "Möglicherweise läuft eine Anwendung mit Log-Level > 2. Entsprechende EInträge wurden ignoriert ('" . substr($line, 1, 100) + "')";
    } else {
      $acs[] = $t;
    }
//    $acs[] = $t;
    $n++;
  }

  if (! empty($warning)) {
    echo "<br>Warnung: $warning";
  }
}

function microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}

  echo date("Y-m-d H:i:s", $acs[0]->date) . "&nbsp;-&nbsp;" . date("H:i:s", $acs[sizeof($acs) - 1]->date);
?>

<p style="font-size: 7px">
  <?php echo $n; ?> Zeilen ausgewertet in <?php echo round((microtime_float() - $t_start) * 1000); ?> ms
   (Quelle: <?php echo $logfile; ?>)
</p>

<script type="text/javascript">
Access = function(date, collection, query_string, query_parameters, query_type, ip, host, time, time_msecs)
{
  this.timestamp = date;
  this.date = new Date(date * 1000);
  this.collection = collection;
  this.query_string = query_string;
  this.query_parameters = query_parameters;
  this.query_type = query_type;
  this.ip = ip;
  this.host = host;
  this.time = time;
  this.time_msecs = parseInt(time_msecs);
  
  acs.push(this);
}

var acs = new Array();
var ip_order_asc = false;
var date_order_asc = false;
var time_order_asc = false;
var collection_order_asc = false;

function draw_table(acs)
{
	result_table.innerHTML = 
		"<tr>"
		+ "<th style=\"width: 100px;\"><a href=\"javascript:draw_table_by_date(acs);\">Zeitpunkt</a></th>"
		+ "<th<a href=\"javascript:draw_table_by_collection(acs);\">Anwendung</a></th>"
		+ "<th>Anfrage</th>"
		+ "<th style=\"width: 10px;\">Typ</th>"
		+ "<th>Parameter</th>"
		+ "<th><a href=\"javascript:draw_table_by_ip(acs);\">IP</a></th>"
		+ "<th><a href=\"javascript:draw_table_by_time(acs);\">Dauer</a></th>"
		+ "</tr>"

		for (var j in acs)
		{
		  access = acs[j];

		  var first_entry = result_table.childNodes[1];
		  var tr = document.createElement("tr");
		  
		  result_table.appendChild(tr);
		  
		  tr.innerHTML =
		  "<td style=\"width: 100px;\">" + access.date.dateFormat("Y-m-d  H:i:s") + "</td>"
		  +	"<td style=\"width: 30px;\">" + access.collection + "</td>"
		  +	"<td>" + access.query_string + "</td>"
		  +	"<td style=\"width: 10px;\">" + access.query_type + "</td>"
		  +	"<td>" + access.query_parameters + "</td>"
		  +	"<td>" + access.ip + "</td>"
		  +	"<td>" + access.time + "</td>";
		}
}


function draw_table_by_date(acs)
{
	acs.sort(sort_by_date);
	draw_table(acs);
  date_order_asc = ! date_order_asc;
}


function sort_by_date(access1, access2)
{
  if (date_order_asc) {
    //    console.log(access1.timestamp + " - " + access2.timestamp)
    return access1.date - access2.date;
  } else {
    //    console.log(access2.timestamp + " - " + access1.timestamp)
    return access2.date - access1.date;
  }
}


function draw_table_by_ip(acs)
{
  ip_order_asc = ! ip_order_asc;
	acs.sort(sort_by_ip);
	draw_table(acs);
}


function sort_by_ip(access1, access2)
{
  if (ip_order_asc) {
    return access1.ip > access2.ip;
  } else {
  	return access1.ip <= access2.ip;
  }
}


function draw_table_by_time(acs)
{
  time_order_asc = ! time_order_asc;
	acs.sort(sort_by_time);
	draw_table(acs);
}


function sort_by_time(access1, access2)
{
  if (time_order_asc) {
    return access1.time_msecs < access2.time_msecs;
  } else {
  	return access1.time_msecs >= access2.time_msecs;
  }
}


function draw_table_by_collection(acs)
{
  collection_order_asc = ! collection_order_asc;
	acs.sort(sort_by_collection);
	draw_table(acs);
}


function sort_by_collection(access1, access2)
{
  if (collection_order_asc) {
    return access1.collection < access2.collection;
  } else {
  	return access1.collection >= access2.collection;
  }
}


function init()
{
    result_table = document.getElementById("acs").getElementsByTagName("tbody")[0];
    draw_table(acs);
}



<?php
  foreach ($acs as $ac) {
    echo "\nacs.push(new Access('{$ac->date}','{$ac->collection}','{$ac->query_string}','{$ac->query_parameters}','{$ac->query_type}','{$ac->ip}','{$ac->host}','{$ac->time}','{$ac->time_msecs}'));";
  }
?>
</script>

<table id="acs" class="acs" width="100%">
	<tbody></tbody>
</table>

</body>
</html>