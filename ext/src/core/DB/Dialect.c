/***************************************************************************
 *   Copyright (C) 2006-2007 by Konstantin V. Arkhipov                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "onphp.h"
#include "onphp_util.h"

#include "ext/standard/php_string.h"

#include "core/DB/Dialect.h"
#include "core/OSQL/DBValue.h"
#include "core/OSQL/DialectString.h"
#include "core/OSQL/Query.h"
#include "core/Exceptions.h"

ONPHP_METHOD(Dialect, quoteValue)
{
	zval *value;
	
	ONPHP_GET_ARGS("z", &value);
	
	// don't know, how to replicate original voodoo
	if (Z_TYPE_P(value) == IS_LONG) {
		RETURN_LONG(Z_LVAL_P(value));
	} else {
		smart_str string = {0};
		char *slashed;
		int length = 0;
		
		if (Z_TYPE_P(value) == IS_STRING) {
			slashed = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		} else {
			zval *copy;
			
			MAKE_STD_ZVAL(copy);
			ZVAL_ZVAL(copy, value, 1, 0);
			
			convert_to_string(copy);
			
			slashed = estrndup(Z_STRVAL_P(copy), Z_STRLEN_P(copy));
		}
		
		length = strlen(slashed);
		
		slashed =
			php_addslashes(
				slashed,
				length,
				&length,
				0 TSRMLS_CC
			);
		
		smart_str_appends(&string, "'");
		smart_str_appends(&string, slashed);
		smart_str_appends(&string, "'");
		smart_str_0(&string);
		
		efree(slashed);
		
		RETURN_STRINGL(string.c, string.len, 0);
	}
}

ONPHP_METHOD(Dialect, quoteField)
{
	zval *field;
	smart_str string = {0};
	
	ONPHP_GET_ARGS("z", &field);
	
	smart_str_appends(&string, "\"");
	onphp_append_zval_to_smart_string(&string, field);
	smart_str_appends(&string, "\"");
	smart_str_0(&string);
	
	RETURN_STRINGL(string.c, string.len, 0);
}

ONPHP_METHOD(Dialect, quoteTable)
{
	zval *table;
	smart_str string = {0};
	
	ONPHP_GET_ARGS("z", &table);
	
	smart_str_appends(&string, "\"");
	onphp_append_zval_to_smart_string(&string, table);
	smart_str_appends(&string, "\"");
	smart_str_0(&string);
	
	RETURN_STRINGL(string.c, string.len, 0);
}

ONPHP_METHOD(Dialect, toCasted)
{
	zval *field, *type;
	smart_str string = {0};
	
	ONPHP_GET_ARGS("zz", &field, &type);
	
	smart_str_appends(&string, "CAST (");
	onphp_append_zval_to_smart_string(&string, field);
	smart_str_appends(&string, " AS ");
	onphp_append_zval_to_smart_string(&string, type);
	smart_str_appends(&string, ")");
	smart_str_0(&string);
	
	RETURN_STRINGL(string.c, string.len, 0);
}

ONPHP_METHOD(Dialect, timeZone)
{
	zend_bool exist = 0;
	
	ONPHP_GET_ARGS("|b", &exist);
	
	if (exist) {
		RETURN_STRING(" WITH TIME ZONE", 1);
	}
	
	RETURN_STRING(" WITHOUT TIME ZONE", 1);
}

ONPHP_METHOD(Dialect, dropTableMode)
{
	zend_bool cascade = 0;
	
	ONPHP_GET_ARGS("|b", &cascade);
	
	if (cascade) {
		RETURN_STRING(" CASCADE", 1);
	}
	
	RETURN_STRING(" RESTRICT", 1);
}

ONPHP_METHOD(Dialect, quoteBinary)
{
	zval *data, *out;
	
	ONPHP_GET_ARGS("z", &data);
	
	ONPHP_CALL_METHOD_1(getThis(), "quotevalue", &out, data);
	
	RETURN_ZVAL(out, 1, 1);
}

ONPHP_METHOD(Dialect, unquoteBinary)
{
	zval *data;
	
	ONPHP_GET_ARGS("z", &data);
	
	RETURN_ZVAL(data, 1, 1);
}

ONPHP_METHOD(Dialect, typeToString)
{
	zval *type, *out;
	
	ONPHP_GET_ARGS("z", &type);
	
	ONPHP_CALL_METHOD_0(type, "getname", &out);
	
	RETURN_ZVAL(out, 1, 1);
}

ONPHP_METHOD(Dialect, fieldToString)
{
	zval *field, *out;
	
	ONPHP_GET_ARGS("z", &field);
	
	if (ONPHP_INSTANCEOF(field, DialectString)) {
		ONPHP_CALL_METHOD_1(field, "todialectstring", &out, getThis());
	} else {
		ONPHP_CALL_METHOD_1(getThis(), "quotefield", &out, field);
	}
	
	RETURN_ZVAL(out, 1, 1);
}

ONPHP_METHOD(Dialect, valueToString)
{
	zval *value, *out;
	
	ONPHP_GET_ARGS("z", &value);
	
	if (ONPHP_INSTANCEOF(value, DBValue)) {
		ONPHP_CALL_METHOD_1(value, "todialectstring", &out, getThis());
	} else {
		ONPHP_CALL_METHOD_1(getThis(), "quotevalue", &out, value);
	}
	
	RETURN_ZVAL(out, 1, 1);
}

smart_str onphp_dialect_to_needed_string(
	zval *this, zval *expression, char *method TSRMLS_DC
)
{
	smart_str string = {0};
	zval *out;
	
	if (ONPHP_INSTANCEOF(expression, DialectString)) {
		// ONPHP_CALL_METHOD_1 can't be used here due to non-void function
		zend_call_method_with_1_params(
			&expression,
			Z_OBJCE_P(expression),
			NULL,
			"todialectstring",
			&out,
			this
		);
		
		if (EG(exception)) {
			return string;
		}
		
		if (ONPHP_INSTANCEOF(expression, Query)) {
			smart_str_appends(&string, "(");
			onphp_append_zval_to_smart_string(&string, out);
			smart_str_appends(&string, ")");
		} else {
			onphp_append_zval_to_smart_string(&string, out);
		}
	} else {
		// unwrapped zend_call_method_with_1_params()
		// since sizeof(method) != strlen(method)
		zend_call_method(
			&this,
			Z_OBJCE_P(this),
			NULL,
			method,
			strlen(method),
			&out,
			1,
			expression,
			NULL TSRMLS_CC
		);
		
		if (EG(exception)) {
			return string;
		}
		
		onphp_append_zval_to_smart_string(&string, out);
	}
	
	smart_str_0(&string);
	zval_dtor(out);
	
	return string;
}

ONPHP_METHOD(Dialect, toFieldString)
{
	zval *expression;
	
	ONPHP_GET_ARGS("z", &expression);
	
	if (Z_TYPE_P(expression) == IS_NULL) {
		RETURN_NULL();
	}
	
	RETURN_STRING(
		onphp_dialect_to_needed_string(
			getThis(),
			expression,
			"quotefield" TSRMLS_CC
		).c,
		0
	);
}

ONPHP_METHOD(Dialect, toValueString)
{
	zval *expression;
	
	ONPHP_GET_ARGS("z", &expression);
	
	if (Z_TYPE_P(expression) == IS_NULL) {
		RETURN_NULL();
	}
	
	RETURN_STRING(
		onphp_dialect_to_needed_string(
			getThis(),
			expression,
			"quotevalue" TSRMLS_CC
		).c,
		0
	);
}

ONPHP_METHOD(Dialect, fullTextSearch)
{
	zend_throw_exception_ex(
		onphp_ce_UnimplementedFeatureException,
		0 TSRMLS_CC,
		"Implement me first"
	);
}

ONPHP_METHOD(Dialect, fullTextRank)
{
	zend_throw_exception_ex(
		onphp_ce_UnimplementedFeatureException,
		0 TSRMLS_CC,
		"Implement me first"
	);
}

static ONPHP_ARGINFO_ONE;
static ONPHP_ARGINFO_TWO;
static ONPHP_ARGINFO_THREE;
static ONPHP_ARGINFO_DBCOLUMN;
static ONPHP_ARGINFO_DATATYPE;

zend_function_entry onphp_funcs_Dialect[] = {
	ONPHP_ABSTRACT_ME(Dialect, preAutoincrement, arginfo_dbcolumn, ZEND_ACC_PUBLIC)
	ONPHP_ABSTRACT_ME(Dialect, postAutoincrement, arginfo_dbcolumn, ZEND_ACC_PUBLIC)
	ONPHP_ABSTRACT_ME(Dialect, hasTruncate, NULL, ZEND_ACC_PUBLIC)
	ONPHP_ABSTRACT_ME(Dialect, hasMultipleTruncate, NULL, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, quoteValue, arginfo_one, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ONPHP_ME(Dialect, quoteField, arginfo_one, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ONPHP_ME(Dialect, quoteTable, arginfo_one, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ONPHP_ME(Dialect, toCasted, arginfo_two, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ONPHP_ME(Dialect, timeZone, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ONPHP_ME(Dialect, dropTableMode, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ONPHP_ME(Dialect, quoteBinary, arginfo_one, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, unquoteBinary, arginfo_one, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, typeToString, arginfo_datatype, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, toFieldString, arginfo_one, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, toValueString, arginfo_one, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, fieldToString, arginfo_one, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, valueToString, arginfo_one, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, fullTextSearch, arginfo_three, ZEND_ACC_PUBLIC)
	ONPHP_ME(Dialect, fullTextRank, arginfo_three, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
