#include <string.h>

#define ERROR_IMPORT
#include "Error.h"

const char* Error_toString(ErrorState state) {
    switch(state) {
        case ERR_SUCCESS:
            return "Succeeded";
        case ERR_FAIL:
            return "Failed";
        case ERR_FILE_NOT_FOUND:
            return "File not found";
        case ERR_FILE_FAILURE:
            return "Failed to read file";
        case ERR_OUT_OF_MEMORY:
            return "Out of memory";
        case ERR_HARDWARE_NOT_FOUND:
            return "Hardware not found";
        case ERR_HARDWARE_FAIL:
            return "Hardware failure";
        case ERR_WRONG_FILE_FORMAT:
            return "Wrong file format";
        case ERR_NULL_PTR:
            return "Pointer was null";
        case ERR_ILLEGAL_ARG:
            return "Argument not valid";
    }

    return "Wrong error code";
}