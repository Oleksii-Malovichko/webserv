#!/usr/bin/php
<?php

echo "Content-Type: text/html\r\n";
echo "Status: 200 OK\r\n";
echo "\r\n";

$reqBody = file_get_contents("php://stdin");

echo $reqBody;
?>
