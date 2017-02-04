/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "stataread.h"
#include "php_stata.h"

struct StataDataFile * do_readStata(char * fileName);
int do_stataClose(struct StataDataFile * dta);
void do_writeStata(char *fileName, zval *data, zval *variables, zval *labels);


/* If you declare any globals in php_stata.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(stata)
*/

/* True global resources - no need for thread safety here */
#define PHP_STATA_FILE_RES_NAME "Stata File Resource"

static int le_stata_file;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("stata.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_stata_globals, stata_globals)
    STD_PHP_INI_ENTRY("stata.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_stata_globals, stata_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* {{{ php_stata_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_stata_init_globals(zend_stata_globals *stata_globals)
{
	stata_globals->global_value = 0;
	stata_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(stata)
{
//	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(stata)
{
#if defined(COMPILE_DL_STATA) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(stata)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(stata)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "stata support", "enabled");
	php_info_print_table_row (2, "version", PHP_STATA_VERSION);
  	php_info_print_table_row (2, "author", PHP_STATA_AUTHOR);
  	php_info_print_table_row (2, "homepage", "http://www.adrianmontero.info");	
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


PHP_MINIT_FUNCTION (stata)
{
  le_stata_file =
    zend_register_list_destructors_ex (NULL, NULL, PHP_STATA_FILE_RES_NAME,
				       module_number);
	
     return SUCCESS;
}

// implementation of a custom my_function()
/* {{{ 
*/
PHP_FUNCTION (stata_open)
{
  struct StataDataFile *dta;
  char *name;
  size_t name_len;
  if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "s", &name, &name_len)
      == FAILURE)
    {
      RETURN_NULL ();
    }
  zend_error(E_NOTICE, "Opening stata file: %s", name);
  dta = do_readStata (name);

  if (dta != NULL)
  {
  	RETURN_RES(zend_register_resource(dta, le_stata_file));
  }
  else
  {
        RETURN_NULL();
  }
}
/* }}} 
*/


PHP_FUNCTION (stata_nvariables)
{
  struct StataDataFile *dta = NULL;
  zval *stataData;


  if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "r", &stataData) ==
      FAILURE)
    {
      RETURN_NULL ();
    }


    dta = (struct StataDataFile *) zend_fetch_resource(Z_RES_P(stataData), PHP_STATA_FILE_RES_NAME, le_stata_file);


  if (dta && dta->nvar > 0)
    {
      RETURN_LONG (dta->nvar);
    }
  else
    {
      RETURN_LONG (0);
    }


}

PHP_FUNCTION (stata_close)
{
  struct StataDataFile *dta = NULL;
  zval *stataData;


  if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "r", &stataData) ==
      FAILURE)
    {
      RETURN_NULL ();
    }


    dta = (struct StataDataFile *) zend_fetch_resource(Z_RES_P(stataData), PHP_STATA_FILE_RES_NAME, le_stata_file);


  if (dta != NULL)
  {
 	 do_stataClose (dta);
         RETURN_BOOL (1);
  }
  else
        RETURN_BOOL(0);

}


PHP_FUNCTION (stata_observations)
{
  struct StataDataFile *dta = NULL;
  zval *stataData;


  if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "r", &stataData) ==
      FAILURE)
    {
      RETURN_NULL ();
    }

    dta = (struct StataDataFile *) zend_fetch_resource(Z_RES_P(stataData), PHP_STATA_FILE_RES_NAME, le_stata_file);

  if (dta && dta->nobs > 0)
    {
      RETURN_LONG (dta->nobs);
    }
  else
    {
      RETURN_LONG (0);
    }
}

PHP_FUNCTION (stata_variables)
{
  struct StataDataFile *dta = NULL;
  struct StataVariable *stv;
  zval *stataData;
  int count = 0;

  if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "r", &stataData) == FAILURE)
  {
      RETURN_NULL ();
  }

  dta = (struct StataDataFile *) zend_fetch_resource(Z_RES_P(stataData), PHP_STATA_FILE_RES_NAME, le_stata_file);

  if (dta == NULL)
	RETURN_NULL();

  array_init(return_value);

  for (stv = dta->variables; stv; stv = stv->next, count++)
  {
      zval tmp_array;
      array_init(&tmp_array);
      add_assoc_string (&tmp_array, "vlabels", stv->vlabels); 
      add_assoc_string (&tmp_array, "dlabels", stv->dlabels);
      add_assoc_string (&tmp_array, "vfmt", stv->vfmt);
      add_assoc_long(&tmp_array, "valueType", stv->valueType);
      add_assoc_zval (return_value, stv->name, &tmp_array);
  }

}


PHP_FUNCTION (stata_labels)
{
  struct StataDataFile *dta;
  struct StataLabel *stl;
  zval *stataData = NULL;

  if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "r", &stataData) ==
      FAILURE)
    {
      RETURN_NULL ();
    }

    dta = (struct StataDataFile *) zend_fetch_resource(Z_RES_P(stataData), PHP_STATA_FILE_RES_NAME, le_stata_file);

  if (dta == NULL)
    RETURN_NULL();
  array_init (return_value);
  int count = 0;
  char buff[256];
  char currName[256];

  buff[0] = 0;
  currName[0] = 0;

  zval innertable, temp_array;
  array_init (&innertable);

  int finishup = 0;

  for (stl = dta->labels; stl; stl = stl->next)
    {
      array_init(&temp_array);

      if (currName[0] == 0)
	{
	  strcpy (currName, stl->name);
	  sprintf (buff, "%d", stl->value);
	  add_assoc_string (&temp_array, buff, stl->string);
	}
      else
	{
	  if (!strcmp (currName, stl->name))
	    {
	      sprintf (buff, "%d", stl->value);
	      add_assoc_string (&temp_array, buff, stl->string);
	      strcpy (currName, stl->name);

	      if (count == dta->nlabels - 1)
		{
		  sprintf (buff, "%d", stl->value);
		  add_assoc_string (&temp_array, buff, stl->string);
		  finishup = 1;
		}

	    }
	  else
	    {
	      add_assoc_zval (&innertable, currName, &temp_array);
	      count++;
	      strcpy (currName, stl->name);
	      sprintf (buff, "%d", stl->value);
	      add_assoc_string (&temp_array, buff, stl->string);

	    }
	}
    }

  if (finishup)
    add_assoc_zval(&innertable, currName, &temp_array);

  add_assoc_zval(return_value, "labels", &innertable);
}



PHP_FUNCTION (stata_data)
{
  struct StataDataFile *dta;
  struct StataObservation *obs;
  struct StataObservationData *obd;
  struct StataVariable *stv;
  zval *stataData, data;
  int observation_counter = 0;
 
  if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "r", &stataData) ==
      FAILURE)
    {
      RETURN_NULL ();
    }


  dta = (struct StataDataFile *) zend_fetch_resource(Z_RES_P(stataData), PHP_STATA_FILE_RES_NAME, le_stata_file);

  if (dta == NULL)
    RETURN_NULL();
  
  array_init (return_value);
  array_init(&data);

  for (obs = dta->observations; obs;
       obs = obs->next, observation_counter++)
    {
      zval observation;
      array_init(&observation);       

      for (obd = obs->data, stv = dta->variables; obd && stv;
	   obd = obd->next, stv = stv->next)
	{
	  switch (stv->valueType)
	    {
	    case STATA_SE_FLOAT:
	    case STATA_SE_DOUBLE:
	    case STATA_FLOAT:
	    case STATA_DOUBLE:
	      add_assoc_double (&observation, stv->name, obd->value.d);
	      break;
	    case STATA_SE_INT:
	    case STATA_INT:
	    case STATA_SE_SHORTINT:
	    case STATA_SHORTINT:
	    case STATA_SE_BYTE:
	    case STATA_BYTE:
	      add_assoc_long (&observation, stv->name, obd->value.i);
	      break;
	    default:
	      if (stv->valueType > 244)
		zend_error (E_ERROR,"unknown data type");
	      add_assoc_string (&observation, stv->name, obd->value.string);
	      break;
	    }
	}
	add_index_zval(&data, observation_counter, &observation);
    }

  add_assoc_zval(return_value, "data", &data);

}
/* 
{{{ */
PHP_FUNCTION(stata_write)
{
    zval *labels, *data, *variables;
    char *fname;
    size_t str_len; 

    if (zend_parse_parameters (ZEND_NUM_ARGS ()TSRMLS_CC, "saaa", &fname, &str_len, &data, &variables, &labels) == FAILURE)
    {
      RETURN_NULL ();
    }

    zend_string * str_labels = zend_string_init("labels", sizeof("labels") - 1, 0);
    HashTable * ht_labels = Z_ARRVAL_P(labels);


    if (zend_hash_exists(ht_labels, str_labels)) {
	  zval *vlabels;
          zval * innerLabels = zend_hash_str_find(ht_labels, "labels", sizeof("labels") - 1);
	  HashTable * ht_innerLabels = Z_ARRVAL_P(innerLabels);

          ZEND_HASH_FOREACH_PTR(ht_innerLabels, vlabels) {

		  if (Z_TYPE_P(vlabels) == IS_ARRAY) {
                        zend_ulong index;
                        HashPosition position;
			zend_string * key_zs;
 			HashTable * ht_vlabels = Z_ARRVAL_P(vlabels);
                        int key_type = zend_hash_get_current_key_ex(ht_vlabels, &key_zs, &index, &position);
 
                        switch (key_type) {
                              case HASH_KEY_IS_STRING:
                              // associative array keys
                              //php_printf("key: %s<br>", key);
                              break;
                              case HASH_KEY_IS_LONG:
                              // numeric indexes
                              //php_printf("index: %ld<br>", index);
                              break;
                             default:
                              break;
                       }

	
		  }		

	  }
          ZEND_HASH_FOREACH_END();
        }
    

    do_writeStata(fname, data, variables, labels);

}
/* }}} */

/* {{{ stata_functions[]
 *
 * Every user visible function must have an entry in stata_functions[].
 */
const zend_function_entry stata_functions[] = {
        PHP_FE(stata_open, NULL)
        PHP_FE(stata_observations, NULL)
        PHP_FE(stata_data, NULL)
        PHP_FE(stata_variables, NULL)
        PHP_FE(stata_nvariables, NULL)
        PHP_FE(stata_labels, NULL)
        PHP_FE(stata_close, NULL)
        PHP_FE(stata_write, NULL)
        PHP_FE_END      /* Must be the last line in stata_functions[] */
};
/* }}} */

/* {{{ stata_module_entry
 */
zend_module_entry stata_module_entry = {
        STANDARD_MODULE_HEADER,
        "stata",
        stata_functions,
        PHP_MINIT(stata),
        PHP_MSHUTDOWN(stata),
        PHP_RINIT(stata),               /* Replace with NULL if there's nothing to do at request start */
        PHP_RSHUTDOWN(stata),   /* Replace with NULL if there's nothing to do at request end */
        PHP_MINFO(stata),
        PHP_STATA_VERSION,
        STANDARD_MODULE_PROPERTIES
};
/* }}} */


#ifdef COMPILE_DL_STATA
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(stata)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
