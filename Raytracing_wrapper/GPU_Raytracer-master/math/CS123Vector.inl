/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
/**<!-------------------------------------------------------------------->
   @brief
      Provides basic functionality for a constant-sized Vector.
   <!-------------------------------------------------------------------->**/

#ifndef __CS123_VECTOR_INL__
#define __CS123_VECTOR_INL__

#include <assert.h>
#include <iostream>
using namespace std;


// Extra operators where Vector is on right-hand side
// --------------------------------------------------

//! @returns the N-length vector resulting from multiplying a scalar by an N-length vector

//! @returns the N-length vector resulting from multiplying a scalar by an N-length vector
template<typename T>
vec4<T> operator* (const T &scale, const vec4<T> &rhs) { return rhs * scale; }





//! @returns (-1) * rhs, which is a negated version of the original right-hand side vector
template<typename T>
vec4<T> operator- (const vec4<T> &rhs) { return rhs * (-1); }




template <typename T>
inline std::ostream &operator<<(std::ostream &os, const vec4<T> &v) {
    os << "[ " << v.x << " " << v.y << " " << v.z << " " << v.w << " ]";
    return os;
}

template<typename T>
inline T vec4<T>::normalize() {
    T m = (T)1.0 / sqrt(x*x + y*y + z*z + w*w);
#pragma vector align
    for (unsigned i = 0; i < 4; ++i)
        data[i] *= m;
    return 1.0/m;
};


#endif // __CS123_VECTOR_INL__

