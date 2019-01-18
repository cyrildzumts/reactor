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


ConcreteService::ConcreteService()

{
   service_resource_usage = 0;
}


int ConcreteService::process_request(int request, int delay)
{

    if(delay > PROCESSING_DURATION || delay < 0){
        LOG("Service DELAY ", delay);
        throw ServiceError("Service: Bad delay argument: " + std::to_string(delay));
    }
    // simulate the time required to process the request
    std::this_thread::sleep_for(duration_ms_t(delay));

    return delay;
}




int ConcreteService::operator ()(int request, int delay)
{
    // simulate the time required to process the request
    std::this_thread::sleep_for(duration_ms_t(delay));
    return delay;
}

int job(int req, int delay)
{
    /*
     * this function sleeps to simulate a processing duration
     */

    if(delay > PROCESSING_DURATION || delay < 0 ){
        throw ServiceError("Service: Bad delay argument:  " + std::to_string(delay));
    }
   //std::cout <<"Job Request : " << req<< " - delay :" << delay << TIME_UNIT <<std::endl;
    std::this_thread::sleep_for(duration_ms_t(delay));
    return delay + req;
}

Http::Http()
{
    buffer.data = static_cast<char*>(malloc(1));
    buffer.size =0;

    curl = curl_easy_init();
    info = curl_version_info(CURLVERSION_NOW);
    setOption(CURLOPT_FOLLOWLOCATION, 1L);
    setOption(CURLOPT_WRITEDATA, static_cast<void*>(&buffer));
    setOption(CURLOPT_WRITEFUNCTION, write_callback);
    setOption(CURLOPT_USERAGENT, "circuit-breaker-agent/1.0");
    if(curl){
        LOG("Http object initialized");
    }
    else {
        LOG("Error : Http curl not initialuzed");
        exit(EXIT_FAILURE);
    }
}

Http::Http(const std::string &url):Http()
{
    this->url = url;
}

Http::~Http()
{
    if(curl)
        curl_easy_cleanup(curl);
//    if(info){
//        curl_free(info);
//    }
    if(buffer.data)
        delete buffer.data;
    curl_global_cleanup();
}

CURLcode Http::fetch(const std::string &url)
{
    CURLcode code = CURLE_CHUNK_FAILED;
    Datapointer buffer;
    buffer.size = 0;
    CURL *_curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA, static_cast<void*>(&buffer));
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl,CURLOPT_USERAGENT, "circuit-breaker-agent/1.0");
    if(curl && !url.empty()){
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        //LOG("CURL URL : ", url);
        code = curl_easy_perform(curl);
        if(code == CURLE_FAILED_INIT){
            LOG("CHECK CURL init: curl init failed");
            exit(EXIT_FAILURE);
        }
        LOG("CURL - Received ", buffer.size, " bytes");
        //std::cout <<" Received " << buffer.size << " bytes\n";
        //std::cout << "Received Data : \n" << buffer.data << std::endl;
        free(buffer.data);
        buffer.size =0;

    }
    curl_easy_cleanup(curl);
    return code;
}

CURLcode Http::fetch_default()
{
    return fetch(url);
}

std::string Http::curl_version() const
{
    return std::string(info->version);
}

Http* Http::intance(){
    static Http _instance = Http();
    return &_instance;
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

void Http::install_write_callback()
{

}



size_t write_callback(char *contents, size_t size, size_t nmemb, void *userdata)
{
    size_t real_size = size * nmemb;
    //Datapointer *dptr = static_cast<Datapointer*>(userdata);
    /*char *ptr = static_cast<char*>(realloc(dptr->data, dptr->size + real_size +1));
    if(ptr == nullptr){
        std::cout << "Out of memory : realloc returned null" << std::endl;
        return 0;
    }
    */
    //LOG("Curl received : ", real_size);
    /*
    dptr->data = ptr;
    memcpy(&(dptr->data[dptr->size]), contents, real_size);
    dptr->size += real_size;
    dptr->data[dptr->size] = 0;
    */
    return real_size;
}
