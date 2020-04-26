<?php
/* Reading */

echo "Opening file test_read.dta...\n";
$res = stata_open("test_read.dta");

echo "Listing metadata...\n";
echo "Stata observations: " . stata_observations($res) . "\n";
echo "Stata variables: " . stata_nvariables($res) . "\n";

echo "Printing variables information:\n";

print_r(stata_variables($res));

echo "Opening data contents:\n";
$df = stata_data($res);

echo "Printing data\n";
print_r($df['data']);

$labels = stata_labels($res)['labels'];

echo "Show labels: \n";
print_r($labels);

echo "Closing Stata file\n";
stata_close($res);

echo "::set-output name=done::Done\n";
