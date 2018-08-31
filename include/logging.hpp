#pragma once
#include <iostream>

#define LOG(msg) Log(__FILE__, __LINE__, Part<bool, bool>() << msg)

template<typename T> struct PartTrait { typedef T Type; };

// Workaround GCC 4.7.2 not recognizing noinline attribute
#ifndef NOINLINE_ATTRIBUTE
  #ifdef __ICC
    #define NOINLINE_ATTRIBUTE __attribute__(( noinline ))
  #else
    #define NOINLINE_ATTRIBUTE
  #endif // __ICC
#endif // NOINLINE_ATTRIBUTE

// Mark as noinline since we want to minimize the log-related instructions
// at the call sites
template<typename T>
void Log(const char* file, int line, const T& msg) NOINLINE_ATTRIBUTE {
    std::cout << file << ":" << line << ": " << msg << std::endl;
}

template<typename TValue, typename TPreviousPart>
struct Part : public PartTrait<Part<TValue, TPreviousPart>>
{
    Part()
        : value(nullptr), prev(nullptr) { }

    Part(const Part<TValue, TPreviousPart>&) = default;
    Part<TValue, TPreviousPart> operator=(
                           const Part<TValue, TPreviousPart>&) = delete;

    Part(const TValue& v, const TPreviousPart& p)
        : value(&v), prev(&p) { }

    std::ostream& output(std::ostream& os) const {
        if (prev)
            os << *prev;
        if (value)
            os << *value;
        return os;
    }

    const TValue* value;
    const TPreviousPart* prev;
};

// Specialization for stream manipulators (eg endl)

typedef std::ostream& (*PfnManipulator)(std::ostream&);

template<typename TPreviousPart>
struct Part<PfnManipulator, TPreviousPart>
    : public PartTrait<Part<PfnManipulator, TPreviousPart>> {
    Part()
        : pfn(nullptr), prev(nullptr) { }

    Part(const Part<PfnManipulator, TPreviousPart>& that) = default;
    Part<PfnManipulator, TPreviousPart> operator=(const Part<PfnManipulator,
                                                  TPreviousPart>&) = delete;

    Part(PfnManipulator pfn_, const TPreviousPart& p)
    : pfn(pfn_), prev(&p) { }

    std::ostream& output(std::ostream& os) const {
        if (prev)
            os << *prev;
        if (pfn)
            pfn(os);
        return os;
    }

    PfnManipulator pfn;
    const TPreviousPart* prev;
};

template<typename TPreviousPart, typename T>
typename std::enable_if<
    std::is_base_of<PartTrait<TPreviousPart>, TPreviousPart>::value, 
    Part<T, TPreviousPart> >::type
operator<<(const TPreviousPart& prev, const T& value) {
    return Part<T, TPreviousPart>(value, prev);
}

template<typename TPreviousPart>
typename std::enable_if<
    std::is_base_of<PartTrait<TPreviousPart>, TPreviousPart>::value,
    Part<PfnManipulator, TPreviousPart> >::type
operator<<(const TPreviousPart& prev, PfnManipulator value) {
    return Part<PfnManipulator, TPreviousPart>(value, prev);
}

template<typename TPart>
typename std::enable_if<
    std::is_base_of<PartTrait<TPart>, TPart>::value,
    std::ostream&>::type
operator<<(std::ostream& os, const TPart& part) {
    return part.output(os);
}