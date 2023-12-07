<?php

$input_data = stream_get_contents(STDIN);

$filename = getenv("FILE_NAME");
$path = getenv("PATH_INFO");

/* // Extract the extension from the original filename */
/* $extension = pathinfo($filename, PATHINFO_EXTENSION); */
/**/
/* // Create the new filename based on the extension */
/* $new_filename = "image." . $extension; */

file_put_contents($path . '/' . $filename, $input_data);
