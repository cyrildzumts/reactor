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
    buffer.data = new char;
    buffer.size = 0;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    info = curl_version_info(CURLVERSION_NOW);
    setOption(CURLOPT_FOLLOWLOCATION, 1L);
    setOption(CURLOPT_WRITEDATA, static_cast<void*>(&buffer));
    setOption(CURLOPT_WRITEFUNCTION, write_callback);
    setOption(CURLOPT_USERAGENT, "circuit-breaker-agent/1.0");

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
    delete buffer.data;
    curl_global_cleanup();
}

CURLcode Http::fetch(const std::string &url)
{
    CURLcode code;
    if(curl && !url.empty()){
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        code = curl_easy_perform(curl);
        std::cout <<" Received " << buffer.size << " bytes\n";
        //std::cout << "Received Data : \n" << buffer.data << std::endl;
        free(buffer.data);
        buffer.data = new char;
        buffer.size =0;

    }
    //curl_easy_cleanup(curl);
    return code;
}

CURLcode Http::fetch()
{
    return fetch(url);
}

std::string Http::curl_version() const
{
    return std::string(info->version);
}

void Http::install_write_callback()
{

}



size_t write_callback(char *contents, size_t size, size_t nmemb, void *userdata)
{
    size_t real_size = size * nmemb;
    Datapointer *dptr = static_cast<Datapointer*>(userdata);
    char *ptr = static_cast<char*>(realloc(dptr->data, dptr->size + real_size +1));
    if(ptr == nullptr){
        std::cout << "Out of memory : realloc returned null" << std::endl;
        return 0;
    }

    dptr->data = ptr;
    memcpy(&(dptr->data[dptr->size]), contents, real_size);
    dptr->size += real_size;
    dptr->data[dptr->size] = 0;
    return real_size;
}
