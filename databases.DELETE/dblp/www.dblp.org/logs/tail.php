<?php

function read_file($file, $lines) {
   //global $fsize;
    $handle = fopen($file, "r");
    $linecounter = $lines;
    $pos = -2;
    $beginning = false;
    $text = array();
    while ($linecounter > 0) {
        $t = " ";
        while ($t != "\n") {
            if(fseek($handle, $pos, SEEK_END) == -1) {
                $beginning = true; 
                break; 
            }
            $t = fgetc($handle);
            $pos --;
        }
        $linecounter --;
        if ($beginning) {
            rewind($handle);
        }
        $text[$lines-$linecounter-1] = fgets($handle);
        if ($beginning) break;
    }
    fclose ($handle);
    $lines = array_reverse($text);
    foreach ($lines as $line) {
    echo "\n" . $line . "<br>";}

    return array_reverse($text);
}
 
$lines = read_file($file, $lines);
foreach ($lines as $line) {
    echo $line;
}
?>
