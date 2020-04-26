# PHP Extension for reading and writing STATA files

This extension was created to facilitate the data dissemination of the following projects: 

- The Understanding America Study Datapages (http://uasdata.usc.edu) 
- The Gateway to Global Aging Data (http://g2aging.org). 

It has already been implemented in:

- Read and display Highcharts charts directly from Stata
- Provide descriptive information to the UAS Datapages viewers
- Generate question carts by opening and combining Stata files on the fly

This PHP module has only been tested with Apache/Linux.

Example use:

In order to compile the module, please type **phpize** on the directory to generate the config files.

1. install php7-dev (for phpize)
2. cd to directory with source files
3. $ phpize
4. $ ./configure --enable-stata
5. $ make install
6. On successful execution, the extension stata.so can be found in the subdirectory ***modules***


Example use:

```php
<?php

/* Writing */

echo "Opening file test_read.dta for writing...\n";

stata_write("test_read.dta", array("data" => array(
                      1 => array("prim_key" => "232342342",
                                 "testswitch" => 32.3234,
                                 "mode" => 32741),
                      2 => array("prim_key" => "33333333333333333",
                                 "testswitch" => pow(2.0, 1023),
                                 "mode" => 2147483621) )) ,
                array("prim_key" => array("vlabels" => "",
                                           "dlabels" => "PRIM KEY",
                                           "vfmt" => "%17s",
                                           "valueType" => 20 ),
                      "testswitch" => array("vlabels" => "",
                                            "dlabels" => "TEST SWITCH",
                                            "vfmt" => "%9.0g",
                                            "valueType" => 255),
                      "mode"  => array("vlabels" => "gfk2_live_vl5",
                                       "dlabels" => "INTERVIEW MODE",
                                       "vfmt" => "%9.0g", "valueType" => 253)),
               array("labels" => array( "gfk2_live_vl5" =>
                                              array(44 => "44 Face" ,
                                                    55 => "55 Call center"))));

echo "Stata file written...\n";
?>

```


```php
<?php

// Reading

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

echo "Done!\n";

?>
```
