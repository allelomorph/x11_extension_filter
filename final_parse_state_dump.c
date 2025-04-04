#include <stdio.h>     // fopen fprintf
#include <string.h>    // strerror
#include <errno.h>
#include <stdbool.h>   // needed for parse.h
#include <assert.h>

#include "final_parse_state_dump.h"
#include "parse.h"

/*
struct constant {
    unsigned long value;
    const char *name;
};

struct request {
    const char *name;
    const struct parameter *parameters;
    const struct parameter *answers;

    request_func *request_func;
    reply_func *reply_func;
    // stack values to be transfered to the reply code
    int record_variables;
};
struct event {
    const char *name;
    const struct parameter *parameters;
    enum event_type { event_normal = 0, event_xge = 1} type;
#define event_COUNT 2
};

struct extension {
    const char *name;
    size_t namelen;
    const struct request *subrequests;
    unsigned char numsubrequests;
    const struct event *events;
    unsigned char numevents;
    const char * const *errors;
    unsigned char numerrors;
    unsigned short numxgevents;
    const struct event *xgevents;
};

struct parameter {
    // The offset within the event, request, reply or Struct this
    // applies to. If OFS_LATER it is after the last list item
    // in this parameter-list.
    size_t offse;
    // NULL means end of list
    const char *name;
    enum fieldtype {
        // signed endian specific:
        ft_INT8, ft_INT16, ft_INT32,
        // unsigned decimal endian specific:
        ft_UINT8, ft_UINT16, ft_UINT32,
        // unsigned hex endian specific:
        ft_CARD8, ft_CARD16, ft_CARD32,
        // enums (not in constant list is error):
        ft_ENUM8, ft_ENUM16, ft_ENUM32,
        // counts for following strings, lists, ...
        // value-mask for LISTofFormat
        ft_STORE8, ft_STORE16, ft_STORE32,
        // to be ft_GET later into the store register
        ft_PUSH8, ft_PUSH16, ft_PUSH32,
        // bitfields: multiple values are possible
        ft_BITMASK8, ft_BITMASK16, ft_BITMASK32,
        // Different forms of lists:
        //	- boring ones
        ft_STRING8, ft_LISTofCARD32, ft_LISTofATOM,
        ft_LISTofCARD8, ft_LISTofCARD16,
        ft_LISTofUINT8, ft_LISTofUINT16,
        ft_LISTofUINT32,
        ft_LISTofINT8, ft_LISTofINT16,
        ft_LISTofINT32,
        //	- one of the above depening on last FORMAT
        ft_LISTofFormat,
        //	- iterate of list description in constants field
        ft_LISTofStruct,
        //	- same but length is mininum length and
        //	  actual length is taken from end of last list
        //	  or LASTMARKER, unless there is a SIZESET
        ft_LISTofVarStruct,
        //	- like ENUM for last STORE, but constants
        //	  are of type (struct value*) interpreteted at this
        //	  offset
        ft_LISTofVALUE,
        // an LISTofStruct with count = 1
        ft_Struct,
        // specify bits per item for LISTofFormat
        ft_FORMAT8,
        // an event
        // (would have also been possible with Struct and many IF)
        ft_EVENT,
        // jump to other parameter list if matches
        ft_IF8,
        ft_IF16,
        ft_IF32,
        // jump to other parameter list if matches atom name
        ft_IFATOM,
        // set end of last list manually, (for LISTofVarStruct)
        ft_LASTMARKER,
        // set the end of the current context, also change length
        // of a VarStruct:
        ft_SET_SIZE,
        // a ft_CARD32 looking into the ATOM list
        ft_ATOM,
        // always big endian
        ft_BE32,
        // get the #ofs value from the stack. (0 is the last pushed)
        ft_GET,
        // a fixed-point number 16+16 bit
        ft_FIXED,
        // a list of those
        ft_LISTofFIXED,
        // a fixed-point number 32+32 bit
        ft_FIXED3232,
        // a list of those
        ft_LISTofFIXED3232,
        // a 32 bit floating pointer number
        ft_FLOAT32,
        // a list of those
        ft_LISTofFLOAT32,
        // fraction with nominator and denominator 16 bit
        ft_FRACTION16_16,
        // dito 32 bit
        ft_FRACTION32_32,
        // nominator is unsigned
        ft_UFRACTION32_32,
        // a 64 bit number consisting of first the high 32 bit, then
        // the low 32 bit
        ft_INT32_32,
        // decrement stored value by specific value
        ft_DECREMENT_STORED,
        ft_DIVIDE_STORED,
        // set stored value to specific value
        ft_SET
    } type;
    union parameter_option {
        // for integers and fields of integers
        const struct constant *constants;
        // for IFs, Structs, ...
        const struct parameter *parameters;
        // for LISTofVALUE
        const struct value *values;
    } o;
};
struct value {
    unsigned long flag;
    // NULL means EndOfValues
    const char *name;
    // only elementary type (<= ft_BITMASK32 are allowed ),
    enum fieldtype type;
    const struct constant *constants;
};

extern const struct request *requests;
extern size_t num_requests;
extern const struct event *events;
extern size_t num_events;
extern const char * const *errors;
extern size_t num_errors;
extern const struct extension *extensions;
extern size_t num_extensions;
extern const struct parameter *unexpected_reply;
extern const struct parameter *setup_parameters;
*/

static const char* request_func_name(request_func* rf) {
    if (rf == requestQueryExtension ) {
        return "requestQueryExtension";
    } else if (rf == requestInternAtom ) {
        return "requestInternAtom";
    } else if (rf == requestGetAtomName ) {
        return "requestGetAtomName";
    } else if ( rf == NULL ) {
        return NULL;
    }
    return "(unknown)";
}

static const char* reply_func_name(reply_func* rf) {
    if (rf == replyListFontsWithInfo ) {
        return "replyListFontsWithInfo";
    } else if (rf == replyQueryExtension ) {
        return "replyQueryExtension";
    } else if (rf == replyInternAtom ) {
        return "replyInternAtom";
    } else if (rf == replyGetAtomName ) {
        return "replyGetAtomName";
    } else if ( rf == NULL ) {
        return NULL;
    }
    return "(unknown)";
}

static const char* fieldtype_name(const int ft) {
    assert(ft_INT8 == 0);
    assert(ft >= ft_INT8 && ft <= ft_SET);
    static const char* fieldtype_names[] = {
        // signed endian specific:
        "ft_INT8", "ft_INT16", "ft_INT32",
        // unsigned decimal endian specific:
        "ft_UINT8", "ft_UINT16", "ft_UINT32",
        // unsigned hex endian specific:
        "ft_CARD8", "ft_CARD16", "ft_CARD32",
        // enums (not in constant list is error):
        "ft_ENUM8", "ft_ENUM16", "ft_ENUM32",
        // counts for following strings, lists, ...
        // value-mask for LISTofFormat
        "ft_STORE8", "ft_STORE16", "ft_STORE32",
        // to be "ft_GET" later into the store register
        "ft_PUSH8", "ft_PUSH16", "ft_PUSH32",
        // bitfields: multiple values are possible
        "ft_BITMASK8", "ft_BITMASK16", "ft_BITMASK32",
        // Different forms of lists:
        //	- boring ones
        "ft_STRING8", "ft_LISTofCARD32", "ft_LISTofATOM",
        "ft_LISTofCARD8", "ft_LISTofCARD16",
        "ft_LISTofUINT8", "ft_LISTofUINT16",
        "ft_LISTofUINT32",
        "ft_LISTofINT8", "ft_LISTofINT16",
        "ft_LISTofINT32",
        //	- one of the above depening on last FORMAT
        "ft_LISTofFormat",
        //	- iterate of list description in constants field
        "ft_LISTofStruct",
        //	- same but length is mininum length and
        //	  actual length is taken from end of last list
        //	  or LASTMARKER, unless there is a SIZESET
        "ft_LISTofVarStruct",
        //	- like ENUM for last STORE, but constants
        //	  are of type (struct value*) interpreteted at this
        //	  offset
        "ft_LISTofVALUE",
        // an LISTofStruct with count = 1
        "ft_Struct",
        // specify bits per item for LISTofFormat
        "ft_FORMAT8",
        // an event
        // (would have also been possible with Struct and many IF)
        "ft_EVENT",
        // jump to other parameter list if matches
        "ft_IF8",
        "ft_IF16",
        "ft_IF32",
        // jump to other parameter list if matches atom name
        "ft_IFATOM",
        // set end of last list manually, (for LISTofVarStruct)
        "ft_LASTMARKER",
        // set the end of the current context, also change length
        // of a VarStruct:
        "ft_SET_SIZE",
        // a "ft_CARD32" looking into the ATOM list
        "ft_ATOM",
        // always big endian
        "ft_BE32",
        // get the #ofs value from the stack. (0 is the last pushed)
        "ft_GET",
        // a fixed-point number 16+16 bit
        "ft_FIXED",
        // a list of those
        "ft_LISTofFIXED",
        // a fixed-point number 32+32 bit
        "ft_FIXED3232",
        // a list of those
        "ft_LISTofFIXED3232",
        // a 32 bit floating pointer number
        "ft_FLOAT32",
        // a list of those
        "ft_LISTofFLOAT32",
        // fraction with nominator and denominator 16 bit
        "ft_FRACTION16_16",
        // ditto 32 bit
        "ft_FRACTION32_32",
        // nominator is unsigned
        "ft_UFRACTION32_32",
        // a 64 bit number consisting of first the high 32 bit, then
        // the low 32 bit
        "ft_INT32_32",
        // decrement stored value by specific value
        "ft_DECREMENT_STORED",
        "ft_DIVIDE_STORED",
        // set stored value to specific value
        "ft_SET"
    };
    return fieldtype_names[ft];
}

#define DUMP_FILENAME "final_parse_state.txt"
#define TAB4 "    "

void final_parse_state_dump(void) {
    FILE *out_fs = fopen(DUMP_FILENAME, "w");
    if ( out_fs == NULL ) {
        fprintf( stderr,
                 "%s fopen(\"%s\", \"%s\"): %d=%s\n",
                 __func__, DUMP_FILENAME, "w", errno, strerror(errno) );
        return;
    }

    fprintf( out_fs, "parse state after `finalize_everything(parser)`:\n");
    fprintf( out_fs, "\n");

    /* extern const struct request *requests; */
    /* extern size_t num_requests; */
    fprintf( out_fs, "num_requests: %lu\n", num_requests );
    for (size_t i = 0; i < num_requests; ++i) {
        struct request req = requests[i];
        fprintf( out_fs, "requests[%lu] {\n", i );
        fprintf( out_fs, TAB4 "name: %s\n", req.name );
        // const struct parameter *parameters;
        // unknownrequest, or parameters_finalize(namespace->request.request)
        // note parse.c print_parameters
        fprintf( out_fs, TAB4 "parameters: {\n" );
        for ( const struct parameter* param = req.parameters;
              param != NULL && param->name != NULL; ++param ) {
            fprintf( out_fs, TAB4 TAB4 "{\n" );
            fprintf( out_fs, TAB4 TAB4 TAB4 "name: %s\n", param->name );
            fprintf( out_fs, TAB4 TAB4 TAB4 "type: %s\n", fieldtype_name(param->type) );
            fprintf( out_fs, TAB4 TAB4 "}\n" );
        }
        fprintf( out_fs, TAB4 "}\n" );
        // const struct parameter *answers;
        fprintf( out_fs, TAB4 "answers: {\n" );
        for ( const struct parameter* ans = req.answers;
              ans != NULL && ans->name != NULL; ++ans ) {
            fprintf( out_fs, TAB4 TAB4 "{\n" );
            fprintf( out_fs, TAB4 TAB4 TAB4 "name: %s\n", ans->name );
            fprintf( out_fs, TAB4 TAB4 TAB4 "type: %s\n", fieldtype_name(ans->type) );
            fprintf( out_fs, TAB4 TAB4 "}\n" );
        }
        fprintf( out_fs, TAB4 "}\n" );
        // unknownresponse, or parameters_finalize(namespace->request.response)
        fprintf( out_fs, TAB4 "request_func: %s\n", request_func_name(req.request_func) );
        fprintf( out_fs, TAB4 "reply_func: %s\n", reply_func_name(req.reply_func) );
        fprintf( out_fs, TAB4 "record_variables: %i  // stack values to be transferred to the reply code\n",
                 req.record_variables );
        fprintf( out_fs, "}\n" );
    }

    /* extern const struct event *events; */
    /* extern size_t num_events; */
    /* extern const char * const *errors; */
    /* extern size_t num_errors; */
    /* extern const struct extension *extensions; */
    /* extern size_t num_extensions; */
    /* extern const struct parameter *unexpected_reply; */
    /* extern const struct parameter *setup_parameters; */
    return;
}
