<?php

if (array_key_exists('rid', $_POST)) {
	$rid = $_POST['rid'];
}
else {
	$rid = -1;
}

//ob_clean();
ob_start();

// Die ID dieses Requests zurückgeben
echo "<rid>$rid</rid>";

?>