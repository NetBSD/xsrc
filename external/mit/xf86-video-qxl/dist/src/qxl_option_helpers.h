#ifndef OPTION_HELPERS_H
#define OPTION_HELPERS_H

#include <xf86Crtc.h>
#include <xf86Opt.h>

int get_int_option(OptionInfoPtr options, int token,
                   const char *env_name);

const char *get_str_option(OptionInfoPtr options, int token,
                           const char *env_name);

int get_bool_option(OptionInfoPtr options, int token,
                     const char *env_name);

#endif // OPTION_HELPERS_H
