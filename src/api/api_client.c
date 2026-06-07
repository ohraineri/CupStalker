#include "api_client.h"

#include "../../config.h"
#include "../util/logger.h"
#include "../util/memory.h"

#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

static size_t append_response_chunk(char *chunk, size_t size, size_t count, void *user_data)
{
    size_t chunk_size = size * count;
    ApiResponse *response = user_data;

    char *grown = memory_realloc(response->data, response->length + chunk_size + 1);
    response->data = grown;

    memcpy(response->data + response->length, chunk, chunk_size);
    response->length += chunk_size;
    response->data[response->length] = '\0';

    return chunk_size;
}

Result api_client_global_init(void)
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        return RESULT_ERROR("curl_global_init failed");
    }
    return RESULT_OK;
}

void api_client_global_cleanup(void)
{
    curl_global_cleanup();
}

static void build_matches_url(char *url, size_t url_size)
{
    snprintf(url, url_size, "%s/competitions/%d/matches", API_BASE_URL, COMPETITION_ID);
}

Result api_client_fetch_matches(ApiResponse *response)
{
    if (response == NULL) {
        return RESULT_ERROR("api_client_fetch_matches: response is NULL");
    }

    CURL *handle = curl_easy_init();
    if (handle == NULL) {
        return RESULT_ERROR("curl_easy_init failed");
    }

    ApiResponse buffer = { .data = NULL, .length = 0 };
    char url[512];
    build_matches_url(url, sizeof url);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "X-Auth-Token: " API_KEY);
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, append_response_chunk);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, (long)API_TIMEOUT_SECONDS);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "cup-stalker/1.0");

    CURLcode transfer = curl_easy_perform(handle);

    Result result = RESULT_OK;
    if (transfer != CURLE_OK) {
        memory_free((void **)&buffer.data);
        result = result_error(curl_easy_strerror(transfer));
    } else {
        long http_status = 0;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &http_status);
        if (http_status < 200 || http_status >= 300) {
            memory_free((void **)&buffer.data);
            char message[RESULT_MESSAGE_CAPACITY];
            snprintf(message, sizeof message, "API returned HTTP %ld", http_status);
            result = result_error(message);
        } else {
            logger_debug("fetched %zu bytes from %s", buffer.length, url);
            *response = buffer;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(handle);
    return result;
}

void api_response_free(ApiResponse *response)
{
    if (response == NULL) {
        return;
    }

    memory_free((void **)&response->data);
    response->length = 0;
}
