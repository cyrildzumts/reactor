#ifndef FUNCTION_WRAPPER_H
#define FUNCTION_WRAPPER_H
#include <memory>
#include <utility> // std::invoke()
#include <functional> //std::forward()
#include <type_traits>

template<typename Callable, typename... Args>
decltype (auto) call_callable(Callable &&op, Args&&... args){
    if constexpr(std::is_same_v<std::invoke_result_t<Callable, Args...>,
                 std::invoke(std::forward<Callable>(op),std::forward<Args...>(args...))>){
        return;
    }
    else {
        //decltype (auto) ret = std::invoke(std::forward<Callable>(op),std::forward<Args...>(args...));
        return std::invoke(std::forward<Callable>(op),std::forward<Args...>(args...));
    }
}


/**
 * @brief The FunctionWrapper class a function wrapper used to generalized the functor object.
 * this class is need since std::function is copyable , not movable. It is needed so that when
 * used with a std::package_task ( which is only movable) the move action will be possible.
 */
class FunctionWrapper{
private:
    struct impl_base{
        virtual void call() = 0;
        virtual ~impl_base(){}
    };
    std::unique_ptr<impl_base>impl;
    template<typename F>
    struct impl_type : impl_base{
        F f;
        impl_type(F&& f): f(std::move(f)){

        }
        void call(){
            f();
        }
    };


public:
    template<typename Func>
    FunctionWrapper(Func &&f): impl(new impl_type<Func>(std::move(f))){

    }

    void operator()(){
        impl->call();
    }

    FunctionWrapper() = default;
    FunctionWrapper(FunctionWrapper && other):impl(std::move(other.impl)){

    }
    FunctionWrapper &operator=(FunctionWrapper &&other){
        impl = std::move(other.impl);
        return *this;
    }

    FunctionWrapper(const FunctionWrapper&) = delete;
    FunctionWrapper(FunctionWrapper&) = delete;
    FunctionWrapper &operator=(const FunctionWrapper&) = delete;

};

#endif // FUNCTION_WRAPPER_H
