#ifndef CUP_STALKER_API_CLIENT_H
#define CUP_STALKER_API_CLIENT_H

#include "../util/result.h"

typedef struct {
    char  *data;
    size_t length;
} ApiResponse;

Result api_client_global_init(void);
void api_client_global_cleanup(void);
Result api_client_fetch_matches(ApiResponse *response);
void api_response_free(ApiResponse *response);

#endif
