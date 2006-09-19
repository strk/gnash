<?php

# This simple script is intended for use with the online
# ActionScript testcases of Gnash. The testcases will POST
# to this file for logging run results

# Set the log filename
$filename = "/tmp/gnashtestreport.txt";

# Set the url corrisponding to the log filename
$url = "file:///tmp/gnashtestreport.txt";

function print_it($msg)
{
	global $logfile;
	fwrite($logfile, $msg);
}

function report()
{
	global $HTTP_POST_VARS;
	global $logfile;

	print_it("\n[".gmdate("M d Y H:i:s")."]\n");
	print_it($HTTP_POST_VARS{'traced'});
}

$logfile = fopen($filename, "a+");
# todo: check return from fopen()
report();

echo "<B>Your tests results have been <A HREF=".$url.">logged</A>, thanks a lot.</B>"


?>
