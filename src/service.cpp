#include "service.h"
#include <time.h>
#include <iostream>
#include <thread>
#include <chrono>


ServiceError::ServiceError():std::runtime_error("BAD REQUEST"){
    what_string = std::string("BAD REQUEST");
}

ServiceError::ServiceError(const char* what_args):std::runtime_error(what_args){
    what_string = std::string(what_args);
}

ServiceError::ServiceError(const std::string& what_args):std::runtime_error(what_args){
    what_string = std::string(what_args);
}

const char* ServiceError::what()const noexcept{
    return what_string.c_str();
}


int job(const int &req, const int &delay)
{
    /*
     * this function sleeps to simulate a processing duration
     */

    if(delay > PROCESSING_DURATION || delay < 0 ){
        throw ServiceError("Service: Bad delay argument:  " + std::to_string(delay));
    }
    std::this_thread::sleep_for(duration_ms_t(delay));
    return delay + req;
}

CURLcode http_job(const std::string &url)
{

    CURLcode code = CURLE_CHUNK_FAILED;
    //thread_local Datapointer buffer;
    //buffer.size = 0;
    CURL *curl = curl_easy_init();
    if(!url.empty()){
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl,CURLOPT_WRITEDATA, nullptr);
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl,CURLOPT_USERAGENT, "circuit-breaker-agent/1.0");
        code = curl_easy_perform(curl);
        if(code == CURLE_FAILED_INIT){
            LOG("CHECK CURL init: curl init failed");
            exit(EXIT_FAILURE);
        }
        /*
        else{
            LOG("CURL - Received ", buffer.size, " bytes");
            //std::cout << "Received Data : \n" << buffer.data << std::endl;
            free(buffer.data);
            buffer.size =0;
        }
        */
    }
    curl_easy_cleanup(curl);
    return code;
}


size_t write_callback(char *contents, size_t size, size_t nmemb, void *userdata)
{
    size_t real_size = size * nmemb;
    /*Datapointer *dptr = static_cast<Datapointer*>(userdata);
    char *ptr = static_cast<char*>(realloc(dptr->data, dptr->size + real_size +1));
    if(ptr == nullptr){
        std::cout << "Out of memory : realloc returned null" << std::endl;
        return 0;
    }

    //LOG("Curl received : ", real_size);

    dptr->data = ptr;
    memcpy(&(dptr->data[dptr->size]), contents, real_size);
    dptr->size += real_size;
    dptr->data[dptr->size] = 0;
    */
    return real_size;
}
