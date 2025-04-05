#include <stdio.h>     // fopen fprintf
#include <string.h>    // strerror
#include <errno.h>
#include <stdbool.h>   // needed for parse.h
#include <assert.h>
#include <stdint.h>

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

#define DUMP_FILENAME "final_parse_state.txt"
#define TAB2 "  "
#define TAB4 "    "
#define TABx1 TAB2
#define TABx2 TABx1 TABx1
#define TABx3 TABx2 TABx1
#define TABx4 TABx3 TABx1
#define TABx5 TABx4 TABx1
#define TABx6 TABx5 TABx1
#define TABx7 TABx6 TABx1
#define TABx8 TABx7 TABx1
#define TABx9 TABx8 TABx1

static const char* base_indent(const unsigned int base_tab_ct) {
    assert( base_tab_ct < 10 );
    switch ( base_tab_ct ) {
    case 1: return TABx1;
    case 2: return TABx2;
    case 3: return TABx3;
    case 4: return TABx4;
    case 5: return TABx5;
    case 6: return TABx6;
    case 7: return TABx7;
    case 8: return TABx8;
    case 9: return TABx9;
    }
    return "";
}

static unsigned int field_width(unsigned int max) {
    unsigned int pow10 = 0;
    for (; max; max /= 10) {
        ++pow10;
    }
    return pow10;
}

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

static void fprint_parameter(FILE* ofs, const unsigned int base_tab_ct,
                             const struct parameter* param) {
    assert(ofs != NULL);
    assert(param != NULL);
    const char* _base_indent = base_indent(base_tab_ct);
    fprintf( ofs, "%s{\n",
             _base_indent );
    fprintf( ofs, TABx1 "%sname: %s\n",
             _base_indent, param->name );
    fprintf( ofs, TABx1 "%stype: %s\n",
             _base_indent, fieldtype_name(param->type) );
    if ( param->o.constants == NULL ||
         param->o.parameters == NULL ||
         param->o.values == NULL ) {  // intentionally redundant check of union
        goto close_parameter;
    }
    if (param->type >= ft_INT8 && param->type <= ft_BITMASK32) {
        fprintf( ofs, TABx1 "%so.constants: {\n", _base_indent );
        for (const struct constant* _const = param->o.constants;
             _const != NULL && _const->name != NULL; ++_const) {
            fprintf( ofs, TABx2 "%s{\n", _base_indent );
            switch (param->type) {
            case ft_INT8:
            case ft_INT16:
            case ft_INT32:
                fprintf( ofs, TABx3 "%svalue: %li\n", _base_indent, _const->value );
                break;
            case ft_UINT8:
            case ft_UINT16:
            case ft_UINT32:
                fprintf( ofs, TABx3 "%svalue: %lu\n", _base_indent, _const->value );
                break;
            case ft_CARD8:  // hex
            case ft_CARD16:
            case ft_CARD32:
                fprintf( ofs, TABx3 "%svalue: 0x%lx\n", _base_indent, _const->value );
                break;
            case ft_ENUM8:  // int
            case ft_ENUM16:
            case ft_ENUM32:
                fprintf( ofs, TABx3 "%svalue: %li\n", _base_indent, _const->value );
                break;
            /* case ft_STORE8: */
            /* case ft_STORE16: */
            /* case ft_STORE32: */
            /*     break; */
            /* case ft_PUSH8: */
            /* case ft_PUSH16: */
            /* case ft_PUSH32: */
            /*     break; */
            case ft_BITMASK8:
            case ft_BITMASK16:
            case ft_BITMASK32:
                fprintf( ofs, TABx3 "%svalue: 0x%lx\n", _base_indent, _const->value );
                break;
            default:
                break;
            }
            fprintf( ofs, TABx3 "%sname: %s\n", _base_indent, _const->name );
            fprintf( ofs, TABx2 "%s}\n", _base_indent );
        }
        fprintf( ofs, TABx1 "%s}\n", _base_indent );
    } else if (false) {
    }
    switch (param->type) {
        // Different forms of lists:
        //	- boring ones
    case ft_STRING8:  // tl
        break;
    case ft_LISTofCARD32:  // tl
        break;
    case ft_LISTofATOM:  // tl
        break;
    case ft_LISTofCARD8:  // tl
        break;
    case ft_LISTofCARD16:  // tl
        break;
    case ft_LISTofUINT8:
        break;
    case ft_LISTofUINT16:
        break;
    case ft_LISTofUINT32:
        break;
    case ft_LISTofINT8:
        break;
    case ft_LISTofINT16:
        break;
    case ft_LISTofINT32:
        break;
        //	- one of the above depening on last FORMAT
    case ft_LISTofFormat:  // tl
        break;
        //	- iterate of list description in constants field
    case ft_LISTofStruct:  // tl
        break;
        //	- same but length is mininum length and
        //	  actual length is taken from end of last list
        //	  or LASTMARKER, unless there is a SIZESET
    case ft_LISTofVarStruct:  // tl
        break;
        //	- like ENUM for last STORE, but constants
        //	  are of type (struct value*) interpreteted at this
        //	  offset
    case ft_LISTofVALUE:
        fprintf( ofs, TABx1 "%so.values: {\n", _base_indent );
        for (const struct value* val = param->o.values;
             val != NULL && val->name != NULL; ++val) {
            fprintf( ofs, TABx2 "%s{\n", _base_indent );
            fprintf( ofs, TABx3 "%sflag: 0x%lx\n", _base_indent, val->flag );
            fprintf( ofs, TABx3 "%sname: %s\n", _base_indent, val->name );
            assert(val->type <= ft_BITMASK32);
            fprintf( ofs, TABx3 "%stype: %s\n", _base_indent, fieldtype_name(val->type) );
            fprintf( ofs, TABx3 "%sconstants: {\n", _base_indent );
            for (const struct constant* _const = val->constants;
                 _const != NULL && _const->name != NULL; ++_const) {
                fprintf( ofs, TABx4 "%s{\n", _base_indent );
                switch (val->type) {
                case ft_INT8:
                case ft_INT16:
                case ft_INT32:
                    fprintf( ofs, TABx5 "%svalue: %li\n", _base_indent, _const->value );
                    break;
                case ft_UINT8:
                case ft_UINT16:
                case ft_UINT32:
                    fprintf( ofs, TABx5 "%svalue: %lu\n", _base_indent, _const->value );
                    break;
                case ft_CARD8:   // hex
                case ft_CARD16:
                case ft_CARD32:
                    fprintf( ofs, TABx5 "%svalue: 0x%lx\n", _base_indent, _const->value );
                    break;
                case ft_ENUM8:   // int
                case ft_ENUM16:
                case ft_ENUM32:
                    fprintf( ofs, TABx5 "%svalue: %li\n", _base_indent, _const->value );
                    break;
                /* case ft_STORE8: */
                /* case ft_STORE16: */
                /* case ft_STORE32: */
                /*     break; */
                /* case ft_PUSH8: */
                /* case ft_PUSH16: */
                /* case ft_PUSH32: */
                /*     break; */
                case ft_BITMASK8:
                case ft_BITMASK16:
                case ft_BITMASK32:
                    fprintf( ofs, TABx5 "%svalue: 0x%lx\n", _base_indent, _const->value );
                    break;
                default:
                    break;
                }
                fprintf( ofs, TABx5 "%sname: %s\n", _base_indent, _const->name );
                fprintf( ofs, TABx4 "%s}\n", _base_indent );
            }
            fprintf( ofs, TABx3 "%s}\n", _base_indent );
            fprintf( ofs, TABx2 "%s}\n", _base_indent );
        }
        fprintf( ofs, TABx1 "%s}\n", _base_indent );
        break;
        // an LISTofStruct with count = 1
    case ft_Struct:  // tl
        break;
        // specify bits per item for LISTofFormat
    case ft_FORMAT8:  // tl
        break;
        // an event
        // (would have also been possible with Struct and many IF)
    case ft_EVENT:  // tl
        break;
        // jump to other parameter list if matches
    case ft_IF8:
        break;
    case ft_IF16:
        break;
    case ft_IF32:
        break;
        // jump to other parameter list if matches atom name
    case ft_IFATOM:  // tl
        break;
        // set end of last list manually, (for LISTofVarStruct)
    case ft_LASTMARKER:
        break;
        // set the end of the current context, also change length
        // of a VarStruct:
    case ft_SET_SIZE:
        // a ft_CARD32 looking into the ATOM list
    case ft_ATOM:  // tl
        break;
        // always big endian
    case ft_BE32:
        break;
        // get the #ofs value from the stack. (0 is the last pushed)
    case ft_GET:
        break;
        // a fixed-point number 16+16 bit
    case ft_FIXED:
        break;
        // a list of those
    case ft_LISTofFIXED:
        break;
        // a fixed-point number 32+32 bit
    case ft_FIXED3232:
        break;
        // a list of those
    case ft_LISTofFIXED3232:
        break;
        // a 32 bit floating pointer number
    case ft_FLOAT32:
        break;
        // a list of those
    case ft_LISTofFLOAT32:
        break;
        // fraction with numerator and denominator 16 bit
    case ft_FRACTION16_16:  // tl
        // dito 32 bit
    case ft_FRACTION32_32:
        // nominator is unsigned
    case ft_UFRACTION32_32:
        // a 64 bit number consisting of first the high 32 bit, then
        // the low 32 bit
    case ft_INT32_32:
        // decrement stored value by specific value
    case ft_DECREMENT_STORED:
    case ft_DIVIDE_STORED:
        // set stored value to specific value
    case ft_SET:
        break;

    }
close_parameter:
    fprintf( ofs, "%s}\n", _base_indent );
}

static void fprint_requests(FILE* ofs, const unsigned int base_tab_ct,
                            const struct request* requests,
                            const unsigned int request_ct,
                            const char* request_ct_varname) {
    assert(ofs != NULL);
    assert(requests != NULL);
    assert(request_ct_varname != NULL);
    const char* _base_indent = base_indent(base_tab_ct);
    fprintf( ofs, "%s%s: %u\n",
             _base_indent, request_ct_varname, request_ct );
    fprintf( ofs, "%srequests {\n",
             _base_indent );
    for (unsigned int i = 0; i < request_ct; ++i) {
        const struct request req = requests[i];
        fprintf( ofs, TABx1 "%s[%u] {\n",
                 _base_indent, i );
        fprintf( ofs, TABx2 "%sname: %s\n",
                 _base_indent, req.name );
        // note parse.c print_parameters
        fprintf( ofs, TABx2 "%sparameters: {\n",
                 _base_indent );
        for ( const struct parameter* param = req.parameters;
              param != NULL && param->name != NULL; ++param ) {
            fprint_parameter(ofs, base_tab_ct + 3, param);
        }
        fprintf( ofs, TABx2 "%s}\n",
                 _base_indent );
        fprintf( ofs, TABx2 "%sanswers: {\n",
                 _base_indent );
        for ( const struct parameter* ans = req.answers;
              ans != NULL && ans->name != NULL; ++ans ) {
            fprint_parameter(ofs, base_tab_ct + 3, ans);
        }
        fprintf( ofs, TABx2 "%s}\n",
                 _base_indent );
        fprintf( ofs, TABx2 "%srequest_func: %s\n",
                 _base_indent, request_func_name(req.request_func) );
        fprintf( ofs, TABx2 "%sreply_func: %s\n",
                 _base_indent, reply_func_name(req.reply_func) );
        fprintf( ofs, TABx2 "%srecord_variables: %i  // stack values to be transferred to the reply code\n",
                 _base_indent, req.record_variables );
        fprintf( ofs, TABx1 "%s}\n",
                 _base_indent );
    }
    fprintf( ofs, "%s}\n",
             _base_indent );
}

static void fprint_events(FILE* ofs, const unsigned int base_tab_ct,
                          const struct event* events,
                          const unsigned int event_ct,
                          const char* event_ct_varname) {
    assert(ofs != NULL);
    assert(events != NULL);
    assert(event_ct_varname != NULL);
    const char* _base_indent = base_indent(base_tab_ct);
    fprintf( ofs, "%s%s: %u\n",
             _base_indent, event_ct_varname, event_ct );
    fprintf( ofs, "%sevents {\n",
             _base_indent );
    for (unsigned int i = 0; i < num_events; ++i) {
        const struct event evnt = events[i];
        fprintf( ofs, TABx1 "%s[%u] {\n",
                 _base_indent, i );
        fprintf( ofs, TABx2 "%sname: %s\n",
                 _base_indent, evnt.name );
        fprintf( ofs, TABx2 "%sparameters: {\n",
                 _base_indent );
        for ( const struct parameter* param = evnt.parameters;
              param != NULL && param->name != NULL; ++param ) {
            fprint_parameter(ofs, base_tab_ct + 3, param);
        }
        fprintf( ofs, TABx2 "%s}\n",
                 _base_indent );
        fprintf( ofs, TABx2 "%stype: %s (%i)\n",
                 _base_indent,
                 evnt.type == 0 ? "event_normal" : "event_xge", evnt.type );
        fprintf( ofs, TABx1 "%s}\n",
                 _base_indent );
    }
    fprintf( ofs, "%s}\n",
             _base_indent );
}

static void fprint_errors(FILE* ofs, const unsigned int base_tab_ct,
                          const char* const* errors,
                          const unsigned int error_ct,
                          const char* error_ct_varname) {
    assert(ofs != NULL);
    assert(errors != NULL);
    assert(error_ct_varname != NULL);
    const char* _base_indent = base_indent(base_tab_ct);
    fprintf( ofs, "%s%s: %u\n",
             _base_indent, error_ct_varname, error_ct );
    fprintf( ofs, "%serrors {\n",
             _base_indent );
    for (unsigned int i = 0, iwidth = field_width(error_ct);
         i < num_errors; ++i) {
        const char* error = errors[i];
        fprintf( ofs, TABx1 "%s[%*u] %s\n",
                 _base_indent, (int)iwidth, i, error );
    }
    fprintf( ofs, "%s}\n",
             _base_indent );
}

static void fprint_extensions(FILE* ofs) {
    fprintf( ofs, "num_extensions: %lu\n", num_extensions );
    for (size_t i = 0; i < num_extensions; ++i) {
        const struct extension ext = extensions[i];
        fprintf( ofs, "extensions[%lu] {\n", i );
        fprintf( ofs, TABx1 "name: %s\n", ext.name );
        fprintf( ofs, TABx1 "namelen: %lu\n", ext.namelen );
        /* fprint_requests( ofs, 1, ext.subrequests, ext.numsubrequests, "numsubrequests" ); */
        /* fprint_events( ofs, 1, ext.events, ext.numevents, "numevents" ); */
        /* fprint_errors( ofs, 1, ext.errors, ext.numerrors, "numerrors" ); */
        /* fprint_events( ofs, 1, ext.xgevents, ext.numxgevents, "numxgevents" ); */
        fprintf( ofs, "}\n" );
    }
}

void final_parse_state_dump(void) {
    FILE *ofs = fopen(DUMP_FILENAME, "w");
    if ( ofs == NULL ) {
        fprintf( stderr,
                 "%s fopen(\"%s\", \"%s\"): %d=%s\n",
                 __func__, DUMP_FILENAME, "w", errno, strerror(errno) );
        return;
    }

    fprintf( ofs, "parse state after `finalize_everything(parser)`:\n");
    fprintf( ofs, "\n");

    /* extern const struct request *requests; */
    /* extern size_t num_requests; */
    fprint_requests( ofs, 0, requests, num_requests, "num_requests" );
    fprintf( ofs, "\n\n" );

    /* extern const struct event *events; */
    /* extern size_t num_events; */
    fprint_events( ofs, 0, events, num_events, "num_events" );
    fprintf( ofs, "\n\n" );

    /* extern const char* const* errors; */
    /* extern size_t num_errors; */
    fprint_errors( ofs, 0, errors, num_errors, "num_errors" );
    fprintf( ofs, "\n\n" );

    /* extern const struct extension *extensions; */
    /* extern size_t num_extensions; */
    fprint_extensions(ofs);
    fprintf( ofs, "\n\n" );

    /* extern const struct parameter *unexpected_reply; */

    /* extern const struct parameter *setup_parameters; */

    fclose( ofs );
    return;
}
