#ifndef FUNCTION_WRAPPER_H
#define FUNCTION_WRAPPER_H
#include <memory>

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
