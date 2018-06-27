#ifndef PGB_NODE_NAPI_HELPER_H
#define PGB_NODE_NAPI_HELPER_H

#include <stdio.h>
#include <pgb/debug.h>

#define OK_OR_RAISE_EXCEPTION(env, cond, message) do {						\
	if (!(cond)) {										\
		OK_OR_WARN(cond);								\
		napi_throw_error(env, NULL, message);						\
	}											\
} while (0)

static inline
napi_status napi_helper_declare_method(napi_env env, napi_value exports, void *function, const char *name)
{
	napi_status ret;
	napi_value fn;

	ret = napi_create_function(env, NULL, 0, function, NULL, &fn);
	OK_OR_RETURN(ret == napi_ok, ret);

	ret = napi_set_named_property(env, exports, name, fn);
	OK_OR_WARN(ret == napi_ok);

	return ret;
}

static inline
napi_status napi_helper_get_string(napi_env env, napi_callback_info info, napi_value value, char **__str)
{
	size_t length;
	napi_status ret;
	char *str;

	ret = napi_get_value_string_utf8(env, value, NULL, 0, &length);
	OK_OR_RETURN(ret == napi_ok, ret);

	// Account for the null terminator
	length += 1;

	str = calloc(length, sizeof(char));
	OK_OR_RETURN(ret == 0, napi_generic_failure);

	ret = napi_get_value_string_utf8(env, value, str, length, NULL);
	OK_OR_RETURN(ret == napi_ok, napi_generic_failure);

	*__str = str;

	return ret;
}

#endif /* PGB_NODE_NAPI_HELPER_H */