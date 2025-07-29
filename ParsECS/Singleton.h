#pragma once

template<typename T>
class Singleton
{
public:
    static T* GetInstance() {
        if (instance_)
            return instance_;
        return instance_ = new T();
    }

private:
    static T* instance_;
};

template<typename T>
T* Singleton<T>::instance_ = 0x0;