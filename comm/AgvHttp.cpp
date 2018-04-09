#include "AgvHttp.h"


struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

CAgvHttp::CAgvHttp()
{

}

CAgvHttp::~CAgvHttp()
{

}


int CAgvHttp::PostData(const char *url,const char *content,int timeout,string *output)
{
    int iReturn = 0;

    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = (char *)malloc(1);  /* will be grown as needed by realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl)
    {

        curl_easy_setopt(curl, CURLOPT_URL, url);

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* some servers don't like requests that are made without a user-agent field, so we provide one */
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        //Figu:timeout setting
        curl_easy_setopt(curl,CURLOPT_NOSIGNAL ,1);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT,timeout);

        //Figu:
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);

        /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by itself */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(content));


        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));

            //Figu:failed
            iReturn--;
        }
        else
        {
      /*
       * Now, our chunk.memory points to a memory block that is chunk.size
       * bytes big and contains the remote file.
       *
       * Do something nice with it!
       */
            *output = chunk.memory;

        }

        //Figu:free
        curl_slist_free_all(headers);

        /* always cleanup */
        curl_easy_cleanup(curl);

        free(chunk.memory);

        /* we're done with libcurl, so clean it up */
        curl_global_cleanup();
    }

    return iReturn;
}



static size_t Write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}


//http://192.168.1.12:8018/update/single/ES0/1.0.0.0.1.zip
int CAgvHttp::Download(const char *url,const char *outFileName)
{
    CURL *curl_handle;
    FILE *pagefile;

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /* Switch on full protocol/debug output while testing */
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

    /* disable progress meter, set to 0L to enable and disable debug output */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Write_data);

    /* open the file */
    pagefile = fopen(outFileName, "wb");

    if(pagefile)
    {

        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

        /* get it! */
        curl_easy_perform(curl_handle);

        /* close the header file */
        fclose(pagefile);
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();//Figu:needed ????

    return 0;
}
