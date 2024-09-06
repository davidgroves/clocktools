// gcc -g -Wall set_tai_offset.c -o set_tai_offset -lrt -lcurl

#include <sys/timex.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define TAI_OFFSET_URL "https://data.iana.org/time-zones/tzdb/leap-seconds.list"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        return 0;  // out of memory
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int fetch_and_count_lines(const char *url) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  // will be grown as needed by the realloc above
    chunk.size = 0;    // no data at this point

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
        return -1;
    }

    curl_easy_cleanup(curl_handle);

    int line_count = 0;
    char *token = strtok(chunk.memory, "\n");
    while (token != NULL) {
        if (token[0] != '#') {
            line_count++;
        }
        token = strtok(NULL, "\n");
    }

    free(chunk.memory);
    curl_global_cleanup();

    return line_count;
}

/* Function to set the TAI offset */
int set_tai_offset(int offset) {
    struct timex tx;
    int ret;

    memset(&tx, 0, sizeof(tx));
    tx.modes = ADJ_TAI;
    tx.constant = offset;

    ret = adjtimex(&tx);
    if (ret == -1) {
        perror("adjtimex");
        exit(1);
    }

    printf("TAI offset set successfully to %d.\n", offset);
    return ret;
}

int show_tai_offset() {
    struct timex tx;
    int ret;
    memset(&tx, 0, sizeof(tx));
    ret = adjtimex(&tx);
    if (ret == -1) {
        perror("adjtimex");
        exit(1);
    }
    printf("Current TAI offset is %d.\n", tx.tai);
    return ret;
}

int main(int argc, char* argv[]) {

    /* Check for correct number of arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <offset|-r|-s>\n", argv[0]);
        return 1;
    }

    int offset;
    if (strcmp(argv[1], "-s") == 0) {
        return show_tai_offset();
    }

    if (strcmp(argv[1], "-r") == 0) {
        /* 
        Add 9 to the offset to account for leap seconds between 1958 and 1970.
        1958 is the start of the TAI epoch.
        1970 is the start of the UNIX epoch.
        */
        offset = fetch_and_count_lines(TAI_OFFSET_URL) + 9;
        if (offset == -1) {
            return 1;
        }
    } else {
        offset = atoi(argv[1]);
    }

    return set_tai_offset(offset);
}