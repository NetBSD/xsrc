#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <strings.h>

#include <xf86.h>
#include <xf86Opt.h>

#include "qxl_option_helpers.h"

int get_int_option(OptionInfoPtr options, int token,
                   const char *env_name)
{
    int value;
    if (env_name && getenv(env_name)) {
        return atoi(getenv(env_name));
    }
    return xf86GetOptValInteger(options, token, &value) ? value : 0;
}

const char *get_str_option(OptionInfoPtr options, int token,
                           const char *env_name)
{
    if (getenv(env_name)) {
        return getenv(env_name);
    }
    return xf86GetOptValString(options, token);
}

int get_bool_option(OptionInfoPtr options, int token,
                     const char *env_name)
{
    const char* value = getenv(env_name);

    if (!value) {
        return xf86ReturnOptValBool(options, token, FALSE);
    }
    if (strcmp(value, "0") == 0 ||
        strcasecmp(value, "off") == 0 ||
        strcasecmp(value, "false") == 0 ||
        strcasecmp(value, "no") == 0) {
        return FALSE;
    }
    if (strcmp(value, "1") == 0 ||
        strcasecmp(value, "on") == 0 ||
        strcasecmp(value, "true") == 0 ||
        strcasecmp(value, "yes") == 0) {
        return TRUE;
    }

    fprintf(stderr, "spice: invalid %s: %s\n", env_name, value);
    exit(1);
}
