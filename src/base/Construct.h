#ifndef _CONSTRUCT_H_
#define _CONSTRUCT_H_
#include <new>

template <class T>
inline void destroy(T* pointer)
{
    pointer->~T();
}

template <class T>
inline void construct(T* p)
{
    new (p) T();
}

template <class T, class T1>
inline void construct(T* p, const T1& a1)
{
    new (p) T(a1);
}

template <class T, class T1, class T2>
inline void construct(T* p, const T1& a1, const T2& a2)
{
    new (p) T(a1, a2);
}

template <class T, class T1, class T2, class T3>
inline void construct(T* p, const T1& a1, const T2& a2, const T3& a3)
{
    new (p) T(a1, a2, a3);
}

template <class T, class T1, class T2, class T3, class T4>
inline void construct(T* p, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
    new (p) T(a1, a2, a3, a4);
}

#endif //_CONSTRUCT_H_