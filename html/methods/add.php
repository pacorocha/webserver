<?php
$query_string = getenv("QUERY_STRING");

$values = explode('&', $query_string);

$array = [];

foreach ($values as &$expr) {
  $val = explode('=', $expr);
  $array[$val[0]] = intval($val[1]);
}

echo "Result: " . ($array["num1"] + $array["num2"]);
