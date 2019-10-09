#ifndef _NEW_H_
#define _NEW_H_

#include "base/Allocator.h"
#include "base/Construct.h"

#define     ALLOCATOR       Allocator

template <class T>
class New
{
public:
    typedef     T           Value;
    typedef     T*          Point;
    typedef     T&          Ref;
    typedef     ALLOCATOR   Alloc;

public:
    static Point allocate() {
        Point obj = (Point)Alloc::allocate(sizeof(Value));
        construct(obj);
        return obj;
    }

    template <class T1>
    static Point allocate(const T1& a1) {
        Point obj = (Point)Alloc::allocate(sizeof(Value));
        construct(obj, a1);
        return obj;
    }

    template <class T1, class T2>
    static Point allocate(const T1& a1, const T2& a2) {
        Point obj = (Point)Alloc::allocate(sizeof(Value));
        construct(obj, a1, a2);
        return obj;
    }

    template <class T1, class T2, class T3>
    static Point allocate(const T1& a1, const T2& a2, const T3& a3) {
        Point obj = (Point)Alloc::allocate(sizeof(Value));
        construct(obj, a1, a2, a3);
        return obj;
    }

    template <class T1, class T2, class T3, class T4>
    static Point allocate(const T1& a1, const T2& a2, const T3& a3, const T4& a4) {
        Point obj = (Point)Alloc::allocate(sizeof(Value));
        construct(obj, a1, a2, a3, a4);
        return obj;
    }
};

class Delete
{
public:
    typedef     ALLOCATOR   Alloc;

    template <class T1>
    static void release(T1* point) {
        destroy(point);
        Alloc::deallocate(point, sizeof(T1));
    }

};

#endif //_NEW_H_