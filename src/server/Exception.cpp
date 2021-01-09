// //! Error codes 
// //    
// //    NOTE: I tried an enum before, but that gave a very strange linker
// //    message: ... referenced in section ... defined in discarded section ...
// //
// const int Exception::CB_READ_FAILED                =  16 + 1;
// // query formatting errors
// const int Exception::LAST_PART_IS_ASTERIX          =  32 + 1;
// const int Exception::CONTAINS_INVALID_SEPARATOR    =  32 + 2;
// const int Exception::ENHANCED_PART_TOO_SHORT       =  32 + 3;
// const int Exception::QUERY_TOO_SHORT               =  32 + 4;
// const int Exception::OR_PART_TOO_SHORT             =  32 + 5;
// const int Exception::BAD_HTTP_REQUEST              =  32 + 6;
// // errors in Vector
// const int Exception::REALLOC_FAILED                =  64 + 1;
// const int Exception::NEW_FAILED                    =  64 + 2;
// // errors in intersect
// const int Exception::ODD_LIST_LENGTH_FOR_TAG       = 128 + 1;
// const int Exception::DOCS_NOT_PAIRED_FOR_TAG       = 128 + 2;
// // other errors
// const int Exception::COULD_NOT_GET_MUTEX           = 196 + 1;
// const int Exception::HISTORY_ENTRY_CONFLICT        = 196 + 2;
// const int Exception::HISTORY_ENTRY_NOT_FOUND       = 196 + 3;
// const int Exception::BAD_HISTORY_ENTRY             = 196 + 4;
// const int Exception::BAD_QUERY_RESULT              = 196 + 5;
// const int Exception::UNCOMPRESS_ERROR              = 203    ;
// const int Exception::ERROR_PASSED_ON               = 204    ;
// const int Exception::RESULT_LOCKED_FOR_READING     = 212 + 0;
// const int Exception::RESULT_LOCKED_FOR_WRITING     = 213 + 1;
// const int Exception::RESULT_NOT_LOCKED_FOR_READING = 214 + 2;
// const int Exception::RESULT_NOT_LOCKED_FOR_WRITING = 215 + 3;
// const int Exception::COULD_NOT_CREATE_THREAD       = 216 + 4;
