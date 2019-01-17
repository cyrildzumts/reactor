
#ifndef SERVICE_H
#define SERVICE_H

#include "common.h"
#include "generator.h"
#include <log.h>
#include <optional>
#include <exception>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>
#include <curl/curl.h>
#include <utility>
#include <type_traits>
#include <iostream>
#include <string>
#include <cstring>

#define URL_1 "http://www.mocky.io/v2/5c404307350000b02dec3c0e/?mocky-delay=500ms" // receive 21 Bytes
#define URL_2 "http://www.mocky.io/v2/5c405ffe0f00007408e7b3f9/?mocky-delay=500ms" // receive 52 Bytes
#define URL_3 "http://slowwly.robertomurray.co.uk/delay/500/url/https://example.com/" //receive 1270 Bytes

template<typename Callback, typename... Args>
struct typeof_t{
    using result_type = std::invoke_result_t<std::decay_t<Callback>, std::decay_t<Args>...>;
    using type = Callback(*)(Args...);
};


struct curl_callback_t {
    using result_type = size_t;
    using type = size_t(*)(char*, size_t, size_t, void*);
};

struct Datapointer{
    char *data;
    size_t size;
};
size_t write_callback(char *contents, size_t size, size_t nmemb, void *userdata);
int job(int req, int delay);



class ServiceError : public std::runtime_error{
public:
    enum Error {
        BAD_REQUEST = 1, UNAUTHORISED, SYSTEM, TOO_FAST_REQUEST
    };
    ServiceError(const char* what_args);
    ServiceError(const std::string& what_args);
    ServiceError();
    // Exception Interface
    virtual const char* what() const noexcept;

private:
    std::string what_string;

};

class Service
{
public:
    virtual ~Service(){}
    virtual int process_request(int request,int delay = PROCESSING_DURATION) = 0;
    virtual int operator()(int request, int delay = PROCESSING_DURATION) = 0;
};


class ConcreteService : public Service{
private:
    int service_resource_usage;
public:
    ConcreteService();
    // Service interface
public:
    /**
     * @brief process_request return the number from samples at the location indicated by request.
     * if that number is even, the service returns normally. If the number is odd, the
     * service throws a ServiceError Exception.
     * @param request : request >= 0 && request < samples.size()
     * @return the number located at the position indicated by request in samples.
     *   throws a ServiceError if request is invalide.
     *
     */
    virtual int process_request(int request, int delay = PROCESSING_DURATION);

    // Service interface
public:
    virtual int operator ()(int request, int delay = PROCESSING_DURATION) override;
};


class Http{
private:
    std::string url;
    CURL *curl;
    CURLcode code;
    curl_version_info_data *info;
    Datapointer buffer;

public:
    Http();
    Http(const std::string &url);
    ~Http();
    static Http* intance();
    CURLcode fetch(const std::string &url);
    CURLcode fetch_default();
    std::string curl_version() const;
    template<typename T,typename U,
             typename=std::enable_if_t<std::is_same_v<CURLoption,T>>,
             typename=std::enable_if_t<!std::is_pointer_v<T>>,
             typename=std::enable_if_t<!std::is_null_pointer_v<T>>,
             typename=std::enable_if_t<!std::is_null_pointer_v<U>>>
    CURLcode setOption(T&& first,U&& other){
        curl_easy_setopt(curl,std::forward<T>(first), std::forward<U>(other));
    }

    template<typename Callable, typename...Args,
             typename=std::enable_if_t<std::is_same_v<Callable,typeof_t<Callable,Args...>::type>>>
    CURLcode add_observer(Callable callback, Args&&... args){

    }


    void install_write_callback();

};

void http_init();

CURLcode http_job(const std::string &url);
#endif // SERVICE_H
