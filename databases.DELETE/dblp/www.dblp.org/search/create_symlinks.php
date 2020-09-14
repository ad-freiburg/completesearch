<?php

$files = array("autocomplete.css", "autocomplete_ie55.css", "autocomplete_ie5-6.css", "images", "index.php", "relay.php");

require("autocomplete_config.php");

$needle = "/autocomplete/";
$t = strrpos($config->autocomplete_path, $needle);
$target = substr($config->autocomplete_path, 0, $t - strlen($needle) + 1);
//echo "\n".$target;

foreach ($files as $file) 
{
    if (! file_exists($file)) {
        echo "\nCreating symbolic link for $file ...";
        $cmd = sprintf("ln -s %s %s", "..$target/$file", $file);
        echo "\n$cmd";
        echo exec($cmd);
    }
}

?>
