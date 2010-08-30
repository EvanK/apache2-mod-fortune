/*
    Copyright (c) 2010 Evan Kaufman

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

/*
 * Include the core server components.
 */
#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

/*
 * This module
 */
module AP_MODULE_DECLARE_DATA fortune_module;

/*
 * This modules per-server configuration structure.
 */
typedef struct {
    int maxlen;
    char *binloc;
} modfortune_config;

/*
 * Remove trailing newline(s) from pipe output
 */
static char* mod_fortune_chomp_output(char* str)
{
    char* orig = str + strlen(str);
    while(*--orig == '\n');
    *(orig + 1) = '\0';
    return str;
}

/*
 * Register as a handler for HTTP methods and thus be invoked for all requests
 */
static int mod_fortune_method_handler (request_rec *r)
{
    // assuming short fortune to be 160 chars (plus null char)
    // TODO: allow custom fortune max size from an optional config directive (ie: FortuneMaxLength), defaulting to 160 + 1
    char fortune_output[161];
    FILE *fortune_pipe;
    
    // invoke "fortune -s" in a subshell
    // TODO: get fortune binary location from optional config directive (ie: FortuneProgram), defaulting to /usr/games/fortune, and verify it exists and is executable
    // TODO: add "-n <maxlength>" to subshell command
    char fortune_cmd[] = "/usr/games/fortune -s";
    fortune_pipe = popen(fortune_cmd, "r");
    // if opening pipe fails, write a message to stderr
    if (fortune_pipe == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, "mod_fortune: Failed to open pipe to %s", fortune_cmd);
    }
    // otherwise funnel pipe's output into cstring and set as environment variable
    else {
        int fortune_size = fread(fortune_output, sizeof(char), sizeof(fortune_output)-1, fortune_pipe);
        fortune_output[ fortune_size ] = '\0';
        pclose(fortune_pipe);
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_fortune: Retrieved fortune of %d chars", (int)strlen(fortune_output));
        apr_table_set(r->subprocess_env, "FORTUNE_COOKIE", mod_fortune_chomp_output(fortune_output) );
    }
    
    // Return DECLINED so that the Apache core will keep looking for
    // other modules to handle this request.  This effectively makes
    // this module completely transparent.
    return DECLINED;
}

/**
 * A declaration of the configuration directives that are supported by this module.
 */
static const command_rec mod_fortune_cmds[] =
{
	AP_INIT_TAKE1(
		"FortuneMaxLength",
		ap_set_int_slot,
		(void*)APR_OFFSETOF(modfortune_config, maxlen),
		OR_ALL,//RSRC_CONF,
		"FortuneMaxLength <integer> -- the maximum length in characters of fortune to retrieve."
	),
	AP_INIT_TAKE1(
		"FortuneProgram",
		ap_set_string_slot,
		(void*)APR_OFFSETOF(modfortune_config, binloc),
		OR_ALL,//RSRC_CONF,
		"FortuneProgram <string> -- the location of the executable fortune binary."
	),
	{NULL}
};

/**
 * Creates the per-server configuration records.
 */
static void* create_modfortune_config(apr_pool_t* pool, server_rec* s) {
	modfortune_config* svr = apr_pcalloc(pool, sizeof(modfortune_config));
	/* Set up the default values for fields of svr */
	svr->maxlen = 160;
	svr->binloc = "/usr/games/fortune";
	return svr ;
}

/*
 * This function is a callback and it declares what other functions
 * should be called for request processing and configuration requests.
 * This callback function declares the Handlers for other events.
 */
static void mod_fortune_register_hooks (apr_pool_t *p)
{
    ap_hook_handler(mod_fortune_method_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/*
 * Declare and populate the module's data structure.
 */
module AP_MODULE_DECLARE_DATA fortune_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,
    NULL,
    create_modfortune_config,   /* Create config rec for host */
    NULL,
    mod_fortune_cmds,           /* Configuration directives */
    mod_fortune_register_hooks, /* Hook into APR API */
};

