#ifndef ERROR_H
#define ERROR_H

#ifdef ERROR_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

typedef enum {
    ERR_SUCCESS,
    ERR_FAIL,
    ERR_FILE_NOT_FOUND,
    ERR_FILE_FAILURE,
    ERR_WRONG_FILE_FORMAT,
    ERR_OUT_OF_MEMORY,
    ERR_HARDWARE_NOT_FOUND,
    ERR_HARDWARE_FAIL,
    ERR_NULL_PTR,
    ERR_ILLEGAL_ARG
} ErrorState;

EXTERN const char* Error_toString(ErrorState state);

#undef ERROR_IMPORT
#undef EXTERN

#endif /* ERROR_H */