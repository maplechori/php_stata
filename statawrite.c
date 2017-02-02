/*
 *  PHP Stata Extension 
 *  Copyright (C) 2014 Adrian Montero
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.gnu.org/licenses/gpl-2.0.html
 */

// http://www.stata.com/help.cgi?dta_115

#include "stataread.h"
#include "php.h"
int stata_endian;
#define R_PosInf INFINITY
#define R_NegInf -INFINITY


/* Mainly for use in packages */
int R_finite(double x)
{
#ifdef HAVE_WORKING_ISFINITE
    return isfinite(x);
#else
    return (!isnan(x) & (x != R_PosInf) & (x != R_NegInf));
#endif
}

#define R_FINITE(x)  R_finite(x)
 
static void OutIntegerBinary(int i, FILE * fp, int naok)
{
    i=((i == NA_INTEGER) & !naok ? STATA_INT_NA : i);
    if (fwrite(&i, sizeof(int), 1, fp) != 1)
	zend_error(E_ERROR, "a binary write error occurred");

}

static void OutByteBinary(unsigned char i, FILE * fp)
{
    if (fwrite(&i, sizeof(char), 1, fp) != 1)
	zend_error(E_ERROR, "a binary write error occurred");
}
static void OutDataByteBinary(int i, FILE * fp)
{
    i=(unsigned char) ((i == NA_INTEGER) ? STATA_BYTE_NA : i);
    if (fwrite(&i, sizeof(char), 1, fp) != 1)
	zend_error(E_ERROR, "a binary write error occurred");
}

static void OutShortIntBinary(int i,FILE * fp)
{
  unsigned char first,second;

#ifdef WORDS_BIGENDIAN
    first = (unsigned char)(i >> 8);
    second = i & 0xff;
#else
    first = i & 0xff;
    second = (unsigned char)(i >> 8);
#endif
  if (fwrite(&first, sizeof(char), 1, fp) != 1)
    zend_error(E_ERROR, "a binary write error occurred");
  if (fwrite(&second, sizeof(char), 1, fp) != 1)
    zend_error(E_ERROR, "a binary write error occurred");
}


static void  OutDoubleBinary(double d, FILE * fp, int naok)
{
    d = (R_FINITE(d) ? d : STATA_DOUBLE_NA);
    if (fwrite(&d, sizeof(double), 1, fp) != 1)
	zend_error(E_ERROR,"a binary write error occurred");
}


static void OutStringBinary(const char *buffer, FILE * fp, int nchar)
{
    if (nchar == 0) return;
    if (fwrite(buffer, nchar, 1, fp) != 1)
	zend_error(E_ERROR,"a binary write error occurred");
}

static char* nameMangleOut(char *stataname, int len)
{
    int i;
    for(i = 0; i < len; i++)
      if (stataname[i] == '.') stataname[i] = '_';
    return stataname;
}


/* Writes out a value label (name, and then labels and levels). 
 * theselevels can be R_NilValue in which case the level values will be
 * written out as 1,2,3, ...
 */
static int 
writeStataValueLabel(const char *labelName, zval * theselabels,
		     int theselevels, const int namelength, FILE *fp)
{
   int txtlen = 0, index; 
   size_t len = 0;
   len = 4*2*(zend_hash_num_elements(Z_ARRVAL_P(theselabels)) + 1);
   txtlen = 0;
   zval * currentLabel;
   zend_string * str_vars;

   ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(theselabels), str_vars, currentLabel) 
   {

	printf("type: %d , len: %d str: %s\n\r", Z_TYPE_P(currentLabel), Z_STRLEN_P(currentLabel), Z_STRVAL_P(currentLabel));	

	txtlen += Z_STRLEN_P(currentLabel) + 1;
   }
   ZEND_HASH_FOREACH_END();	

   printf("textlen: %d\n\r", txtlen); 
   len += txtlen;
   OutIntegerBinary((int)len, fp, 0); 
   zend_error(E_NOTICE,"len: %ld", len);
/* length of table */
    char labelName2[namelength + 1];
    strncpy(labelName2, labelName, namelength + 1); // nameMangleOut changes its arg.
    OutStringBinary(nameMangleOut(labelName2, strlen(labelName)), fp, namelength);
    OutByteBinary(0, fp);
/* label format name */
    OutByteBinary(0, fp); 
    OutByteBinary(0, fp); 
    OutByteBinary(0, fp);
 /*padding*/
    OutIntegerBinary(zend_hash_num_elements(Z_ARRVAL_P(theselabels)), fp, 0);
    OutIntegerBinary(txtlen, fp, 0);
 /* offsets */
    len = 0;
   
 /*
    for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(theselabels), &labelposition), i=0;
                            zend_hash_get_current_data_ex(Z_ARRVAL_PP(theselabels), (void**) &currentLabel, &labelposition) == SUCCESS;
                                     zend_hash_move_forward_ex(Z_ARRVAL_PP(theselabels), &labelposition), i++) {

	

	OutIntegerBinary((int)len, fp, 0);
        zend_error(E_NOTICE, "%s %d", Z_STRVAL_PP(currentLabel), Z_STRLEN_PP(currentLabel));
	len += Z_STRLEN_PP(currentLabel) + 1;
    }
  */

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(theselabels), str_vars, currentLabel)
    { 
        OutIntegerBinary((int)len, fp, 0);
        zend_error(E_NOTICE, "%s %d", Z_STRVAL_P(currentLabel), Z_STRLEN_P(currentLabel));
        len += Z_STRLEN_P(currentLabel) + 1;
    } 
    ZEND_HASH_FOREACH_END();


   // values: just 1,2,3,...
    ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(theselabels), index, str_vars)
    { 
	zend_error(E_NOTICE, "currentValue: %ld", index);
	OutIntegerBinary(index, fp, 0);
    } 
    ZEND_HASH_FOREACH_END();
/****
    
       for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(theselabels), &labelposition), i=0;
                            zend_hash_get_current_data_ex(Z_ARRVAL_PP(theselabels), (void**) &currentLabel, &labelposition) == SUCCESS;
                                     zend_hash_move_forward_ex(Z_ARRVAL_PP(theselabels), &labelposition), i++) {
 
		char *keyStr;
        	uint key_len, key_type;
        	long index;
        	int k;

        	key_type = zend_hash_get_current_key_ex(Z_ARRVAL_PP(theselabels), &keyStr, &key_len, &index, 0, &labelposition);
		zend_error(E_NOTICE, "currentValue: %ld", index);
		OutIntegerBinary(index, fp, 0);
        }

****/
// the actual labels 

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(theselabels), str_vars, currentLabel) 
    {
	len = Z_STRLEN_P(currentLabel);
	printf("str: %s len: %d\n\r", Z_STRVAL_P(currentLabel), len);
	OutStringBinary(Z_STRVAL_P(currentLabel), fp, (int)len);
	OutByteBinary(0, fp);
	txtlen -= len+1;

	if (txtlen < 0)
		zend_error(E_WARNING, "this shoud not happen: overrun");
	

    }
    ZEND_HASH_FOREACH_END();

/****
        for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(theselabels), &labelposition), i=0;
                            zend_hash_get_current_data_ex(Z_ARRVAL_PP(theselabels), (void**) &currentLabel, &labelposition) == SUCCESS;
                                     zend_hash_move_forward_ex(Z_ARRVAL_PP(theselabels), &labelposition), i++) {


	len = Z_STRLEN_PP(currentLabel);
        OutStringBinary(Z_STRVAL_PP(currentLabel), fp, (int)len);
        OutByteBinary(0, fp);
        txtlen -= len+1;

        if (txtlen < 0)
                zend_error(E_WARNING, "this should happen: overrun");

         }

****/    
    if (txtlen > 0) 
	zend_error(E_WARNING, "this should not happen: underrun");

    return 1;
}


void R_SaveStataData(FILE *fp, zval *data, zval *vars, zval *labels)
{
    int version = 10;
    int i, j, k = 0, l, nvar, nobs, charlen;
    char datalabel[81] = "Created by PHP Stata (Adrian Montero)",
	timestamp[18], aname[33];
    char format9g[50] = "%9.0g", strformat[50] = "";
    //const char *thisnamechar;

    zval * data_inner;
    //zval * variables_inner;

    int namelength = 8;
    int fmtlist_len = 12;

    if (version >= 7) namelength=32;
    if (version >= 10) fmtlist_len = 49;

    /* names are 32 characters in version 7 */

    /** first write the header **/
    if (version == 6)
	OutByteBinary((char) VERSION_6, fp);            /* release */
    else if (version == 7)
	OutByteBinary((char) VERSION_7, fp);
    else if (version == 8)  /* and also 9, mapped in R code */
	OutByteBinary((char) VERSION_8, fp);
    else if (version == 10) /* see comment above */
	OutByteBinary((char) VERSION_114, fp);
    OutByteBinary((char) CN_TYPE_NATIVE, fp);
    OutByteBinary(1, fp);            /* filetype */
    OutByteBinary(0, fp);            /* padding */

    nvar = zend_hash_num_elements(Z_ARRVAL_P(vars));
    printf("number of variables: %d\n\r", nvar);
    HashTable * ht_data = Z_ARRVAL_P(data);

    zend_string * str_data = zend_string_init("data", sizeof("data") -1 , 0);
    data_inner = zend_hash_find(ht_data, str_data);
	
    nobs = zend_hash_num_elements(Z_ARRVAL_P(data_inner)); 

    zend_error(E_NOTICE, "Observations: %d", nobs);
    zend_error(E_NOTICE, "Variables: %d", nvar);

    OutShortIntBinary(nvar, fp);
    OutIntegerBinary(nobs, fp, 1);

    printf("number of observations: %d\n\r", nobs);

    int ** types = ecalloc(nvar, sizeof(int*));
    int ** wrTypes = ecalloc(nvar, sizeof(int*));

    for (i = 0; i < nvar; i++)
    {
	types[i] = ecalloc(1, sizeof(int*));
	wrTypes[i] = ecalloc(1, sizeof(int*));
	*(types[i]) = -1;
    }

    /* number of cases */
    datalabel[80] = '\0';
    OutStringBinary(datalabel, fp, 81); 

    time_t rawtime;
    time (&rawtime);
    struct tm  *timeinfo = localtime (&rawtime);

    memset(&timestamp, 0, 18);  
    strftime(timestamp, 18, "%d %b %Y %H:%M",timeinfo);
    timestamp[17] = 0;
    OutStringBinary(timestamp, fp, 18);   /* file creation time - zero terminated string */

    zval *variable_traverse;

    /* version 8, 10 */
    i=0;
    zend_string * str_vars; 

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(vars), str_vars, variable_traverse) {
	
	zval * valueType = zend_hash_str_find(Z_ARRVAL_P(variable_traverse), "valueType", sizeof("valueType") - 1);
	*wrTypes[i] = Z_LVAL_P(valueType);	

	switch(Z_LVAL_P(valueType)) {

		case STATA_SE_BYTE:
			printf("byte %ld\n\r", Z_LVAL_P(valueType));
                        OutByteBinary(STATA_SE_BYTE, fp);
			break;	
		case STATA_SE_INT:
			printf("int %ld\n\r", Z_LVAL_P(valueType));
			OutByteBinary(STATA_SE_INT, fp);
			break;
		case STATA_SE_SHORTINT:
			printf("short %ld\n\r", Z_LVAL_P(valueType));
			OutByteBinary(STATA_SE_SHORTINT, fp);
			break;
		case STATA_SE_FLOAT:
			OutByteBinary(STATA_SE_DOUBLE,fp);
			printf("float: %ld\n\r", Z_LVAL_P(valueType));
			break;
		case STATA_SE_DOUBLE:
			OutByteBinary(STATA_SE_DOUBLE,fp);
			printf("double: %ld\n\r", Z_LVAL_P(valueType));
			break;
		default: 
			charlen = 0;
			zval * strSize;
			long * num;

		        ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data_inner), num, strSize)
			{
			    zval * obs_value = zend_hash_find(Z_ARRVAL_P(strSize), str_vars);
		            k = Z_STRLEN_P(obs_value); 

                	    if (k>charlen)
                            {
                               charlen=k;
                               *types[i] = charlen;
                            }

			}
			ZEND_HASH_FOREACH_END();

			if (charlen > 244)
			   zend_error(E_WARNING, "character strings of >244 byes in column %s will be truncated", "keyStr");

			charlen = (charlen < 244 ) ? charlen : 244;			

			if (charlen == 0)
			{
			   charlen = 2;
			   *types[i] = charlen;
			}
			printf("string %d\n\r", charlen);
			//printf("charlen: %d\n\r", charlen);
			OutByteBinary((unsigned char)(charlen+STATA_SE_STRINGOFFSET), fp);
			break;
			
	}
	i++;
    }
    ZEND_HASH_FOREACH_END();

/** names truncated to 8 (or 32 for v>=7) characters**/

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(vars), str_vars, variable_traverse) {
	strncpy(aname, str_vars->val , namelength);
        OutStringBinary(nameMangleOut(aname, namelength), fp, namelength);
        OutByteBinary(0, fp);
    }
    ZEND_HASH_FOREACH_END();


    /** sortlist -- not relevant **/
   for (i = 0; i < 2*(nvar+1); i++) 
	OutByteBinary(0, fp);

    /** format list: arbitrarily write numbers as %9g format
	but strings need accurate types */

    for (i = 0; i < nvar; i++) {
	printf("%d : %d\n\r", i, *types[i]);
	if (*types[i] != -1){
	    // string types are at most 244 character so we can't get a buffer overflow in sprintf 
	    memset(strformat, 0, 50);
	    sprintf(strformat, "%%%ds", *types[i]);
	    OutStringBinary(strformat, fp, fmtlist_len);
	} else {
	    OutStringBinary(format9g, fp, fmtlist_len);
	}
    }

    /** value labels.  These are stored as the names of label formats,
	which are themselves stored later in the file.
	The label format has the same name as the variable. **/

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(vars), str_vars, variable_traverse) {
        zval * vLabels = zend_hash_str_find(Z_ARRVAL_P(variable_traverse), "vlabels", sizeof("vlabels") - 1);
	
	if (Z_STRLEN_P(vLabels) == 0)
	{
      	   // no label
 	   for(j = 0; j < namelength+1; j++)
              OutByteBinary(0, fp);
	}
	else
	{
	    // label
	    strncpy(aname, Z_STRVAL_P(vLabels), namelength);
	    OutStringBinary(nameMangleOut(aname, namelength), fp, namelength);
	    OutByteBinary(0, fp);
	}
    }
    ZEND_HASH_FOREACH_END();

    memset(datalabel, 0, 81); 

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(vars), str_vars, variable_traverse) {
        zval * dLabels = zend_hash_str_find(Z_ARRVAL_P(variable_traverse), "dlabels", sizeof("dlabels") - 1);

        if (Z_STRLEN_P(dLabels) == 0)
        {
           // no label
	   memset(datalabel, 0, 81);
	   OutStringBinary(datalabel, fp, 81);
        }
        else
        {
            // label
            strncpy(datalabel, Z_STRVAL_P(dLabels), 81);
            datalabel[80] = 0;
            OutStringBinary(datalabel, fp, 81);
        }
    }
    ZEND_HASH_FOREACH_END();

    //The last block is always zeros
    OutByteBinary(0, fp);
    OutByteBinary(0, fp);
    OutByteBinary(0, fp);
   
    if (version >= 7) { 
   /*longer in version 7. This is wrong in the manual*/
	OutByteBinary(0, fp);
	OutByteBinary(0, fp);
    }
	
    long * num = 0;
    zval * strSize;
    /** The Data **/
    data_inner = zend_hash_find(Z_ARRVAL_P(data), str_data);

    i=0;
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data_inner), num, strSize)
    {
    	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(vars), str_vars, variable_traverse) {
        	zval * obs_value = zend_hash_find(Z_ARRVAL_P(strSize), str_vars);
 
		switch(Z_TYPE_P(obs_value))
        	{
                	case IS_LONG:
                	case _IS_BOOL:
                                zend_error(E_NOTICE, "IS LONG %ld", Z_LVAL_P(obs_value));
                                if (*wrTypes[i] == STATA_SE_SHORTINT)
                                        OutShortIntBinary(Z_LVAL_P(obs_value), fp);
                                else
                                        OutIntegerBinary(Z_LVAL_P(obs_value), fp, 0);
                                break;
                	case IS_DOUBLE:
                                OutDoubleBinary(Z_DVAL_P(obs_value), fp, 0);
                                zend_error(E_NOTICE, "IS DOUBLE %f", Z_DVAL_P(obs_value));
                        	break;
                        case IS_STRING:
                                zend_error(E_NOTICE, "IS STRING %s %d", Z_STRVAL_P(obs_value), Z_STRLEN_P(obs_value));
                                k = Z_STRLEN_P(obs_value);
                                if (k == 0)
                                {
                                   OutByteBinary(0, fp);                
                                   k=1;
                                }
                                else
                                {
                                        if (k > 244)
                                            k = 244;

                                        OutStringBinary(Z_STRVAL_P(obs_value), fp, k);
                                }
                                int l = 0;
                                for (l = *(types[i])-k; l > 0; l--)
                                {
                                        OutByteBinary(0, fp);
                                }
                                break;
                        default:
                                zend_error(E_NOTICE, "this should not happen");
                                break;

        	}

	}
	ZEND_HASH_FOREACH_END();
	i++;
    }
    ZEND_HASH_FOREACH_END();

    zval * value_labels; 
    zval * inner_labels;

    value_labels = zend_hash_str_find(Z_ARRVAL_P(labels), "labels", sizeof("labels") - 1);

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(value_labels), str_vars, inner_labels)
    {
	strncpy(aname, str_vars->val, namelength);
        printf("WriteValueLabel: %s %x %d\n\r", aname, inner_labels, namelength);
	writeStataValueLabel(aname, inner_labels, 0, namelength, fp);	
	
    }
    ZEND_HASH_FOREACH_END();

    for (i=0; i < nvar; i++)
    {
        efree(types[i]);
	efree(wrTypes[i]);
    }
    efree(types);
    efree(wrTypes);

}


void do_writeStata(char *fileName, zval *data, zval *variables, zval *labels)
{
    FILE *fp = NULL;

    if ((sizeof(double) != 8) | (sizeof(int) != 4) | (sizeof(float) != 4))
    {
      zend_error(E_ERROR, "cannot yet read write .dta on this platform");
      return;
    }


    fp = fopen(fileName, "wb");

    if (fp == NULL)
    {	     
	 zend_error(E_WARNING, "unable to open file or path for writing: %s", strerror(errno));	 
 	 return;    
    }
    else
    {
    	R_SaveStataData(fp, data, variables, labels);
    	fclose(fp);
    }

}
