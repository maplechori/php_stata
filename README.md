php_stata
=========

PHP Extension for reading and writing STATA files<br>

This extension was created to facilitate the data dissemination of the following projects: <br>

<li><ul><b>Understanding America Study Datapages (http://uasdata.usc.edu) </b></ul></li>
<li><ul><b>Gateway to Global Aging Data (http://g2aging.org). </b></ul></li>

1) Read and display Highcharts charts directly from Stata<br>
2) Provide descriptive information to the UAS Datapages viewers<br>
3) Generate question carts by opening and combining Stata files on the fly<br>

This PHP module has been tested using Apache.

In order to compile the module, please type <b>phpize</b> on the directory to generate the config files.

1) install php5-dev (for phpize)<br>
2) cd to directory with source files<br>
3) $ phpize<br>
4) $ ./configure<br>
5) $ make install<br>
6) On successful execution, the extension stata.so can be found in the subdirectory “modules”<br><br>



Example use:
<pre>

/* Reading */
$res = stata_open("/var/www/html/filename.dta");

echo "Stata observations: " . stata_observations($res);
echo "Stata variables: " . stata_nvariables($res);


print_r(stata_variables($res));

$df = stata_data($res);

echo $df['data'][0]['variablename']
$labels = stata_labels($res)['labels'];
stata_close($res);


/* Writing */

stata_write("filename.dta", array("data" => array( 1 => array("prim_key" => "232342342", 
                                                              "testswitch" => 32.3234, 
                                                              "mode" => 32741), 
                                                   2 => array("prim_key" => "33333333333333333", 
                                                              "testswitch" => pow(2.0, 1023), 
                                                              "mode" => 2147483621) )),  
                
		                            array("prim_key" =>   array("vlabels" => "",
                                                                        "dlabels" => "PRIM KEY",
                                                                        "vfmt" => "%17s",
                                                                        "valueType" => 20 ),
                                                  "testswitch" => array("vlabels" => "",
                                                                        "dlabels" => "TEST SWITCH",
                                                                        "vfmt" => "%9.0g",
                                                                        "valueType" => 255), 
                                                      "mode"  =>  array("vlabels" => "gfk2_live_vl5",
                                                                        "dlabels" => "INTERVIEW MODE", 
                                                                        "vfmt" => "%9.0g", 
									"valueType" => 253)), 
                                           array("labels" => array( "gfk2_live_vl5" => 
                                                                                       array(44 => "44 Face" ,
                                                                                             55 => "55 Call center")
	                                                          )
					    )
				);
</pre>

