/*  Cpp5 Framework - A small implementation in C++ (mostly C) of the Processing and p5.js frameworks.
    Copyright (c) 2020 Martin Fairbanks

    Licensing information can be found at the end of the file.
*/

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "Xinput9_1_0.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#pragma warning( disable : 4100; disable : 4201; disable: 4996 )

#ifdef NOCRT
#define _NO_CRT_STDIO_INLINE
extern "C" int _fltused = 0; // for floating point
#endif

#include <windows.h>
#include <stdint.h> // types
#include <stdio.h> // for vsprintf_s
#include <xinput.h>
#include <gl/gl.h>
#include <gl/glu.h> // gluOrtho2D
#include <math.h>
#include <malloc.h> 

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ext/include/stb_image.h"
#include "ext/include/stb_image_write.h"

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef size_t sizeT; // size_t is 64 bits on 64bit builds and 32-bit on 32bit builds

#define internal static
#define local static
#define global static

#define arrayCount(a) (sizeof(a) / sizeof(a[0]))
#define arrayColumns(a) (sizeof(a[0]) / sizeof(a[0][0]))
#define arrayRows(a) (sizeof(a) / sizeof(a[0]))

inline void
clearMemory(void *memory, sizeT size)
{
    u8 *byte = (u8 *)memory;
    while (size--) {
        *byte++ = 0;
    }
}

#define clearArray(a) clearMemory(a, sizeof(a))
#define clearStruct(a) clearMemory(&(a), sizeof(a))

#if DEVELOPER
#include <crtdbg.h> // memory leaks

void debugPrint(char *format, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);
#if DEBUGGER_MSVC
    OutputDebugStringA(buffer);
#else
    printf("%s\n", buffer);
#endif
}

#define debugPrintVariable(var) debugPrint(#var" = %d\n", var);
#define Assert(x) if (!(x)) { MessageBoxA(0, #x, "Assertion Failure", MB_OK); __debugbreak(); }
#else
#define Assert(x)
#endif

internal void
quitError(const char *errorMessage, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, errorMessage);
    vsprintf_s(buffer, sizeof(buffer), errorMessage, args);
    va_end(args);
    //	OutputDebugStringA(buffer);
    MessageBoxA(0, buffer, "Fatal Error", MB_ICONERROR);
    ExitProcess(1);
}

//
// Data Structures
// 

// dynamic array
struct Array {
    i32 length;
    i32 size;
    void **data;

    void reserve(i32 initSize)
    {
        length = 0;
        size = initSize;
        data = (void **)malloc(sizeof(void *) * size);
    }

    void resize(i32 newSize)
    {

#if defined(_DEBUG) || defined(DEVELOPER)
        printf("Realloc the Array: %d to %d\n", size, newSize);
#endif
        void **newArray = (void **)realloc(data, sizeof(void *) * newSize);
        if (newArray)
        {
            data = newArray;
            size = newSize;
        }
    }

    void push(void *value)
    {
        if (size == length)
            resize(size * 2);
        data[length++] = value;
    }

    void splice(i32 index)
    {
        if (index < 0 || index >= length)
            return;

        free(data[index]);
        data[index] = 0;

        for (int i = index; i < length - 1; i++) {
            data[i] = data[i + 1];
            data[i + 1] = 0;
        }

        length--;

        if (length > 0 && length == size / 4)
            resize(size / 2);
    }

    void set(int index, void *value)
    {
        if (index >= 0 && index < length)
            data[index] = value;
    }

    void *get(int index)
    {
        if (index >= 0 && index < length)
            return data[index];
        return 0;
    }

    void freeMem()
    {
        for (i32 i = 0; i < length; i++)
            free(data[i]);

        free(data);
    }

};

// Sean Barrets stretchy buffer
// https://github.com/nothings/stb/blob/master/stretchy_buffer.h

#define freeArray(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define pushArray(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define countArray(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      ((a) = stb__sbgrowf((a), (n), sizeof(*(a))))

static void *stb__raw_sbgrowf(void *arr, int increment, int itemsize)
{
    int dbl_cur = arr ? 2 * stb__sbm(arr) : 0;
    int min_needed = countArray(arr) + increment;
    int m = dbl_cur > min_needed ? dbl_cur : min_needed;
    int *p = (int *)realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int) * 2);
    if (p) {
        if (!arr)
            p[1] = 0;
        p[0] = m;
        return p + 2;
    } else {
        return (void *)(2 * sizeof(int));
    }
}

template<class T>
static T *stb__sbgrowf(T *arr, int increment, int itemsize) {
    return (T *)stb__raw_sbgrowf((void *)arr, increment, itemsize);
}


//
// Math
//
#include <random>

#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647693f
#define HALF_PI PI/2
#define minimum(a, b) ((a < b) ? (a) : (b))
#define maximum(a, b) ((a > b) ? (a) : (b))
#define SWAP(x, y)\
x = x ^ y; \
y = x ^ y; \
x = x ^ y;
#if 0

#define swap(x, y)\
f32 temp = x; \
x = y; \
y = temp;
#endif

#define	swapArray(a, x, y) \
a[x] = a[x] ^ a[y]; \
a[y] = a[x] ^ a[y]; \
a[x] = a[x] ^ a[y];

// truncate 64bits to 32bits and assert truncation
inline u32 safeTruncateUInt64(u64 value)
{
    u32 result = (u32)value;
    return result;			
}

// round down to the nearest integer
// the floor function truncates towards negative instead of zero
inline i32 floorFloatToInt(f32 value)
{
    i32 result = (i32)floorf(value);
    return result;
}

// round up to the nearest integer
inline i32 ceilFloatToInt(f32 value)
{
    i32 result = (i32)ceilf(value);
    return result;
}

// round a float value to nearest integer
inline i32 roundFloatToInt(f32 value)
{
    //i32 result = (i32)(value + 0.5f);
    i32 result = (i32)roundf(value);
    return result;
}

// round a float value to nearest unsigned integer
inline u32 roundFloatToUInt(f32 value)
{
    //u32 result = (u32)(value + 0.5f);
    u32 result = (u32)roundf(value);
    return result;
}

// returns the square root of a number, the result is always positive
// a square root of a number a is a number y such that y^2 = a
inline f32 squareRoot(f32 value)
{
    f32 result = sqrtf(value);
    return result;
}

// returns value squared (value^2), the result is always positive
inline f32 square(f32 a)
{
    f32 result = a*a;
    return result;
}

// an absolute value is defined as the distance from zero, absoluteValue(-5) will return 5
inline f32 absoluteValue(f32 value)
{
    f32 result = (f32)fabs(value);
    return result;
}

// returns 1 if the value is positive and -1 if the value is negative
inline i32 signOf(i32 value)
{
    i32 result = (value >= 0) ? 1 : -1;
    return result;
}

// constrains a value to be within a range
inline i32 constrain(i32 value, i32 min, i32 max)
{
    return (value < min) ? min : ((value > max) ? max : value);
}

inline f32 constrainf(f32 value, f32 min, f32 max)
{
    return (value < min) ? min : ((value > max) ? max : value);
}

// re-maps a value from one range to another
inline f32 map(f32 value, f32 inMin, f32 inMax, f32 outMin, f32 outMax)
{
    if (fabs(inMin - inMax) < FLT_EPSILON)
    {
        return outMin;
    }
    else
    {
        f32 outVal = ((value - inMin) / (inMax - inMin) * (outMax - outMin) + outMin);
        
        if (outMax < outMin)
        {
            if (outVal < outMax) 
                outVal = outMax;
            else if (outVal > outMin) 
                outVal = outMin;
        }
        else
        {
            if (outVal > outMax) 
                outVal = outMax;
            else if (outVal < outMin) 
                outVal = outMin;
        }
        
        return outVal;
    }
}

// returns a interpolated value between two numbers, the amount parameter is the amount to interpolate between the two values. 
inline f32 lerp(f32 min, f32 max, f32 amount)
{
    /* 
       Linear blend
       C = A + (B - A) * t
       t is the coefficient, a value between 0 and 1, it could be thought of as
       some procentage of, it's how much alpha we want.
       if the coefficient is 0, Color has the value of A, and as t 
       gets closer to 1, Color becomes closer to B.
       B-A is the distance.
       A(startvalue, background) and B(endvalue, sprite) are the two colors.
    
       Color = A + t(B - A)
       Color = A + (B - A) * t
       A + tB - tA
       A - tA + tB
       (1 - t)A + tB

        In C language: (1.0f - t) * a + t * b;

    */
    f32 result = amount * (max - min) + min; // (1.0f - t) * a + t * b;
    return result;
}

//
// Trigonometry
//

// sinus(), cosinus() takes radians as input and returns a number between -1 and 1
inline f32 sinus(f32 angle)
{
    f32 result = sinf(angle);
    return result;
}

inline f32 cosinus(f32 angle)
{
    f32 result = cosf(angle);
    return result;
}

// used for calculating an angle
inline f32 arcTangent2(f32 y, f32 x)
{
    f32 result = atan2f(y, x);
    return result;
}

// returns the arc cosine (inverse cosine, cos-1) of a number in radians
inline f32 arcCosine(f32 angle)
{
    f32 result = acosf(angle);
    return result;
}

//convert radians to degrees
inline f32 degrees(f32 radians)
{
    f32 degrees = (radians / TWO_PI) * 360;
    return degrees;
}

//convert degrees to radians
inline f32 radians(f32 degrees)
{
    f32 radians = (degrees / 360) * TWO_PI;
    return radians;
}


//
// Random numbers
//

// pseudo-random uniform distribution of numbers
// set the random seed to constant value to return the same pseudo random numbers every time
inline void randomSeed(u32 value)
{
    srand(value);
}

// returns random integer between min and max
inline i32 random(i32 min, i32 max)
{
    /* ex:  Random(-10,20) -> will give -10 to, and including, 20. */
    max += 1;
    min -= 1;
    return i32(rand() / (f32)(RAND_MAX + 1) * (max - min) + min);
}

// returns random integer between 0 and max
inline i32 random(i32 max)
{
    // i32 min = -1;
    max += 1;
    return i32(rand() / (f32)(RAND_MAX + 1) * (max));
}

// returns a random float between 0 and 1
inline f32 randomf()
{
    return ((f32)rand() / (RAND_MAX));
    //random float between -1 and 1
    //return (((f32)rand() / (RAND_MAX)) * 2 - 1.0f);
}

// returns random float between min and max
inline f32 randomf(f32 min, f32 max)
{
    return min + randomf() * (max - min);
}

//the likelihood that a random value will be picked is equal to the the first random number (r1)
//returns a random value between 0 and 1
inline f32 montecarlo()
{
    //loop until we find a qualifying random number
    while (true)
    {
        f32 r1 = randomf();
        f32 probability = r1;
        //f32 probability = pow(1.0 - r1, 8);

        f32 r2 = randomf();

        if (r2 < probability)
            return r1;
    }
}

//
// Noise
//

#include "ext/include/slang_library_noise.cpp"
//1D Perlin noise, returns noise value at specified coordinate, the value is always between 0 and 1.
inline f32 noise(f32 x)
{
    return _slang_library_noise1(x) * 0.5f + 0.5f;
}

//2D Perlin noise
inline f32 noise(f32 x, f32 y)
{
    return _slang_library_noise2(x, y) * 0.5f + 0.5f;
}

//3D Perlin noise
inline f32 noise(f32 x, f32 y, f32 z)
{
    return _slang_library_noise3(x, y, z) * 0.5f + 0.5f;
}


//
// Vectors 
//

union v2 {
    
    struct {
        f32 x, y;
    };
    struct {
        f32 w, h;
    };
    
    v2() { };
    v2(f32 X, f32 Y) : x(X), y(Y) {}
    
    // calculate length (magnitude) of the vector, square root of x^2 + y^2 -> same as sqrt(dotProduct(*this))
    // The Pythagorean Theorem
    f32 length() 
    {
        f32 result = sqrt(x * x + y * y);
        return result;
    }
    
    // returns the angle of rotation, the heading (direction), of the vector.
    f32 heading() 
    { 
        f32 result = arcTangent2(y, x);
        return result;
    }
    
    // calculate the dot(inner) product, returns a scalar
    // dot product of a * b = lenght of a * length of b * cos(angle between them)
    // The dot product consists of multiplying each element of the A vector with its counterpart from vector B and taking the sum of each product.
    f32 dotProduct(v2 a)
    {
        f32 result = x * a.x + y * a.y;
        return result;
    }
    
    // calculates and returns the angle (in radians) between two vectors
    // dot product of a * b = lenght of a * length of b * cos(angle between them)
    f32 angleBetween(v2 a)
    {
        f32 d = a.dotProduct(*this);
        f32 result = acos(d / (a.length() * (*this).length()));
        return result;
    }
    
    // set length of vector
    void setLength(f32 length)
    {
        f32 a = heading();
        x = cos(a) * length;
        y = sin(a) * length;
    }
    
    // set the angle of the vector
    void setAngle(f32 angle)
    {
        f32 l = length();
        x = cos(angle) * l;
        y = sin(angle) * l;
    }
    
    // limit the magnitude of the vector
    void limit(f32 max)
    {
        if (length() > max)
            setLength(max);
    }
    
    // calculate a unit vector. normalizing a vector makes its length equal to 1.
    // to normalize a vector - multiply it by the inverse of its length
    // or divide each component by its length
    void normalize()
    {
        f32 l = length();
        // avoid divide by 0
        if (l > 0) { 
            x *= 1 / l;
            y *= 1 / l;
            //x /= l;
            //y /= l;
        }
    }
};

// overloaded operators
// vector addition: newVector = v1 + v2;
inline v2 operator+(v2 a, v2 b)
{
    v2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

// v1 += v2;
inline v2 &operator+=(v2 &a, v2 b)
{
    a = a + b;
    return a;
}

// vector subtraction
inline v2 operator-(v2 a, v2 b)
{
    v2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline v2 operator-=(v2 &a, v2 b)
{
    a = a - b;
    return a;
}

// vector multiplication with a scalar number (vector scaling)
inline v2 operator*(f32 scalar, v2 a)
{
    v2 result;
    result.x = scalar * a.x;
    result.y = scalar * a.y;
    return result;
}

inline v2 operator*(v2 a, f32 scalar)
{
    v2 result = scalar * a;
    return result;
}

inline v2 &operator*=(v2 &a, f32 scalar)
{
    a = scalar * a;
    return a;
}

// divide by a scalar number
inline v2 operator/(f32 scalar, v2 a)
{
    v2 result;
    result.x = scalar / a.x;
    result.y = scalar / a.y;
    return result;
}

inline v2 operator/(v2 a, f32 scalar)
{
    v2 result;
    result.x = a.x / scalar;
    result.y = a.y / scalar;
    return result;
}

inline v2 &operator/=(v2 &a, f32 scalar)
{
    a.x /= scalar;
    a.y /= scalar;
    return a;
}

// vector negation, vector equals it's negative
inline v2 operator-(v2 a)
{
    v2 result;
    
    result.x = -a.x;
    result.y = -a.y;
    
    return result;
}

// the hadamard product - element-wise product of two vectors which return a new vector
inline v2 v2Hadamard(v2 a, v2 b)
{
    v2 result = { a.x * b.x, a.y * b.y };
    return result;
}

// calculate the dot(inner) product which gives us the angle between two vectors, returns a scalar
// The dot product tells you what amount of one vector goes in the direction of another.
// dot product of a * b = lenght of a * length of b * cos(angle between them)
// The dot product consists of multiplying each element of the A vector with its counterpart from vector B and taking the sum of each product.
inline f32 v2DotProduct(v2 a, v2 b)
{
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

// using the Pythagorean Theorem to calculate the distance between 2 points
// same as calculating a vectors length
inline f32 dist(v2 a, v2 b)
{
    return squareRoot((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

//random 2d direction
inline v2 random2d()
{
    v2 vec = v2(1.0, 1.0);
    //vec.normalize();
    vec.setAngle(randomf() * TWO_PI);
    return vec;
}

inline void swapV2(v2 *a, v2 *b)
{
    v2 temp;
    temp.x = a->x;
    a->x = b->x;
    b->x = temp.x;
    temp.y = a->y;
    a->y = b->y;
    b->y = temp.y;
}

typedef struct {
    i32 x;
    i32 y;
} v2i;

// overloaded operators
// vector addition: newVector = v1 + v2;
inline v2i operator+(v2i a, v2i b)
{
    v2i result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

// v1 += v2;
inline v2i &operator+=(v2i &a, v2i b)
{
    a = a + b;
    return a;
}

// vector subtraction
inline v2i operator-(v2i a, v2i b)
{
    v2i result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline v2i operator-=(v2i &a, v2i b)
{
    a = a - b;
    return a;
}

// vector multiplication with a scalar number (vector scaling)
inline v2i operator*(i32 scalar, v2i a)
{
    v2i result;
    result.x = scalar * a.x;
    result.y = scalar * a.y;
    return result;
}

inline v2i operator*(v2i a, i32 scalar)
{
    v2i result = scalar * a;
    return result;
}

inline v2i &operator*=(v2i &a, i32 scalar)
{
    a = scalar * a;
    return a;
}

//divide by a scalar number
inline v2i operator/(i32 scalar, v2i a)
{
    v2i result;
    result.x = scalar / a.x;
    result.y = scalar / a.y;
    return result;
}

inline v2i operator/(v2i a, i32 scalar)
{
    v2i result;
    result.x = a.x / scalar;
    result.y = a.y / scalar;
    return result;
}

inline v2i &operator/=(v2i &a, i32 scalar)
{
    a.x /= scalar;
    a.y /= scalar;
    return a;
}

// Vector3

union v3 {
    
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
    struct {
        f32 u, v, w;
    };
    
    // swizzle
    struct
    {
        v2 xy;
        f32 _ignored0;
    };
    
    struct
    {
        f32 _ignored1;
        v2 yz;
    };
    
    f32 e[3];
    
    v3() { };
    v3(f32 X, f32 Y, f32 Z) : x(X), y(Y), z(Z) {}
    // overloaded operator for vector addition: v3 = v1 + v2;
    v3 operator+(const v3& v2) const
    {
        return v3{ x + v2.x, y + v2.y, z + v2.z };
    }
    
    // v1 += v2;
    friend v3& operator+=(v3& v1, const v3& v2)
    {
        v1.x += v2.x;
        v1.y += v2.y;
        v1.z += v2.z;
        return v1;
    }
    
    // subtracting two vectors
    v3 operator-(const v3& v2) const
    {
        return v3{ x - v2.x, y - v2.y, z - v2.z };
    }
    
    friend v3& operator-=(v3& v1, const v3& v2)
    {
        v1.x -= v2.x;
        v1.y -= v2.y;
        v1.z -= v2.z;
        return v1;
    }
    
    //multiply by a scalar number
    v3 operator*(f32 scalar)
    {
        return v3{ x * scalar, y * scalar, z * scalar };
    }
    
    v3& operator*=(f32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    
    //divide by a scalar number
    v3 operator/(f32 scalar)
    {
        return v3{ x / scalar, y / scalar, z / scalar };
    }
    
    v3& operator/=(f32 scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        
        return *this;
    }
    
    
    //calculate length (magnitude) of the vector, square root of x^2 + y^2 + z^2
    f32 length() { return sqrt(x * x + y * y + z * z); }
    
    //normalizing a vector makes its length equal to 1.
    //to normalize a vector - multiply it by the inverse of its length
    void normalize()
    {
        f32 l = length();
        if (l > 0) //avoid divide by 0
        {
            (*this) *= 1 / l;
        }
    }
};

// vector negation, vector equals it's negative
inline v3 operator-(v3 a)
{
    v3 result;
    
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    
    return result;
}

typedef union v4 {
    struct {
        f32 r, g, b, a;
    };
    struct {
        f32 h, s, b, a;
    };
    struct {
        f32 h, s, l, a;
    };
    struct {
        f32 x, y, z, w;
    };
    f32 e[4];
} v4;

#define v2i(x, y) v2iInit(x, y)
v2i v2iInit(i32 x, i32 y)
{
    v2i result = { x, y };
    return result;
}

#define v2(x, y) v2Init(x, y)
v2 v2Init(f32 x, f32 y)
{
    v2 result = { x, y };
    return result;
}

#define v3(x, y, z) v3Init(x, y, z)
v3 v3Init(f32 x, f32 y, f32 z)
{
    v3 result = { x, y, z };
    return result;
}

#define v4(x, y, z, w) v4Init(x, y, z, w)
v4 v4Init(f32 x, f32 y, f32 z, f32 w)
{
    v4 result = { x, y, z, w };
    return result;
}

// vector addition
inline v2
v2Add(v2 a, v2 b)
{
    v2 result = v2(a.x + b.x, a.y + b.y);
    return result;
}

// vector subtraction
inline v2
v2Sub(v2 a, v2 b)
{
    v2 result = v2(a.x - b.x, a.y - b.y);
    return result;
}

// multiplication with a scalar
inline v2
v2Mul(v2 a, f32 s)
{
    v2 result = v2(a.x * s, a.y * s);
    return result;
}

// calculate length (magnitude) of the vector, square root of x^2 + y^2 + z^2
f32 v3Length(v3 inVec)
{
    f32 result = sqrt(inVec.x * inVec.x + inVec.y * inVec.y + inVec.z * inVec.z);
    return result;
}

// normalizing a vector makes its length equal to 1.
// to normalize a vector - multiply it by the inverse of its length
v3 v3Normalize(v3 inVec)
{
    f32 l = v3Length(inVec);
    //avoid divide by 0
    if (l > 0) {
        inVec.x *= 1 / l;
        inVec.y *= 1 / l;
        inVec.z *= 1 / l;
    }
    return inVec;
}

inline f32 v3DotProduct(v3 a, v3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

//calculate the cross product of two vectors
inline v3 v3CrossProduct(v3 v1, v3 v2)
{
    v3 vResult;
    vResult.x = v1.y * v2.z - v2.y * v1.z;
    vResult.y = -v1.x * v2.z + v2.x * v1.z;
    vResult.z = v1.x * v2.y - v2.x * v1.y;
    return vResult;
}

// calculate the unit normal from 3 point on a plane in CCW-order
v3 v3GetNormalVector(v3 vP1, v3 vP2, v3 vP3)
{
    v3 vV1, vV2;
    v3 vNormal;
    vV1 = vP2 - vP1;
    vV2 = vP3 - vP1;

    vNormal = v3CrossProduct(vV1, vV2);
    vNormal.normalize();
    return vNormal;
}

// gets the three coefficients of a plane equation given three points on the plane
void v3GetPlaneEquation(v3 vPoint1, v3 vPoint2, v3 vPoint3, f32 vPlane[4])
{
    // Get normal vector from three points. The normal vector is the first three coefficients
    // to the plane equation...
    v3 vP = v3GetNormalVector(vPoint1, vPoint2, vPoint3);
    vPlane[0] = vP.x;
    vPlane[1] = vP.y;
    vPlane[2] = vP.z;
    // Final coefficient found by back substitution
    vPlane[3] = -(vPlane[0] * vPoint3.x + vPlane[1] * vPoint3.y + vPlane[2] * vPoint3.z);
}

typedef union
{
    //struct {
    //v2 min, max;
    //};
    
    struct {
        f32 x, y, w, h;
    };
} Rect;


//
// Matrices
//

struct m4 {
	f32 e[4][4] = { 0 };
};

m4 m4LoadIdentity()
{
    m4 m =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.f,  1.0f, 0.0f, 0.0f,
        0.f,  0.f,  1.0f, 0.0f,
        0.f,  0.f,  0.f,  1.0f
    };
    return m;
#if 0
    m4 identity = {
        {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1},
        }
    };
#endif
}

m4 vecToMatrix(v4 v)
{
    m4 m = {0};;
    m.e[0][0] = v.x;
    m.e[1][0] = v.y;
    m.e[2][0] = v.z;
    m.e[3][0] = v.w;
    return m;
}


m4 vecToMatrix(v3 v)
{
    m4 m = {0};;
    m.e[0][0] = v.x;
    m.e[1][0] = v.y;
    m.e[2][0] = v.z;
    m.e[3][0] = 0;
    return m;
}


v4 matrixToVec(m4 m)
{
    v4 result;
    result.x = m.e[0][0];
    result.y = m.e[1][0];
    result.z = m.e[2][0];
    result.w = m.e[3][0];
    return result;
}

// b multiplys with a, my stuff
m4 m4Multiply(m4 m1, m4 m2)
{
	m4 result;
    
	result.e[0][0] = m2.e[0][0] * m1.e[0][0] + m2.e[1][0] * m1.e[0][1] + m2.e[2][0] * m1.e[0][2] + m2.e[3][0] * m1.e[0][3];
	result.e[1][0] = m2.e[0][0] * m1.e[1][0] + m2.e[1][0] * m1.e[1][1] + m2.e[2][0] * m1.e[1][2] + m2.e[3][0] * m1.e[1][3];
	result.e[2][0] = m2.e[0][0] * m1.e[2][0] + m2.e[1][0] * m1.e[2][1] + m2.e[2][0] * m1.e[2][2] + m2.e[3][0] * m1.e[2][3];
	result.e[3][0] = m2.e[0][0] * m1.e[3][0] + m2.e[1][0] * m1.e[3][1] + m2.e[2][0] * m1.e[3][2] + m2.e[3][0] * m1.e[3][3];
    
    result.e[0][1] = m2.e[0][1] * m1.e[0][0] + m2.e[1][1] * m1.e[0][1] + m2.e[2][1] * m1.e[0][2] + m2.e[3][1] * m1.e[0][3];
	result.e[1][1] = m2.e[0][1] * m1.e[1][0] + m2.e[1][1] * m1.e[1][1] + m2.e[2][1] * m1.e[1][2] + m2.e[3][1] * m1.e[1][3];
	result.e[2][1] = m2.e[0][1] * m1.e[2][0] + m2.e[1][1] * m1.e[2][1] + m2.e[2][1] * m1.e[2][2] + m2.e[3][1] * m1.e[2][3];
	result.e[3][1] = m2.e[0][1] * m1.e[3][0] + m2.e[1][1] * m1.e[3][1] + m2.e[2][1] * m1.e[3][2] + m2.e[3][1] * m1.e[3][3];
    
    result.e[0][2] = m2.e[0][2] * m1.e[0][0] + m2.e[1][2] * m1.e[0][1] + m2.e[2][2] * m1.e[0][2] + m2.e[3][2] * m1.e[0][3];
	result.e[1][2] = m2.e[0][2] * m1.e[1][0] + m2.e[1][2] * m1.e[1][1] + m2.e[2][2] * m1.e[1][2] + m2.e[3][2] * m1.e[1][3];
	result.e[2][2] = m2.e[0][2] * m1.e[2][0] + m2.e[1][2] * m1.e[2][1] + m2.e[2][2] * m1.e[2][2] + m2.e[3][2] * m1.e[2][3];
	result.e[3][2] = m2.e[0][2] * m1.e[3][0] + m2.e[1][2] * m1.e[3][1] + m2.e[2][2] * m1.e[3][2] + m2.e[3][2] * m1.e[3][3];
    
    result.e[0][3] = m2.e[0][3] * m1.e[0][0] + m2.e[1][3] * m1.e[0][1] + m2.e[2][3] * m1.e[0][2] + m2.e[3][3] * m1.e[0][3];
	result.e[1][3] = m2.e[0][3] * m1.e[1][0] + m2.e[1][3] * m1.e[1][1] + m2.e[2][3] * m1.e[1][2] + m2.e[3][3] * m1.e[1][3];
	result.e[2][3] = m2.e[0][3] * m1.e[2][0] + m2.e[1][3] * m1.e[2][1] + m2.e[2][3] * m1.e[2][2] + m2.e[3][3] * m1.e[2][3];
	result.e[3][3] = m2.e[0][3] * m1.e[3][0] + m2.e[1][3] * m1.e[3][1] + m2.e[2][3] * m1.e[3][2] + m2.e[3][3] * m1.e[3][3];
    
    
	return result;
}

v4 m4MultiplyV4(m4 m, v4 v)
{
    v4 result = {0};
    
    result.x = v.e[0]*m.e[0][0] + v.e[1]*m.e[0][1] + v.e[2]*m.e[0][2] + v.e[3]*m.e[0][3];
    result.y = v.e[0]*m.e[1][0] + v.e[1]*m.e[1][1] + v.e[2]*m.e[1][2] + v.e[3]*m.e[1][3];
    result.z = v.e[0]*m.e[2][0] + v.e[1]*m.e[2][1] + v.e[2]*m.e[2][2] + v.e[3]*m.e[2][3];
    result.w = v.e[0]*m.e[3][0] + v.e[1]*m.e[3][1] + v.e[2]*m.e[3][2] + v.e[3]*m.e[3][3];
    
    return result;
}

v3 m4MultiplyV3(m4 m, v3 v)
{
    v3 result;
    
    result.x = v.e[0]*m.e[0][0] + v.e[1]*m.e[0][1] + v.e[2]*m.e[0][2];
    result.y = v.e[0]*m.e[1][0] + v.e[1]*m.e[1][1] + v.e[2]*m.e[1][2];
    result.z = v.e[0]*m.e[2][0] + v.e[1]*m.e[2][1] + v.e[2]*m.e[2][2];
    
    return result;
}

// OpenGL friendly contigous array matrix
typedef f32 Matrix[16];      //column major 4x4 matrix
Matrix transformationMatrix;
Matrix rotationMatrix, translationMatrix, scalingMatrix;


// rotates a vector using a 4x4 matrix, translation column is ignored
void rotateVector(v3 vSrc, Matrix mMatrix, v3 *vOut)
{
	vOut->x = mMatrix[0] * vSrc.x + mMatrix[4] * vSrc.y + mMatrix[8] * vSrc.z;
	vOut->y = mMatrix[1] * vSrc.x + mMatrix[5] * vSrc.y + mMatrix[9] * vSrc.z;
	vOut->z = mMatrix[2] * vSrc.x + mMatrix[6] * vSrc.y + mMatrix[10] * vSrc.z;
}

// load a matrix with the identity matrix
void loadIdentityMatrix(Matrix m)
{
	static Matrix identity = {
		1.0f, 0.0f, 0.0f, 0.0f, // x column
		0.0f, 1.0f, 0.0f, 0.0f, // y column
		0.0f, 0.0f, 1.0f, 0.0f, // z column
		0.0f, 0.0f, 0.0f, 1.0f  // translation
	};
    
	memcpy(m, identity, sizeof(Matrix));
	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(identity);
}

// reset the transformation matrix
void loadIdentityMatrix()
{
	static Matrix identity = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
    
	memcpy(transformationMatrix, identity, sizeof(Matrix));
}

// multiply two 4x4 matricies
void multiplyMatrix(const Matrix m1, const Matrix m2, Matrix mProduct)
{
	mProduct[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
	mProduct[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
	mProduct[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
	mProduct[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
    
	mProduct[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
	mProduct[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
	mProduct[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
	mProduct[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
    
	mProduct[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
	mProduct[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
	mProduct[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
	mProduct[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
    
	mProduct[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];
	mProduct[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];
	mProduct[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];
	mProduct[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];
}

// create a translation matrix
void translateMatrix(f32 x, f32 y, f32 z)
{
	loadIdentityMatrix(translationMatrix);
	translationMatrix[12] = x;
	translationMatrix[13] = y;
	translationMatrix[14] = z;
    //	glMultMatrixf(translationMatrix);
	multiplyMatrix(translationMatrix, transformationMatrix, transformationMatrix);
    
}

void translateMatrix(Matrix m, v3 pos)
{
	loadIdentityMatrix(translationMatrix);
	translationMatrix[12] = pos.x;
	translationMatrix[13] = pos.y;
	translationMatrix[14] = pos.z;
    //	glMultMatrixf(translationMatrix);
	multiplyMatrix(translationMatrix, transformationMatrix, m);
    //return m;
    m[0] = 99;
}


// create a scaling matrix
void scale(f32 x, f32 y, f32 z)
{
	loadIdentityMatrix(scalingMatrix);
	scalingMatrix[0] = x;
	scalingMatrix[5] = y;
	scalingMatrix[10] = z;
	//glMultMatrixf(scalingMatrix);
	multiplyMatrix(transformationMatrix, scalingMatrix, transformationMatrix);
}

//creates a 4x4 rotation matrix, takes radians
void rotate(f32 angle, v3 pos)
{
	f32 sinSave, cosSave, oneMinusCos;
	f32 xx, yy, zz, xy, yz, zx, xs, ys, zs;
    
	if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f) {
		loadIdentityMatrix(rotationMatrix);
		return;
	}
    
	// normalize rotation matrix
	pos.normalize();
    
	sinSave = sin(angle);
	cosSave = cos(angle);
	oneMinusCos = 1.0f - cosSave;
    
	xx = pos.x * pos.x;
	yy = pos.y * pos.y;
	zz = pos.z * pos.z;
	xy = pos.x * pos.y;
	yz = pos.y * pos.z;
	zx = pos.z * pos.x;
	xs = pos.x * sinSave;
	ys = pos.y * sinSave;
	zs = pos.z * sinSave;
    
	rotationMatrix[0] = (oneMinusCos * xx) + cosSave;
	rotationMatrix[4] = (oneMinusCos * xy) - zs;
	rotationMatrix[8] = (oneMinusCos * zx) + ys;
	rotationMatrix[12] = 0.0f;
    
	rotationMatrix[1] = (oneMinusCos * xy) + zs;
	rotationMatrix[5] = (oneMinusCos * yy) + cosSave;
	rotationMatrix[9] = (oneMinusCos * yz) - xs;
	rotationMatrix[13] = 0.0f;
    
	rotationMatrix[2] = (oneMinusCos * zx) - ys;
	rotationMatrix[6] = (oneMinusCos * yz) + xs;
	rotationMatrix[10] = (oneMinusCos * zz) + cosSave;
	rotationMatrix[14] = 0.0f;
    
	rotationMatrix[3] = 0.0f;
	rotationMatrix[7] = 0.0f;
	rotationMatrix[11] = 0.0f;
	rotationMatrix[15] = 1.0f;
    
	multiplyMatrix(transformationMatrix, rotationMatrix, transformationMatrix);
}

// create a translation matrix
void createTranslationMatrix(f32 x, f32 y, f32 z, Matrix mTranslate)
{
    loadIdentityMatrix(mTranslate);
    mTranslate[12] = x;
    mTranslate[13] = y;
    mTranslate[14] = z;
}

// create a scaling matrix
void createScalingMatrix(f32 x, f32 y, f32 z, Matrix mScale)
{
    loadIdentityMatrix(mScale);
    mScale[0] = x;
    mScale[5] = y;
    mScale[10] = z;
}

// creates a 4x4 rotation matrix, takes radians
void createRotationMatrix(f32 angle, v3 pos, Matrix mMatrix)
{
    f32 sinSave, cosSave, oneMinusCos;
    f32 xx, yy, zz, xy, yz, zx, xs, ys, zs;
    
    if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
    {
        loadIdentityMatrix(mMatrix);
        return;
    }
    
    // normalize rotation matrix
    pos.normalize();
    
    sinSave = (f32)sin(angle);
    cosSave = (f32)cos(angle);
    oneMinusCos = 1.0f - cosSave;
    
    xx = pos.x * pos.x;
    yy = pos.y * pos.y;
    zz = pos.z * pos.z;
    xy = pos.x * pos.y;
    yz = pos.y * pos.z;
    zx = pos.z * pos.x;
    xs = pos.x * sinSave;
    ys = pos.y * sinSave;
    zs = pos.z * sinSave;
    
    mMatrix[0] = (oneMinusCos * xx) + cosSave;
    mMatrix[4] = (oneMinusCos * xy) - zs;
    mMatrix[8] = (oneMinusCos * zx) + ys;
    mMatrix[12] = 0.0f;
    
    mMatrix[1] = (oneMinusCos * xy) + zs;
    mMatrix[5] = (oneMinusCos * yy) + cosSave;
    mMatrix[9] = (oneMinusCos * yz) - xs;
    mMatrix[13] = 0.0f;
    
    mMatrix[2] = (oneMinusCos * zx) - ys;
    mMatrix[6] = (oneMinusCos * yz) + xs;
    mMatrix[10] = (oneMinusCos * zz) + cosSave;
    mMatrix[14] = 0.0f;
    
    mMatrix[3] = 0.0f;
    mMatrix[7] = 0.0f;
    mMatrix[11] = 0.0f;
    mMatrix[15] = 1.0f;
}

// creates a shadow matrix out of the plane equation coefficients and the position of the light
void createShadowMatrix(v3 vPoints[3], f32 vLightPos[4], Matrix destMat)
{
    f32 vPlaneEquation[4];
    f32 dot;
    //Matrix destMat;
    
    v3GetPlaneEquation(vPoints[0], vPoints[1], vPoints[2], vPlaneEquation);
    
    // dot product of plane and light position
    dot = vPlaneEquation[0] * vLightPos[0] +
        vPlaneEquation[1] * vLightPos[1] +
        vPlaneEquation[2] * vLightPos[2] +
        vPlaneEquation[3] * vLightPos[3];
    
    // calculate shadow projection
    // 1st column
    destMat[0] = dot - vLightPos[0] * vPlaneEquation[0];
    destMat[4] = 0.0f - vLightPos[0] * vPlaneEquation[1];
    destMat[8] = 0.0f - vLightPos[0] * vPlaneEquation[2];
    destMat[12] = 0.0f - vLightPos[0] * vPlaneEquation[3];
    
    // 2nd column
    destMat[1] = 0.0f - vLightPos[1] * vPlaneEquation[0];
    destMat[5] = dot - vLightPos[1] * vPlaneEquation[1];
    destMat[9] = 0.0f - vLightPos[1] * vPlaneEquation[2];
    destMat[13] = 0.0f - vLightPos[1] * vPlaneEquation[3];
    
    // 3rd Column
    destMat[2] = 0.0f - vLightPos[2] * vPlaneEquation[0];
    destMat[6] = 0.0f - vLightPos[2] * vPlaneEquation[1];
    destMat[10] = dot - vLightPos[2] * vPlaneEquation[2];
    destMat[14] = 0.0f - vLightPos[2] * vPlaneEquation[3];
    
    // 4th Column
    destMat[3] = 0.0f - vLightPos[3] * vPlaneEquation[0];
    destMat[7] = 0.0f - vLightPos[3] * vPlaneEquation[1];
    destMat[11] = 0.0f - vLightPos[3] * vPlaneEquation[2];
    destMat[15] = dot - vLightPos[3] * vPlaneEquation[3];
}

//
// Colors
//

union Color
{
    struct {
        i32 r, g, b, a;
    };
    struct {
        i32 h, s, b, a;
    };
    struct {
        i32 h, s, l, a;
    };
    struct {
        f32 fR, fG, fB, fA;
    };
};

typedef struct {
    f32 r, g, b, a;
} Colorf;

enum ColorModes {
    RGB,
    HSB,
    HSL
};

Color black = { 0,0,0, 255 };
Color white = { 255,255,255, 255 };
Color c64red = { 104, 55, 43, 255 };
Color c64cyan = { 112, 164, 178, 255 };
Color c64purple = { 111, 61, 134, 255 };
Color c64green = { 88, 141, 67, 255 };;
Color c64blue = { 53, 40, 121, 255 };
Color c64yellow = { 184, 199, 111, 255 };
Color c64orange = { 111, 79, 37, 255 };;
Color c64brown = { 67, 57, 0, 255 };
Color c64lightred = { 154, 103, 89, 255 };
Color c64darkgrey = { 68, 68, 68, 255 };
Color c64grey = { 108, 108, 108, 255 };
Color c64lightgreen = { 154, 210, 132, 255 };
Color c64lightblue = { 108, 94, 181, 255 };
Color c64lightgrey = { 149, 149, 149, 255 };
Color red = { 255,0,0, 255 };
Color green = { 0,255,0, 255 };;
Color blue = { 0,0,255, 255 };
Color yellow = { 255,255,0, 255 };
Color cyan = { 0,255,255, 255 };
Color magenta = { 255,0,255, 255 };
Color purple = { 128, 0, 128, 255 };
Color gray = { 128, 128, 128, 255 };
Color grey = { 192, 192, 192, 255 };
Color maroon = { 128, 0, 0, 255 };
Color darkgreen = { 0, 128, 0, 255 };
Color navy = { 0, 0, 128, 255 };
Color teal = { 0, 128, 128, 255 };
Color olive = { 128, 128, 0, 255 };
Color orange = { 255,127,50, 255 };
Color cornflowerblue = { 101, 156, 239, 255 };
Color azure = { 0, 127, 255, 255 };
Color turquoise = { 48, 213, 200, 255 };
Color gold = { 255, 215, 0, 255 };
Color silver = { 192, 192, 192, 255 };
Color pink = { 255, 192, 203, 255 };
Color brown = { 0xa5,0x2a,0x2a, 0xff };

//
// Input
//

typedef struct {
    b32 isDown;
    b32 changed;
} ButtonState;

enum {
    KEY_0 = 0x30,
    KEY_1 = 0x31,
    KEY_2 = 0x32,
    KEY_3 = 0x33,
    KEY_4 = 0x34,
    KEY_5 = 0x35,
    KEY_6 = 0x36,
    KEY_7 = 0x37,
    KEY_8 = 0x38,
    KEY_9 = 0x39,
    KEY_A = 0x41,
    KEY_B = 0x42,
    KEY_C = 0x43,
    KEY_D = 0x44,
    KEY_E = 0x45,
    KEY_F = 0x46,
    KEY_G = 0x47,
    KEY_H = 0x48,
    KEY_I = 0x49,
    KEY_J = 0x4a,
    KEY_K = 0x4b,
    KEY_L = 0x4c,
    KEY_M = 0x4d,
    KEY_N = 0x4e,
    KEY_O = 0x4f,
    KEY_P = 0x50,
    KEY_Q = 0x51,
    KEY_R = 0x52,
    KEY_S = 0x53,
    KEY_T = 0x54,
    KEY_U = 0x55,
    KEY_V = 0x56,
    KEY_W = 0x57,
    KEY_X = 0x58,
    KEY_Y = 0x59,
    KEY_Z = 0x5a,
    KEY_CTRL = VK_CONTROL,
    KEY_ALT = VK_MENU,
    KEY_SHIFT = VK_SHIFT,
    KEY_ENTER = VK_RETURN,
    KEY_ESCAPE = VK_ESCAPE,
    KEY_SPACE = VK_SPACE,
    KEY_BACK = VK_BACK,
    KEY_TAB = VK_TAB,
    KEY_PAUSE = VK_PAUSE,
    KEY_SCROLL = VK_SCROLL,
    KEY_INSERT = VK_INSERT,
    KEY_HOME = VK_HOME,
    KEY_END = VK_END,
    KEY_DELETE = VK_DELETE,
    KEY_PGUP = VK_PRIOR,
    KEY_PGDOWN = VK_NEXT,
    KEY_UP = VK_UP,
    KEY_DOWN = VK_DOWN,
    KEY_LEFT = VK_LEFT,
    KEY_RIGHT = VK_RIGHT,
    KEY_F1 = VK_F1,
    KEY_F2 = VK_F2,
    KEY_F3 = VK_F3,
    KEY_F4 = VK_F4,
    KEY_F5 = VK_F5,
    KEY_F6 = VK_F6,
    KEY_F7 = VK_F7,
    KEY_F8 = VK_F8,
    KEY_F9 = VK_F9,
    KEY_F10 = VK_F10,
    KEY_F11 = VK_F11,
    KEY_F12 = VK_F12,

    KEY_COUNT
};

enum {
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,

    MOUSE_BUTTONS_COUNT
};

enum {
    GAMEPAD_UP,
    GAMEPAD_DOWN,
    GAMEPAD_LEFT,
    GAMEPAD_RIGHT,
    GAMEPAD_START,
    GAMEPAD_BACK,
    GAMEPAD_LEFT_THUMB,
    GAMEPAD_RIGHT_THUMB,
    GAMEPAD_LEFT_SHOULDER,
    GAMEPAD_RIGHT_SHOULDER,
    GAMEPAD_A,
    GAMEPAD_B,
    GAMEPAD_X,
    GAMEPAD_Y,

    GAMEPAD_BUTTONS_COUNT
};

typedef struct {
    f32 leftStickX;
    f32 leftStickY;
    f32 rightStickX;
    f32 rightStickY;

    u8 leftTrigger;
    u8 rightTrigger;

    union {
        ButtonState gamepadButtons[GAMEPAD_BUTTONS_COUNT];

        struct {
            ButtonState up;
            ButtonState down;
            ButtonState left;
            ButtonState right;
            ButtonState start;
            ButtonState back;
            ButtonState leftThumb;
            ButtonState rightThumb;
            ButtonState leftShoulder;
            ButtonState rightShoulder;
            ButtonState aButton;
            ButtonState bButton;
            ButtonState xButton;
            ButtonState yButton;
        };
    };

    b32 keys[KEY_COUNT];
    b32 prevKeys[KEY_COUNT];

    i32 mouseX;
    i32 mouseY;
    i32 prevMouseX;
    i32 prevMouseY;
    b32 mouseDragged;
    b32 mouseMoved;
    i32 mouseWheelDelta;
    ButtonState mouseButtons[MOUSE_BUTTONS_COUNT];
} Input;

internal void
updateGamepadButton(DWORD gamepadState, DWORD buttonBitTest, ButtonState *button)
{
    b32 isDown = (gamepadState & buttonBitTest);
    button->changed = isDown != button->isDown;
    button->isDown = isDown;
}

internal f32
processStickValue(f32 value, f32 deadZoneThreshold)
{
    // return 0 if we got a value that are in the dead zone 
    f32 result = 0;

    if (value < -deadZoneThreshold) {
        // NOTE:if we say that the deadszone ends at 0.2, 
        // this will map the value to 0 - 1 instead of 0.2 - 1
        result = (value + deadZoneThreshold) / (32768.0f - deadZoneThreshold);
    }
    else if (value > deadZoneThreshold) {
        result = (value - deadZoneThreshold) / (32767.0f - deadZoneThreshold);
    }

    return result;
}

internal void
updateMouse(ButtonState *button, b32 isDown)
{
    button->changed = isDown != button->isDown;
    button->isDown = isDown;
}


//
// File I/O
//

//remember to free the memory after you called this function!
char *loadTextFile(char *filename)
{
    char *contents = 0;
    FILE *file = fopen(filename, "rb");

    if (file)
    {
        fseek(file, 0, SEEK_END);				    // go to end of file
        size_t fileSize = ftell(file);			    // get file size
        fseek(file, 0, SEEK_SET);				    // go to beginning of file
        contents = (char *)malloc(fileSize + 1);	// allocate memory for file + 1 byte for null terminator

        fread(contents, fileSize, 1, file);		    // read in the file
        contents[fileSize] = 0;					    // insert the null terminator

        fclose(file);
    }
    return contents;
}

void *loadFile(char *filename)
{
    FILE *fp = fopen(filename, "r");
    //move pointer to the end of the file to get length
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    void *contents = (void *)malloc((fileSize) * sizeof(char));

    //read file
    fread(contents, 1, fileSize, fp);
    fclose(fp);
    return contents;
}

//
// Windows
//

typedef struct {
    BITMAPINFO bitmapInfo; // win32 bitmap info structer
    void *pixels; // pixel memory
    i32 width, height;
} BackBuffer;

struct PlatformState {
    b32 running;
    b32 lockFPS;
    i64 frequencyCounter;

    HWND window;
    i32 windowWidth;
    i32 windowHeight;
    b32 fullscreen;
    
    i32 canvasWidth;
    i32 canvasHeight;

    // API
    b32 projection3DFlag; // 2D projection if false
    i32 colorModeFlag;
    b32 doubleBufferDisabledFlag; // double buffering on/off
    b32 fillFlag; // fill flag for shapes
    i32 lineWidth; // strokeweight	
    i32 rectModeFlag;
    Colorf strokeColor;
    Colorf fillColor;
    f32 milliseconds;

    // UI state
    i32 hotWidget; // widget is below the mouse cursor
    i32 activeWidget; // user is interacting with the widget
    i32 keyboardFocus;
    i32 keyentered;
    i32 keymod;
    i32 keychar;
    i32 lastwidget;
};

global PlatformState platformState;
global Input input;

// processing, p5.js global variables
global i32 width;
global i32 height;
global f32 deltaTime; //target seconds per frame
global u64 frameCount = 0;	// number of frames drawn since program start
global i32 mouseX;
global i32 mouseY;
global b32 mouseDragged;
global v2 center;

void draw();
void setup();
void cleanup();
void initOpenGL();
void buildFont(const char *fontName, int fontSize);
void set3dProjection(i32 width, i32 height, f32 fov, f32 nearZ, f32 farZ);
void set2dProjection(i32 width, i32 height);

internal void
toggleFullscreen()
{
    local WINDOWPLACEMENT windowPosition = { sizeof(windowPosition) };
    DWORD windowStyleFlags = GetWindowLong(platformState.window, GWL_STYLE);
    if (windowStyleFlags & WS_OVERLAPPEDWINDOW) {
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        if (GetWindowPlacement(platformState.window, &windowPosition) &&
            GetMonitorInfo(MonitorFromWindow(platformState.window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo)) {
            SetWindowLong(platformState.window, GWL_STYLE, windowStyleFlags & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(platformState.window, HWND_TOP,
                monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else {
        SetWindowLong(platformState.window, GWL_STYLE, windowStyleFlags | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(platformState.window, &windowPosition);
        SetWindowPos(platformState.window, 0, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

void setWindowTitle(const char *title)
{
    SetWindowTextA(platformState.window, title);
}

inline f32
getSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = ((f32)(end.QuadPart - start.QuadPart) / (f32)platformState.frequencyCounter);
    //f64 result = ((end.QuadPart - start.QuadPart) / (f64)platformState.frequencyCounter);
    return result;
}


internal LRESULT CALLBACK
windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message) {
    case WM_QUIT:
    case WM_CLOSE:
    case WM_DESTROY: {
        platformState.running = false;
    } break;

    case WM_SIZE: {
        platformState.windowWidth = LOWORD(lParam);
        platformState.windowHeight = HIWORD(lParam);

        if (platformState.projection3DFlag) {
            set3dProjection(platformState.windowWidth, platformState.windowHeight, 60.f, 1.0f, 500.0f);
        }
        else {
            set2dProjection(platformState.windowWidth, platformState.windowHeight);
        }
    } break;


    case WM_MOUSEWHEEL: {
        i16 wheelDelta = HIWORD(wParam);
        // wheel rotation is a multiple of WHEEL_DELTA, which is set to 120
        input.mouseWheelDelta = (i32)wheelDelta / WHEEL_DELTA;
    } break;

    default: {
        result = DefWindowProc(window, message, wParam, lParam);
    } break;
    }

    return result;
}

enum { OGL2D, OGL3D };
void createCanvas(i32 winWidth = 100, i32 winHeight = 100, const char *caption = "Creative Framework", i32 renderContext = OGL2D, b32 fullscreen = false)
{
    RECT windowRect;
    windowRect.left = 0;
    windowRect.right = winWidth;
    windowRect.top = 0;
    windowRect.bottom = winHeight;

    platformState.canvasWidth = platformState.windowWidth = width = winWidth;
    platformState.canvasHeight = platformState.windowHeight = height = winHeight;

    center = v2(f32(width / 2), f32(height / 2));

    // calculates the required size of the window rectangle, based on the desired client-rectangle size
    if (AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, 0)) {
        winWidth = windowRect.right - windowRect.left;
        winHeight = windowRect.bottom - windowRect.top;
    }

    WNDCLASSA windowClass = { 0 };
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProc;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.hIcon = (HICON)LoadImage(NULL, "data/icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    windowClass.lpszClassName = "creativeFrameworkClass";

    if (RegisterClassA(&windowClass) == 0) {
        quitError("Failed to initialize window class.");
    }

    platformState.window = CreateWindowExA(0, windowClass.lpszClassName, caption,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU |
        CW_USEDEFAULT, CW_USEDEFAULT,
        winWidth, winHeight, 0, 0, 0, 0);

    if (!(platformState.window)) {
        quitError("Failed to create window.");
    }

    initOpenGL();
    if (renderContext == OGL2D)
        set2dProjection(platformState.windowWidth, platformState.windowHeight);
    else
        set3dProjection(platformState.windowWidth, platformState.windowHeight, 60.f, 1.0f, 500.0f);

    buildFont("Verdana", 18);

    if (fullscreen) {
        toggleFullscreen();
    }

    // set global variables
    width = platformState.canvasWidth;
    height = platformState.canvasHeight;

    platformState.fullscreen = false;
    platformState.colorModeFlag = RGB;
    platformState.fillFlag = true;
    platformState.doubleBufferDisabledFlag = false;
    platformState.strokeColor = { 255, 255, 255, 255 };
    platformState.fillColor = { 255, 255, 255, 255 };
    platformState.lineWidth = 1;
    platformState.rectModeFlag = 0;
    platformState.milliseconds = 0;
    input.mouseDragged = false;
    input.mouseMoved = false;
}

#define WinMainNOCRT void __stdcall WinMainCRTStartup() {
#ifdef NOCRT
WinMainNOCRT
#else
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int commandShow)
{
#endif
    setup();

    HDC deviceContext = GetDC(platformState.window);
    i32 monitorRefreshHz = GetDeviceCaps(deviceContext, VREFRESH);

    // 60hz = 0.0166666675 deltaSecondsPerFrame
    // 30hz = 0.0333333351 deltaSecondsPerFrame
    // the time it takes to draw a frame
    i32 framesPerSecond = 60;
    if (framesPerSecond)
        deltaTime = 1.f / (f32)framesPerSecond;
    else
        deltaTime = 1.f / (f32)monitorRefreshHz;

    f32 deltaSecondsPerFrame = deltaTime;

    // get the frequency of the performance counter in counts per seconds
    LARGE_INTEGER frequencyCounterResult;
    QueryPerformanceFrequency(&frequencyCounterResult);
    platformState.frequencyCounter = frequencyCounterResult.QuadPart;

    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter);
    i64 initialCounter = lastCounter.QuadPart;

    platformState.lockFPS = true;
    platformState.running = true;

    // Main loop
    while (platformState.running) {
        /* Input */
        for (int i = 0; i < MOUSE_BUTTONS_COUNT; i++) input.mouseButtons[i].changed = false;
        input.mouseWheelDelta = 0;
        
        MSG message;
        while (PeekMessageA(&message, platformState.window, 0, 0, PM_REMOVE)) {
            switch (message.message) {
                case WM_LBUTTONDOWN: {
                    updateMouse(&input.mouseButtons[MOUSE_LEFT], true);
                } break;
                
                case WM_LBUTTONUP: {
                    updateMouse(&input.mouseButtons[MOUSE_LEFT], false);
                } break;
                
                case WM_MBUTTONDOWN: {
                    updateMouse(&input.mouseButtons[MOUSE_MIDDLE], true);
                } break;
                
                case WM_MBUTTONUP: {
                    updateMouse(&input.mouseButtons[MOUSE_MIDDLE], false);
                    
                } break;
                
                case WM_RBUTTONDOWN: {
                    updateMouse(&input.mouseButtons[MOUSE_RIGHT], true);
                } break;
                
                case WM_RBUTTONUP: {
                    updateMouse(&input.mouseButtons[MOUSE_RIGHT], false);
                } break;
            
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP: {
                    u32 keyCode = (u32)message.wParam;
                    b32 wasDown = ((message.lParam & (1 << 30)) != 0);
                    b32 isDown = ((message.lParam & (1 << 31)) == 0);
                    b32 altDown = message.lParam & (1 << 29);

                    if (keyCode == VK_F4 && altDown) {
                        platformState.running = false;
                        break;
                    }

                    if (keyCode == VK_RETURN && altDown && isDown && isDown != wasDown) {
                        if (platformState.fullscreen)
                            platformState.fullscreen = false;
                        else
                            platformState.fullscreen = true;
                        toggleFullscreen();
                        break;
                    }

                    if (keyCode == VK_ESCAPE) {
                        platformState.running = false;
                        break;
                    }
                } break;

                default: {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
            }
        }

        BYTE keyboard[256];
        GetKeyboardState(keyboard);

        for (int i = 0; i < KEY_COUNT; i++) {
            input.prevKeys[i] = input.keys[i];
            input.keys[i] = keyboard[i] >> 7;
        }

        // save the mouse coordinates for the previous frame, this is used for mousedrag
        input.prevMouseX = input.mouseX;
        input.prevMouseY = input.mouseY;

        POINT mouseP;
        GetCursorPos(&mouseP);
        ScreenToClient(platformState.window, &mouseP);

        // TODO: Fix this so it works when resizing the window
        i32 aspectX = platformState.windowWidth / platformState.canvasWidth;
        i32 aspectY = platformState.windowHeight / platformState.canvasHeight;
        if (aspectX <= 0 || aspectY <= 0) {
            aspectX = 1;
            aspectY = 1;
        }
        
        input.mouseX = mouseX = mouseP.x/aspectX;
        input.mouseY = mouseY = mouseP.y/aspectX; // NOTE: if BOTTOM up DIB: backBuffer.height-mouseP.y;
        
        if (input.mouseX != input.prevMouseX || input.mouseY != input.prevMouseY)
            input.mouseMoved = true;
        else
            input.mouseMoved = false;

        if ((input.mouseButtons[0].isDown) && (input.mouseX != input.prevMouseX || input.mouseY != input.prevMouseY))
            input.mouseDragged = mouseDragged = true;
        else
            input.mouseDragged = mouseDragged = false;

        XINPUT_STATE controllerState;
        if (XInputGetState(0, &controllerState) == ERROR_SUCCESS) {
            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
            f32 stickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

            input.leftStickX = pad->sThumbLX;
            input.leftStickX = processStickValue(input.leftStickX, stickDeadZone);
            input.leftStickY = pad->sThumbLY;
            input.leftStickY = processStickValue(input.leftStickY, stickDeadZone);
            input.rightStickX = pad->sThumbRX;
            input.rightStickX = processStickValue(input.rightStickX, stickDeadZone);
            input.rightStickY = pad->sThumbRY;
            input.rightStickY = processStickValue(input.rightStickY, stickDeadZone);

            // value between 0 and 255 for the left/right trigger analog control
            input.leftTrigger = pad->bLeftTrigger;
            input.rightTrigger = pad->bRightTrigger;

            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_UP, &input.up);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_DOWN, &input.down);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_LEFT, &input.left);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, &input.right);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_START, &input.start);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_BACK, &input.back);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_THUMB, &input.leftThumb);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_THUMB, &input.rightThumb);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &input.leftShoulder);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &input.rightShoulder);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_A, &input.aButton);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_B, &input.bButton);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_X, &input.xButton);
            updateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_Y, &input.yButton);

        }
        else {
            // gamepad is not available
        }

        //
        // Render
        //

        draw();

        glLoadIdentity();
        if (platformState.doubleBufferDisabledFlag)
            glFlush();
        else
            SwapBuffers(deviceContext);

        //
        // Timing
        //
        LARGE_INTEGER endCounter;
        QueryPerformanceCounter(&endCounter);
#if 0
        i64 counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
        f32 secondsElapsedForFrame = getSecondsElapsed(lastCounter, endCounter);
        if (platformState.lockFPS) {
            if (secondsElapsedForFrame < deltaTime) {
                i32 msToSleep = (i32)((deltaTime - secondsElapsedForFrame) * 1000.f);
                if (msToSleep > 0)
                    Sleep(msToSleep);

                // if there is some time left
                while (secondsElapsedForFrame < deltaTime) {
                    QueryPerformanceCounter(&endCounter);
                    secondsElapsedForFrame = getSecondsElapsed(lastCounter, endCounter);
                }
            }
        }
#endif
        QueryPerformanceCounter(&endCounter);
        deltaSecondsPerFrame = getSecondsElapsed(lastCounter, endCounter);
        
        platformState.milliseconds = (1000.f * (lastCounter.QuadPart - initialCounter) /
            platformState.frequencyCounter);
#if 0
        i64 secondsElapsedSinceStart = (lastCounter.QuadPart - initialCounter) /
            platformState.frequencyCounter;

        f64 msPerFrame = 1000. * getSecondsElapsed(lastCounter, endCounter);
        counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
        f64 fps = (f64)platformState.frequencyCounter / (f64)counterElapsed;

        static i64 lastTime = 0;
        if (secondsElapsedSinceStart - lastTime > 1) {
            debugPrint("seconds=%d, ms=%.05f, dt=%.05f, %.03fms, %.03fFPS\n", secondsElapsedSinceStart, platformState.milliseconds,
                deltaSecondsPerFrame, msPerFrame, fps);
            debugPrint("Wheel delta: %d\n", input.mouseWheelDelta);
            debugPrint("MouseX: %d  MouseY: %d\n", mouseX, mouseY);
            lastTime = secondsElapsedSinceStart;
        }
#endif
        QueryPerformanceCounter(&lastCounter);
        frameCount++;
    }

    cleanup();

#ifdef NOCRT
    ExitProcess(0);
#else
    return 0;
#endif
}

//
// Color Functions
//

void colorMode(i32 mode)
{
    switch (mode) {
    case RGB:
        platformState.colorModeFlag = RGB;
        break;
    case HSB:
        platformState.colorModeFlag = HSB;
        break;
    case HSL:
        platformState.colorModeFlag = HSL;
        break;
    }
}

// converts HSB(=HSV) color to RGB color
Colorf colorHSB(Colorf colorHSB)
{
    if (colorHSB.r < 0 || colorHSB.r > 255)
        colorHSB.r = 0;
    if (colorHSB.g < 0 || colorHSB.g > 255)
        colorHSB.g = 0;
    if (colorHSB.b < 0 || colorHSB.b > 255)
        colorHSB.b = 0;

    f32 r = 0, g = 0, b = 0, h, s, v;
    h = colorHSB.r / 256.0f;
    s = colorHSB.g / 256.0f;
    v = colorHSB.b / 256.0f;

    if (s == 0.0)
        r = g = b = v;
    else {
        f32 f, p, q, t;
        i32 i;
        h *= 6.0;
        i = (i32)h;
        f = h - i;
        p = v * (1.0f - s);
        q = v * (1.0f - (s * f));
        t = v * (1.0f - (s * (1.0f - f)));

        switch (i) {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        case 5:
            r = v; g = p; b = q;
            break;
        }
    }

    Colorf col;
    col.r = r * 255.0f;
    col.g = g * 255.0f;
    col.b = b * 255.0f;
    col.a = 255;
    return col;
}

// converts HSL color to RGB color
Colorf colorHSL(Colorf colorHSL)
{
    f32 r, g, b, h, s, l;
    f32 tempCol1, tempCol2, tempRed, tempGreen, tempBlue;
    h = colorHSL.r / 256.0f;
    s = colorHSL.g / 256.0f;
    l = colorHSL.b / 256.0f;

    if (s == 0)
        r = g = b = l;
    else {
        if (l < 0.5)
            tempCol2 = l * (1 + s);
        else
            tempCol2 = (l + s) - (l * s);

        tempCol1 = 2 * l - tempCol2;
        tempRed = h + 1.0f / 3.0f;

        if (tempRed > 1)
            tempRed--;

        tempGreen = h;
        tempBlue = h - 1.0f / 3.0f;

        if (tempBlue < 0)
            tempBlue++;

        // red
        if (tempRed < 1.f / 6.f)
            r = tempCol1 + (tempCol2 - tempCol1) * 6.f * tempRed;
        else if (tempRed < 0.5f)
            r = tempCol2;
        else if (tempRed < 2.f / 3.f)
            r = tempCol1 + (tempCol2 - tempCol1) * ((2.f / 3.f) - tempRed) * 6.f;
        else
            r = tempCol1;

        // green
        if (tempGreen < 1.f / 6.f)
            g = tempCol1 + (tempCol2 - tempCol1) * 6.f * tempGreen;
        else if (tempGreen < 0.5)
            g = tempCol2;
        else if (tempGreen < 2.f / 3.f)
            g = tempCol1 + (tempCol2 - tempCol1) * ((2.f / 3.f) - tempGreen) * 6.f;
        else
            g = tempCol1;

        // blue
        if (tempBlue < 1.0 / 6.0)
            b = tempCol1 + (tempCol2 - tempCol1) * 6.f * tempBlue;
        else if (tempBlue < 0.5)
            b = tempCol2;
        else if (tempBlue < 2.f / 3.f)
            b = tempCol1 + (tempCol2 - tempCol1) * ((2.f / 3.f) - tempBlue) * 6.f;
        else
            b = tempCol1;
    }

    Colorf col;
    col.r = r * 255.f;
    col.g = g * 255.f;
    col.b = b * 255.f;
    col.a = 255;
    return col;
}

Colorf checkColorMode(Color inColor)
{
    Colorf col = { (f32)inColor.r, (f32)inColor.g, (f32)inColor.b, 255.f };
    Colorf outColor;

    if (platformState.colorModeFlag == HSB)
        outColor = colorHSB(col);
    else if (platformState.colorModeFlag == HSL)
        outColor = colorHSL(col);
    else // RGB
        outColor = col;

    return outColor;
}


//
// OpenGL API
//

void initOpenGL()
{
    HDC deviceContext = GetDC(platformState.window);

    // define the pixelformat for OpenGL, double buffering and RGBA
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32; // bits per pixel
    //pfd.cStencilBits = 8
    //pfd.cAlphaBits = 8;  // NOTE: is this needed?
    //pfd.cDepthBits = 16; // depth of the z-buffer Needed for 3d?
    pfd.dwLayerMask = PFD_MAIN_PLANE; // sets the pfd to be the main drawing plane

    // get the best matching pixel format
    i32 pixelFormatIndex = ChoosePixelFormat(deviceContext, &pfd);

    // NOTE: is this needed?
    PIXELFORMATDESCRIPTOR pixelFormat;
    DescribePixelFormat(deviceContext, pixelFormatIndex,
        sizeof(pixelFormat), &pixelFormat);

    // try to set the pixel format on the window
    SetPixelFormat(deviceContext, pixelFormatIndex, &pixelFormat);
    //SetPixelFormat(deviceContext, pixelFormatIndex, &pfd);

    // create a opengl rendering context
    HGLRC openGLRC = wglCreateContext(deviceContext);

    // makes the specified OpenGL rendering context the calling thread's current rendering context
    if (wglMakeCurrent(deviceContext, openGLRC) == 0) {
        // NOTE: the program won't crash if the other OpenGL calls would fail so we can
        // safely catch the error here
        quitError("Failed to create OpenGL rendering context.");
    }
}


u32	base;								// font base display list
void buildFont(const char *fontName, int fontSize)
{
    HFONT font;
    base = glGenLists(96);

    font = CreateFont(-fontSize,		// font height, negative number = get a font based on the character height
        0,								// font width
        0,								// angle of escapement
        0,								// orientation angle
        FW_BOLD,						// font weight
        FALSE,							// italic
        FALSE,							// underline
        FALSE,							// strikeout
        ANSI_CHARSET,					// character set identifier
        OUT_TT_PRECIS,					// output precision, true type
        CLIP_DEFAULT_PRECIS,			// clipping precision
        ANTIALIASED_QUALITY,			// output quality
        FF_DONTCARE | DEFAULT_PITCH,	// family and pitch
        fontName);						// font name

    HDC dc = GetDC(platformState.window);
    SelectObject(dc, font);
    // create 96 characters, starting at character 32
    wglUseFontBitmaps(dc, 32, 96, base);
    ReleaseDC(platformState.window, dc);
}

void freeFont()
{
    if (base != 0)
        glDeleteLists(base, 96);
}

void text(int xPos, int yPos, const char *str, ...)
{
    if ((base == 0) || (!str))
        return;

    va_list	args;
    char buffer[256];

    va_start(args, str);
    vsprintf(buffer, str, args);
    va_end(args);

    glRasterPos2i(xPos, yPos);

    glPushAttrib(GL_LIST_BIT);

    // starting at character 32
    glListBase(base - 32);
    glCallLists((int)strlen(buffer), GL_UNSIGNED_BYTE, buffer);
    glPopAttrib();
}


void disableDoubleBuffer()
{
    platformState.doubleBufferDisabledFlag = true;
    glDrawBuffer(GL_FRONT);
}


void set2dProjection(i32 windowWidth = platformState.windowWidth, i32 windowHeight = platformState.windowHeight)
{
    glViewport(0, 0, windowWidth, windowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0, platformState.canvasWidth, platformState.canvasHeight, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    platformState.projection3DFlag = false;

    // blending
    glEnable(GL_BLEND);
    // color = (a*source)+(b*dest)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// set orthographic projection
// objects with the same dimension appear the same size, regardless of 
// whether they are near or far from the camera
void ortho(f32 left = 0.f, f32 right = platformState.canvasWidth, f32 bottom = platformState.canvasHeight, f32 top = 0.f, f32 nearZ = 0.0f, f32 farZ = 500.f)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(left, right, bottom, top, nearZ, farZ);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    platformState.projection3DFlag = false;
}

// set perspective projection
void set3dProjection(i32 windowWidth = platformState.windowWidth, i32 windowHeight = platformState.windowHeight, f32 fov = 60.f, f32 nearZ = 1.0f, f32 farZ = 500.0f)
{
    // prevent divide by zero
    if (!windowHeight) return;

    f32 aspect = (f32)platformState.canvasWidth / (f32)platformState.canvasHeight;

    glViewport(0, 0, windowWidth, windowHeight);

    // center
    //glViewport(windowWidth / 4.f, windowHeight / 4.f, windowWidth / 2.f, windowHeight / 2.f);

    // switch to the projection matrix and reset it
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // set camera perspective
    gluPerspective(fov,		// camera angle, field of view in degrees, set to 45 degrees viewing angle
        aspect,				// aspect ratio
        nearZ,				// near z clipping coordinate
        farZ);				// far z clipping coordinate, start and end point for how deep we can draw into the screen

    // switch to GL_MODELVIEW, tells OGL that all future transformations will affect what we draw
    // reset the modelview matrix, wich is where the object information is stored, sets x,y,z to zero
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // enable depth buffer
    glEnable(GL_DEPTH_TEST);

    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    //NOTE: this has been turned on by set2dProjection
    glDisable(GL_BLEND);
    platformState.projection3DFlag = true;
}

// set perspective projection
void perspective(f32 fov, f32 aspect, f32 nearZ, f32 farZ)
{
    // switch to the projection matrix and reset it
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // set camera perspective
    gluPerspective(fov,		// camera angle, field of view in degrees, set to 45 degrees viewing angle
        aspect,				// aspect ratio
        nearZ,				// near z clipping coordinate
        farZ);				// far z clipping coordinate, start and end point for how deep we can draw into the screen

    // switch to GL_MODELVIEW, tells OGL that all future transformations will affect what we draw
    // reset the modelview matrix, wich is where the object information is stored, sets x,y,z to zero
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // enable depth buffer
    //glEnable(GL_DEPTH_TEST);
    // don't calculate inside of objects
    //glEnable(GL_CULL_FACE);		
    // counter clock-wise polygons face out
    //glFrontFace(GL_CCW);
    platformState.projection3DFlag = true;
}

void loadIdentity()
{
    glLoadIdentity();
}

void pushMatrix()
{
    glPushMatrix();
}

void popMatrix()
{
    glPopMatrix();
}

void translate(f32 x, f32 y, f32 z = 0.f)
{
    glTranslatef(x, y, z);
}

void rotateX(f32 angle)
{
    glRotatef(angle, 1.0f, 0.0f, 0.0f);
}

void rotateY(f32 angle)
{
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
}

void rotateZ(f32 angle)
{
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
}

// rotation in radians
void rotate(f32 angle)
{
    f32 deg = degrees(angle);
    glRotatef(deg, 0.0f, 0.0f, 1.0f);
}


// Color

// clear screen and depth buffers
inline void background(Color col, i32 alpha = 255)
{
    Colorf newColor = checkColorMode(col);
    f32 r = newColor.r / 255.f;
    f32 g = newColor.g / 255.f;
    f32 b = newColor.b / 255.f;
    f32 a = (f32)alpha / 255.f;

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void background(i32 inColor, i32 alpha = 255)
{
    Color col = { inColor, inColor, inColor, alpha };
    background(col);
}

inline void background(i32 r, i32 g, i32 b, i32 a = 255)
{
    Color col = { r, g, b, a };
    background(col);
}

inline void clear(Color col, i32 alpha = 255)
{
    background(col, alpha);
}

inline void clear(i32 inColor, i32 alpha = 255)
{
    background(inColor, alpha);
}

inline void clear(i32 r, i32 g, i32 b, i32 a = 255)
{
    background(r, g, b, a);
}

inline void pointSize(f32 value)
{
    glPointSize(value);
}

inline void noStroke()
{
    platformState.lineWidth = 0;
    glLineWidth(1);
}

inline void strokeWeight(i32 value)
{
    platformState.lineWidth = value;
    glLineWidth((f32)value);
    glPointSize((f32)value);
}

inline void stroke(Color col, i32 alpha = 255)
{
    Colorf newColor = checkColorMode(col);
    f32 r = newColor.r / 255.f;
    f32 g = newColor.g / 255.f;
    f32 b = newColor.b / 255.f;
    f32 a = (f32)alpha / 255.f;

    platformState.strokeColor = { r, g, b, a };
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
}

inline void stroke(i32 inColor, i32 alpha = 255)
{
    Color col = { inColor, inColor, inColor, alpha };
    stroke(col);
}

inline void stroke(i32 r, i32 g, i32 b, i32 a = 255)
{
    Color col = { r, g, b, a };
    stroke(col);
}

inline void noFill()
{
    platformState.fillFlag = false;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

inline void fill(Color col)
{
    platformState.fillFlag = true;
    //if (platformState.colorModeFlag == HSB)
    Colorf newColor = checkColorMode(col);
    f32 r = newColor.r / 255.0f;
    f32 g = newColor.g / 255.0f;
    f32 b = newColor.b / 255.0f;
    f32 a = (f32)col.a / 255.0f;

    platformState.fillColor = { r, g, b, a };
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

inline void fill(i32 r, i32 g, i32 b, i32 a = 255)
{
    Color col = { r, g, b, a };
    fill(col);
}

inline void fill(Color col, i32 alpha)
{
    col.a = alpha;
    fill(col);
}

inline void fill(i32 col, i32 alpha = 255)
{
    Color color = { col, col, col, alpha };
    fill(color);
}


// 2D drawing

enum { CENTER = 1 };
inline void rectMode(const i32 mode)
{
    platformState.rectModeFlag = mode;
}

inline void line(i32 x0, i32 y0, i32 x1, i32 y1)
{
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    glBegin(GL_LINES);
    glVertex2i(x0, y0);
    glVertex2i(x1, y1);
    glEnd();
}

inline void line(f32 x0, f32 y0, f32 x1, f32 y1)
{
    line((i32)x0, (i32)y0, (i32)x1, (i32)y1);
}

inline void point(i32 x, i32 y)
{
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

inline void point(f32 x, f32 y, f32 z)
{
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    glBegin(GL_POINTS);
    glVertex3f(x, y, z);
    glEnd();
}

inline void rect(i32 x, i32 y, i32 w, i32 h)
{
    //glRectf(50.0f, 50.0f, 25.0f, 25.0f);
    i32 dx = 0, dy = 0;
    if (platformState.fillFlag) {
        glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);

        glBegin(GL_QUADS);
        if (platformState.rectModeFlag == CENTER) {
            dx = x - w / 2;
            dy = y - h / 2;
            glVertex2i(dx, dy);
            glVertex2i(dx + w, dy);
            glVertex2i(dx + w, dy + h);
            glVertex2i(dx, dy + h);
        }
        else {
            glVertex2i(x, y);
            glVertex2i(x + w, y);
            glVertex2i(x + w, y + h);
            glVertex2i(x, y + h);
        }
        glEnd();

        glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    }

    if (platformState.lineWidth > 0) {
        glBegin(GL_LINE_LOOP);
        if (platformState.rectModeFlag == CENTER) {
            dx = x - w / 2;
            dy = y - h / 2;
            glVertex2i(dx, dy);
            glVertex2i(dx + w, dy);
            glVertex2i(dx + w, dy + h);
            glVertex2i(dx, dy + h);
        }
        else {
            glVertex2i(x, y);
            glVertex2i(x + w, y);
            glVertex2i(x + w, y + h);
            glVertex2i(x, y + h);
        }
        glEnd();
    }
}

inline void rect(f32 x, f32 y, f32 w, f32 h)
{
    rect((i32)x, (i32)y, (i32)w, (i32)h);

}

void quad(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 x4, i32 y4)
{
    if (platformState.fillFlag) {
        glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
        glBegin(GL_QUADS);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glVertex2i(x3, y3);
        glVertex2i(x4, y4);
        glEnd();
        glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    }

    if (platformState.lineWidth > 0) {
        glBegin(GL_LINE_LOOP);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glVertex2i(x3, y3);
        glVertex2i(x4, y4);
        glEnd();
    }
}

void circle(i32 x, i32 y, i32 radius)
{
    if (platformState.fillFlag) {
        glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(x, y);
        for (f32 angle = 0.f; angle < TWO_PI; angle += 0.1f) {
            glVertex2f((f32)(x + cosinus(angle) * radius), (f32)(y + sinus(angle) * radius));
        }

        // close the circle
        glVertex2f((f32)x + (f32)cosinus(TWO_PI) * (f32)radius, (f32)y + (f32)sinus(TWO_PI) * (f32)radius);
        glEnd();
        glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    }

    if (platformState.lineWidth > 0) {
        glBegin(GL_LINE_STRIP);
        for (f32 angle = 0; angle < PI * 4; angle += (PI / 50.0f)) {
            glVertex2f((f32)(x + sinus(angle) * radius), (f32)(y + cosinus(angle) * radius));
        }
        glEnd();
    }
}

void circle(f32 x, f32 y, f32 radius)
{
    circle((i32)x, (i32)y, (i32)radius);
}

void ellipse(i32 x, i32 y, i32 r1, i32 r2 = 0)
{
    if (r2 == 0)
        r2 = r1;
    if (platformState.fillFlag) {
        glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(x, y);
        for (f32 angle = 0.f; angle < TWO_PI; angle += 0.02f) {
            glVertex2f(f32(x + cos(angle) * r1), f32(y + sin(angle) * r2));
        }
        glEnd();
        glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    }

    if (platformState.lineWidth > 0) {
        glBegin(GL_LINE_STRIP);
        for (f32 angle = 0; angle < PI * 4; angle += (PI / 50.0f)) {
            glVertex2f((f32)(x + sin(angle) * r1), (f32)(y + cos(angle) * r2));
        }
        glEnd();
    }
}

void arc(i32 x, i32 y, i32 r1, i32 r2, f32 start, f32 end)
{
    if (r2 == 0)
        r2 = r1;

    if (platformState.lineWidth > 0) {
        glBegin(GL_LINE_STRIP);
        for (f32 angle = start; angle <= end; angle += 0.02f)
            glVertex2f((f32)x + cosinus(angle) * (f32)r1, (f32)y + sinus(angle) * (f32)r2);

        glEnd();
    }
}

void triangle(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3)
{
    glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glVertex2i(x3, y3);
    glEnd();
}

enum { CLOSE = 1 };
void beginShape(i32 close = 0)
{
    if (platformState.fillFlag) {
        glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
        glBegin(GL_TRIANGLE_FAN);
        //glBegin(GL_POLYGON);
    }
    else {
        glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);

        if (close == CLOSE)
            glBegin(GL_LINE_LOOP);
        else
            glBegin(GL_LINE_STRIP);
    }
}

void endShape()
{
    glEnd();
}


// Light

void noLights()
{
    glDisable(GL_LIGHTING);
}

// set default light
void lights()
{
    // light values and coordinates
    f32 ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    f32 diffuseLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    f32 lightPos[] = { 0.f, 50.0f, 100.0f, 0.0f };

    glEnable(GL_LIGHTING);

    // setup and enable light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHT0);

    // enable color tracking
    glEnable(GL_COLOR_MATERIAL);
    // set material properties to follow glColor values
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void ambientLight(f32 r, f32 g, f32 b, f32 a = 1.0f)
{
    glEnable(GL_LIGHTING);

    // ambient light, directionless light, dark white light
    float ambientLight[] = { r, g, b,a };

    // set light model to use ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    // enable material color tracking
    glEnable(GL_COLOR_MATERIAL);
    // front material ambient and diffuse colors track glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

// diffuse light, light from a direction
void directionalLight(f32 r, f32 g, f32 b, f32 x, f32 y, f32 z)
{
    // position/direction of light
    f32 lightPos[] = { x, y, z, 1.0 };
    f32 ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    f32 diffuseLight[] = { r, g, b, 1.0f };
    glEnable(GL_LIGHTING);

    // setup and enable light 0
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glEnable(GL_LIGHT0);

    // enable color tracking
    glEnable(GL_COLOR_MATERIAL);
    // set Material properties to follow glColor values
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void specularLight(f32 r, f32 g, f32 b, f32 x, f32 y, f32 z)
{
    //position/direction of light
    f32 lightPos[] = { x, y, z, 1.f };
    f32 ambientLight[] = { r * 0.3f, g * 0.3f, b * 0.3f, 1.0f };
    f32 diffuseLight[] = { r * 0.7f, g * 0.7f, b * 0.7f, 1.f };
    f32 specularLight[] = { r, g, b, 1.0f };
    f32 specularReflectance[] = { r, g, b, 1.0f };

    glEnable(GL_LIGHTING);

    // setup and enable light 0
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glEnable(GL_LIGHT0);

    // enable color tracking
    glEnable(GL_COLOR_MATERIAL);
    // set material properties to follow glColor values
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // set materials specular reflectivity and shininess
    glMaterialfv(GL_FRONT, GL_SPECULAR, specularReflectance);
    /*	The GL_SHININESS property sets the specular exponent of the material, which specifies how small and focused the specular highlight is.
        The larger the value, the more shiny and pronounced the surface. The range of this parameter is 1-128 */
    glMateriali(GL_FRONT, GL_SHININESS, 40);
}

void spotLight(f32 r, f32 g, f32 b, f32 x, f32 y, f32 z, f32 dirX, f32 dirY, f32 dirZ, f32 angle)
{
    f32 lightPos[] = { x, y, z, 1.0f };
    f32 specular[] = { r, g, b, 1.0f };
    f32 specularRef[] = { r, g, b, 1.0f };
    f32 ambient[] = { r * 0.5f, g * 0.5f, b * 0.5f, 1.0f };
    f32 spotDir[] = { dirX, dirY, dirZ };

    glEnable(GL_LIGHTING);

    // setup and enable light 0
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDir);

    // set cut off angle to 60 degrees to create spot effect
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, angle);

    glEnable(GL_LIGHT0);

    // enable color tracking
    glEnable(GL_COLOR_MATERIAL);

    // set material properties to follow glColor values
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    // set materials specular reflectivity and shininess
    glMaterialfv(GL_FRONT, GL_SPECULAR, specularRef);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
}


// Blending

enum { ALPHA_BLEND, ADDITIVE_BLEND, MULTIPLIED_BLEND };

void blendMode(i32 mode)
{
    switch (mode) {
    case ALPHA_BLEND:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case ADDITIVE_BLEND:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
    case MULTIPLIED_BLEND:
        glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
        break;
    }
}


// Fog

void enableFog(f32 start, f32 end, f32 r, f32 g, f32 b, f32 a)
{
    // fog setup
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, /*GL_EXP2*/GL_LINEAR);		// fog equation
    //glFogf(GL_FOG_DENSITY, 0.6f); //only works if GL_FOG_MODE == GL_EXP eller GL_EXP2
    // Note that GL_FOG_START and GL_FOG_END only have an effect on GL_LINEAR fog
    // where the fog starts and ends
    glFogf(GL_FOG_START, start);
    glFogf(GL_FOG_END, end);
    f32 col[] = { r, g, b, a };
    glFogfv(GL_FOG_COLOR, col);
}

void disableFog()
{
    glDisable(GL_FOG);
}


// 3D shapes

void vertex(f32 x, f32 y, f32 z = 0.f)
{
    glVertex3f(x, y, z);
}

void cube(f32 size = 1.f)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    for (i32 i = 0; i < 2; i++) {
        if (platformState.lineWidth == 0) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
        }

        glBegin(GL_QUADS);
        glNormal3f(0.0, 0.0, 1.0);
        // front face
        glVertex3f(size, size, size);
        glVertex3f(-size, size, size);
        glVertex3f(-size, -size, size);
        glVertex3f(size, -size, size);

        // left face
        glNormal3f(-1.0, 0.0, 0.0);
        glVertex3f(-size, size, size);
        glVertex3f(-size, size, -size);
        glVertex3f(-size, -size, -size);
        glVertex3f(-size, -size, size);

        // back face
        glNormal3f(0.0, 0.0, -1.0);
        glVertex3f(size, size, -size);
        glVertex3f(-size, size, -size);
        glVertex3f(-size, -size, -size);
        glVertex3f(size, -size, -size);

        // right face
        glNormal3f(1.0, 0.0, 0.0);
        glVertex3f(size, size, -size);
        glVertex3f(size, size, size);
        glVertex3f(size, -size, size);
        glVertex3f(size, -size, -size);

        // top face
        glNormal3f(0.0, 1.0, 0.0);
        glVertex3f(size, size, size);
        glVertex3f(-size, size, size);
        glVertex3f(-size, size, -size);
        glVertex3f(size, size, -size);

        // bottom face
        glNormal3f(0.0, -1.0, 0.0);
        glVertex3f(size, -size, size);
        glVertex3f(-size, -size, size);
        glVertex3f(-size, -size, -size);
        glVertex3f(size, -size, -size);
        glEnd();

        if (platformState.lineWidth == 0)
            break;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
    }
}

void plane(f32 w, f32 h)
{
    glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
    glBegin(GL_QUADS);
    //glNormal3f(0.0, 0.0, 1.0);
    //glColor3ub((GLubyte)col.r, (GLubyte)col.g, (GLubyte)col.b);
    //glColor3f(1.0, 0.0, 0.0);
    glVertex3f(w / 2, h / 2, 0.0f);
    glVertex3f(-w / 2, h / 2, 0.0f);
    glVertex3f(-w / 2, -h / 2, 0.0f);
    glVertex3f(w / 2, -h / 2, 0.0f);
    glEnd();
}

void sphere(f32 radius, i32 slices = 24, i32 stacks = 16)
{
    glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
#if 1
    f32 drho = PI / (f32)stacks;
    f32 dtheta = 2.0f * PI / (f32)slices;
    f32 ds = 1.0f / (f32)slices;
    f32 dt = 1.0f / (f32)stacks;
    f32 t = 1.0f;
    f32 s = 0.0f;

    for (i32 i = 0; i < stacks; i++)
    {
        f32 rho = (f32)i * drho;
        f32 srho = sin(rho);
        f32 crho = cos(rho);
        f32 srhodrho = sin(rho + drho);
        f32 crhodrho = cos(rho + drho);

        glBegin(GL_TRIANGLE_STRIP);
        s = 0.0f;
        for (i32 j = 0; j <= slices; j++)
        {
            f32 theta = (j == slices) ? 0.0f : j * dtheta;
            f32 stheta = -sin(theta);
            f32 ctheta = cos(theta);

            f32 x = stheta * srho;
            f32 y = ctheta * srho;
            f32 z = crho;

            glTexCoord2f(s, t);
            glNormal3f(x, y, z);
            glVertex3f(x * radius, y * radius, z * radius);

            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;
            glTexCoord2f(s, t - dt);
            s += ds;
            glNormal3f(x, y, z);
            glVertex3f(x * radius, y * radius, z * radius);
        }
        glEnd();

        t -= dt;
    }
#else
    GLUquadric *q = gluNewQuadric();
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
#endif
}

void torus(f32 majorRadius, f32 minorRadius, i32 numMajor = 61, i32 numMinor = 37)
{
    glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
    v3 normal;
    f32 vNormal[3];
    f64 majorStep = 2.0f * PI / numMajor;
    f64 minorStep = 2.0f * PI / numMinor;
    int i, j;

    for (i = 0; i < numMajor; ++i) {
        f64 a0 = i * majorStep;
        f64 a1 = a0 + majorStep;
        f32 x0 = cosinus((f32)a0);
        f32 y0 = sinus((f32)a0);
        f32 x1 = cosinus((f32)a1);
        f32 y1 = sinus((f32)a1);

        glBegin(GL_TRIANGLE_STRIP);
        for (j = 0; j <= numMinor; ++j) {
            f64 b = j * minorStep;
            f32 c = cosinus((f32)b);
            f32 r = minorRadius * c + majorRadius;
            f32 z = minorRadius * sinus((f32)b);

            glTexCoord2f((f32)(i) / (f32)(numMajor), (f32)(j) / (f32)(numMinor));
            normal.x = x0 * c;
            normal.y = y0 * c;
            normal.z = z / minorRadius;
            normal = v3Normalize(normal);
            vNormal[0] = normal.x;
            vNormal[1] = normal.y;
            vNormal[2] = normal.z;
            glNormal3fv(vNormal);
            glVertex3f(x0 * r, y0 * r, z);

            glTexCoord2f((f32)(i + 1) / (f32)(numMajor), (f32)(j) / (f32)(numMinor));
            vNormal[0] = x1 * c;
            vNormal[1] = y1 * c;
            vNormal[2] = z / minorRadius;
            glNormal3fv(vNormal);
            glVertex3f(x1 * r, y1 * r, z);
        }
        glEnd();
    }
}

void cylinder(f32 w, f32 h, i32 slices = 32, i32 stacks = 7)
{
    glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
    GLUquadricObj *q = gluNewQuadric();
    gluCylinder(q, w, w, h, slices, stacks);
    /*glBegin(GL_TRIANGLE_FAN);
    for (f32 angle = 0; angle < 360; angle++)
    {
    glVertex2f(sin(angle) * width, cos(angle) * width);
    }
    glEnd();
    translate(0.0f, height*2, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    for (f32 angle = 0; angle < 360; angle++)
    {
    glVertex2f(sin(angle) * width, cos(angle) * width);
    }
    glEnd();*/
    gluDeleteQuadric(q);
}

void cone(f32 w, f32 h, i32 slices = 32, i32 stacks = 7)
{
    glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
    GLUquadricObj *q = gluNewQuadric();
    gluCylinder(q, w, 0, h, slices, stacks);

    glBegin(GL_TRIANGLE_FAN);
    for (f32 angle = 0; angle < 360; angle++)
        glVertex2f(sin(angle) * w, cos(angle) * w);

    glEnd();

    gluDeleteQuadric(q);
}

void cone2(f32 w, f32 h)
{
    //glColor4f(fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    f32 x, y, angle;
    i32 pivot = 1;

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 2.0f);

    for (angle = 0.0f; angle < (2.0f * PI); angle += (PI / 8.0f)) {
        // calculate vertex
        x = sin(angle);
        y = cos(angle);

        // alternate colors
        if ((pivot % 2) == 0)
            glColor3f(0.0f, 1.0f, 0.0f);
        else
            glColor3f(0.0f, 0.0f, 1.0f);

        pivot++;
        glVertex2f(x, y);
    }

    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (angle = 0.0f; angle < (2.0f * PI); angle += (PI / 8.0f)) {
        x = sin(angle);
        y = cos(angle);

        if ((pivot % 2) == 0)
            glColor3f(0.0f, 1.0f, 0.0f);
        else
            glColor3f(0.0f, 0.0f, 1.0f);

        pivot++;
        glVertex2f(x, y);
    }

    glEnd();
}

void box(f32 w, f32 h = 0, f32 depth = 0)
{
    if (h == 0 && depth == 0)
        h = depth = w;

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
    for (i32 i = 0; i < 2; i++) {
        if (platformState.lineWidth == 0) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
        }
        glBegin(GL_QUADS);
        // front face
        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(w, h, depth);
        glVertex3f(-w, h, depth);
        glVertex3f(-w, -h, depth);
        glVertex3f(w, -h, depth);

        // left face
        glNormal3f(-1.0, 0.0, 0.0);
        glVertex3f(-w, h, depth);
        glVertex3f(-w, h, -depth);
        glVertex3f(-w, -h, -depth);
        glVertex3f(-w, -h, depth);

        // back face
        glNormal3f(0.0, 0.0, -1.0);
        glVertex3f(w, h, -depth);
        glVertex3f(-w, h, -depth);
        glVertex3f(-w, -h, -depth);
        glVertex3f(w, -h, -depth);

        // right face
        glNormal3f(1.0, 0.0, 0.0);
        glVertex3f(w, h, -depth);
        glVertex3f(w, h, depth);
        glVertex3f(w, -h, depth);
        glVertex3f(w, -h, -depth);

        //  top face
        glNormal3f(0.0, 1.0, 0.0);
        glVertex3f(w, h, depth);
        glVertex3f(-w, h, depth);
        glVertex3f(-w, h, -depth);
        glVertex3f(w, h, -depth);

        //bottom face
        glNormal3f(0.0, -1.0, 0.0);
        glVertex3f(w, -h, depth);
        glVertex3f(-w, -h, depth);
        glVertex3f(-w, -h, -depth);
        glVertex3f(w, -h, -depth);
        glEnd();

        if (platformState.lineWidth == 0)
            break;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
    }
}

void pyramid(f32 w, f32 h)
{
    glColor4f(platformState.fillColor.r, platformState.fillColor.g, platformState.fillColor.b, platformState.fillColor.a);
    //draw pyramid
    glBegin(GL_TRIANGLES);
    //front
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, h, 0.0f);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-w, -w, w);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(w, -w, w);

    //right
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, h, 0.0f);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(w, -w, w);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(w, -w, -w);

    //back
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, h, 0.0f);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(w, -w, -w);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-w, -w, -w);

    //left
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, h, 0.0f);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-w, -w, -w);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-w, -w, w);
    glEnd();

    //bottom of pyramid
    glBegin(GL_QUADS);
    //glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(w, -w, w);
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-w, -w, w);
    //glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-w, -w, -w);
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(w, -w, -w);
    glEnd();
}


// Textures

struct Image
{
    u32 id = 0;
    i32 width = 0;
    i32 height = 0;
};

Image loadImage(const char *filename)
{
    i32 textureWidth, textureHeight, bpp;
    u8 *pix = stbi_load(filename, &textureWidth, &textureHeight, &bpp, 3);
    u32 id;

    // generate 1 texture
    glGenTextures(1, &id);

    // bind the texture id to a texture target
    glBindTexture(GL_TEXTURE_2D, id);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// linear min filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// linear mag filter

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);
    stbi_image_free(pix);

    Image texture;
    texture.id = id;
    texture.width = textureWidth;
    texture.height = textureHeight;

    return texture;
}

u32 loadTexture(const char *filename)
{
    int w, h, bpp;
    unsigned char *pix = stbi_load(filename, &w, &h, &bpp, 3);
    unsigned int id;

    // generate 1 texture
    glGenTextures(1, &id);

    // bind the texture id to a texture target
    glBindTexture(GL_TEXTURE_2D, id);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// linear min filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// linear mag filter

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);
    stbi_image_free(pix);
    return id;
}

void image(Image texture, i32 x, i32 y, i32 w = 0, i32 h = 0)
{
    // check if wireframe rendering is turned on
    if (platformState.fillFlag == false)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glLoadIdentity();
    glTranslatef((f32)x, (f32)y, 0.0f);

    glBindTexture(GL_TEXTURE_2D, texture.id);

    if (w == 0)
        w = texture.width;
    if (h == 0)
        h = texture.height;

    // place texture on quad
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f((f32)w, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f((f32)w, (f32)h);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, (f32)h);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    //glLoadIdentity();

    // change back the stroke color to the current selected
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);

    // turn on noFill again
    if (platformState.fillFlag == false)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void sprite(u32 tex, i32 x, i32 y, i32 w, i32 h)
{
    // check if wireframe rendering is turned on
    if (platformState.fillFlag == false)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glLoadIdentity();
    glTranslatef((f32)x, (f32)y, 0.0f);

    glBindTexture(GL_TEXTURE_2D, tex);

    // place texture on quad
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f((f32)w, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f((f32)w, (f32)h);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, (f32)h);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    //glLoadIdentity();

    // change back the stroke color to the current selected
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);

    // turn on noFill again
    if (platformState.fillFlag == false)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void sprite3d(u32 tex, v3 pos, i32 w, i32 h)
{
    // check if wireframe rendering is turned on
    if (platformState.fillFlag == false)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    pushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glBindTexture(GL_TEXTURE_2D, tex);

    // place texture on quad
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f((f32)w, 0.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f((f32)w, (f32)h, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.0f, (f32)h, 1.0f);
    glEnd();

    popMatrix();
    glDisable(GL_TEXTURE_2D);
    //glLoadIdentity();

    // change back the stroke color to the current selected
    glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);

    // turn on noFill again
    if (platformState.fillFlag == false)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}


struct Sprite
{
    u32 id = 0;
    i32 width = 0;
    i32 height = 0;
    i32 frameWidth = 0; // frame dimensions in spritesheet
    i32 frameHeight = 0;
    u32 *pixels = 0;

    void freeTexture()
    {
        if (id != 0) {
            glDeleteTextures(1, &id);
            id = 0;
        }

        if (pixels != 0) {
            free(pixels);
            pixels = 0;
        }
    }

    u32 loadTexture(const char *filename)
    {
        i32 bpp;
        u8 *pix = stbi_load(filename, &width, &height, &bpp, 3);

        // generate 1 texture
        glGenTextures(1, &id);
        // bind the texture id to a texture target
        glBindTexture(GL_TEXTURE_2D, id);
        // set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// linear min filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// linear mag filter
        // load texture from memory buffer
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);

        stbi_image_free(pix);
        return id;
    }

    b32 load(char *fileName, i32 w, i32 h)
    {
        if (loadTexture(fileName)) {
            frameWidth = w;
            frameHeight = h;
            return true;
        }
        return false;
    }

    b32 loadTexturePixels(u32 *pix, u32 w, u32 h)
    {
        freeTexture();

        width = w;
        height = h;

        // generate 1 texture
        glGenTextures(1, &id);
        // bind the texture id to a texture target
        glBindTexture(GL_TEXTURE_2D, id);
        // set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // load texture from memory buffer
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);
        // unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);

        u32 error = glGetError();
        if (error != GL_NO_ERROR) {
            quitError("Failed to load pixels from texture: %s\n", gluErrorString(error));
            return false;
        }

        return true;
    }

    // lock texture for pixel manipulation
    b32 lock()
    {
        if (pixels == 0 && id != 0) {
            // allocate memory for texture pixels
            u32 size = width * height;
            pixels = (u32 *)malloc(sizeof(u32)*size);

            glBindTexture(GL_TEXTURE_2D, id);
          
            // get pixels
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
           
            glBindTexture(GL_TEXTURE_2D, 0);
            return true;
        }

        return false;
    }

    b32 unlock()
    {
        if (pixels != 0 && id != 0) {
            glBindTexture(GL_TEXTURE_2D, id);

            // update texture
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

            // delete pixels
            free(pixels);
            pixels = 0;

            glBindTexture(GL_TEXTURE_2D, 0);
            return true;
        }

        return false;
    }

    u32 getPixel32(u32 x, u32 y)
    {
        return pixels[y * width + x];
    }

    void setPixel32(u32 x, u32 y, u32 pixel)
    {
        pixels[y * width + x] = pixel;
    }

    // TODO: Fix all the drawing functions
    void draw()
    {
        // check if wireframe rendering is turned on
        if (platformState.fillFlag == false)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // enable texturing
        glEnable(GL_TEXTURE_2D);

        if (id != 0) {
            glBindTexture(GL_TEXTURE_2D, id);

            // place texture on quad
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
            glEnd();
        }

        glDisable(GL_TEXTURE_2D);

        // turn on noFill again
        if (platformState.fillFlag == false)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void draw(f32 x, f32 y, f32 w = 0, f32 h = 0)
    {
        if (id != 0) {
            if (w == 0)
                w = (f32)width;

            if (h == 0)
                h = (f32)height;

            // check if wireframe rendering is turned on
            if (platformState.fillFlag == false)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // enable texturing
            glEnable(GL_TEXTURE_2D);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            // reset identity matrix to remove previous transformations
            glLoadIdentity();
            glTranslatef(x, y, 0.f);
            glBindTexture(GL_TEXTURE_2D, id);

            // place texture on quad
            glBegin(GL_QUADS);
            glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 0.f);
            glTexCoord2f(1.f, 0.f); glVertex2f(w, 0.f);
            glTexCoord2f(1.f, 1.f); glVertex2f(w, h);
            glTexCoord2f(0.f, 1.f); glVertex2f(0.f, h);
            glEnd();

            glDisable(GL_TEXTURE_2D);

            // change back the stroke color to the current selected
            glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);

            // turn on noFill again
            if (platformState.fillFlag == false)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
    }

    // Draw a part of a texture (defined by a rectangle)
    // NOTE: origin is relative to destination rectangle size
    void drawEx(Rect sourceRec, Rect destRec, v2 origin, f32 rotation, Color tint)
    {
        if (id != 0) {
            if (sourceRec.w < 0) sourceRec.x -= sourceRec.w;
            if (sourceRec.h < 0) sourceRec.y -= sourceRec.h;

            // check if wireframe rendering is turned on
            if (platformState.fillFlag == false)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // enable texturing
            glEnable(GL_TEXTURE_2D);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            glBindTexture(GL_TEXTURE_2D, id);

            glPushMatrix();
            glTranslatef((f32)destRec.x, (f32)destRec.y, 0);
            glRotatef(rotation, 0, 0, 1);
            glTranslatef(-origin.x, -origin.y, 0);

            glBegin(GL_QUADS);
            glColor4ub((u8)tint.r, (u8)tint.g, (u8)tint.b, (u8)tint.a);

            // normal vector pointing towards viewer
            glNormal3f(0.0f, 0.0f, 1.0f);

            // bottom-left
            glTexCoord2f((f32)sourceRec.x / width, (f32)sourceRec.y / height);
            glVertex2f(0.0f, 0.0f);

            // bottom-right
            glTexCoord2f((f32)sourceRec.x / width, (f32)(sourceRec.y + sourceRec.h) / height);
            glVertex2f(0.0f, (f32)destRec.h);

            // top-right 
            glTexCoord2f((f32)(sourceRec.x + sourceRec.w) / width, (f32)(sourceRec.y + sourceRec.h) / height);
            glVertex2f((f32)destRec.w, (f32)destRec.h);

            // top-left 
            glTexCoord2f((f32)(sourceRec.x + sourceRec.h) / width, (f32)sourceRec.y / height);
            glVertex2f((f32)destRec.w, 0.0f);
            glEnd();
            glPopMatrix();

            glDisable(GL_TEXTURE_2D);

            // change back the stroke color to the current selected
            glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);

            // turn on noFill again
            if (platformState.fillFlag == false)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
    }

    void draw(i32 posX, i32 posY, Color tint)
    {
        draw(v2{ (f32)posX, (f32)posY }, 0.0f, 1.0f, tint);
    }

    void drawTextureV(Sprite texture, v2 position, Color tint)
    {
        draw(position, 0, 1.0f, tint);
    }

    void draw(v2 position, f32 rotation, f32 scale, Color tint)
    {
        Rect sourceRec = { 0.f, 0.f, (f32)width, (f32)height };
        Rect destRec = { position.x, position.y, width * scale, height * scale };
        v2 origin = { 0, 0 };

        drawEx(sourceRec, destRec, origin, rotation, tint);
    }

    // draw a part of a texture (defined by a rectangle)
    void drawRect(Rect sourceRec, v2 position, Color tint)
    {
        Rect destRec = { position.x, position.y, absoluteValue(sourceRec.w), absoluteValue(sourceRec.h) };
        v2 origin = { 0, 0 };

        drawEx(sourceRec, destRec, origin, 0.0f, tint);
    }

    void draw(f32 x, f32 y, Rect *clip)
    {
#if 0
        Rectf sourceRec = { clip->x, clip->y, clip->w, clip->h };
        Rectf destRec = { (int)x, (int)y, clip->w, clip->h };
        v2 origin = { 0, 0 };
        drawEx(sourceRec, destRec, origin, 0.0f, Color{ 0xff, 0xff, 0xff, 0xff });
#endif

        if (id != 0) {
            glEnable(GL_TEXTURE_2D);
            glLoadIdentity();

            // texture coordinates
            f32 texTop = 0.f;
            f32 texBottom = 1.f;
            f32 texLeft = 0.f;
            f32 texRight = 1.f;

            // vertex coordinates
            f32 quadWidth = (f32)width;
            f32 quadHeight = (f32)height;

            // handle clipping
            if (clip != NULL) {
                // turn pixel coordinates into texture mapping coordinates, take the position and divide by the pixel width(or height for the vertical position)
                texLeft = clip->x / width;
                texRight = (clip->x + clip->w) / width;
                texTop = clip->y / height;
                texBottom = (clip->y + clip->h) / height;

                quadWidth = clip->w;
                quadHeight = clip->h;
            }

            glTranslatef(x, y, 0.f);

            glBindTexture(GL_TEXTURE_2D, id);

            // render textured quad
            glBegin(GL_QUADS);
            glTexCoord2f(texLeft, texTop); glVertex2f(0.f, 0.f);
            glTexCoord2f(texRight, texTop); glVertex2f(quadWidth, 0.f);
            glTexCoord2f(texRight, texBottom); glVertex2f(quadWidth, quadHeight);
            glTexCoord2f(texLeft, texBottom); glVertex2f(0.f, quadHeight);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
    }
};


// 3D Models

// needed for the normal vectors
struct Face
{
    i32 facenum;	// id for the faces from the obj file
    b32 quad;		// is it a quad?
    i32 faces[4];

    // triangle
    Face(i32 facen, i32 f1, i32 f2, i32 f3) : facenum(facen)
    {
        faces[0] = f1;
        faces[1] = f2;
        faces[2] = f3;
        quad = false;
    }
    // quad
    Face(i32 facen, i32 f1, i32 f2, i32 f3, i32 f4) : facenum(facen)
    {
        faces[0] = f1;
        faces[1] = f2;
        faces[2] = f3;
        faces[3] = f4;
        quad = true;
    }
};

char *moveToNextLine(char *cont)
{
    while (*cont != '\n')
        cont++;
    // move past \n
    cont++;
    return cont;
}

// load blender obj file
int loadModel(char *filename)
{
    v3 *vertex = 0;
    Face *faces = 0;
    v3 *normals = 0;

    char *contents = (char *)loadTextFile(filename);
    char *contentsAddress = contents;

    while (*contents != '\0') {
        if (*contents == 'v' && *(1 + contents) == ' ') {
            f32 tmpx, tmpy, tmpz;
            sscanf(contents, "v %f %f %f", &tmpx, &tmpy, &tmpz);
            pushArray(vertex, v3(tmpx, tmpy, tmpz));
        }
        else if (*contents == 'v' && *(1 + contents) == 'n') {
            // parse the normals
            f32 tmpx, tmpy, tmpz;
            sscanf(contents, "vn %f %f %f", &tmpx, &tmpy, &tmpz);
            pushArray(normals, v3(tmpx, tmpy, tmpz));
        }
        else if (*contents == 'f') {
            // parse the faces, which points that makes a face
            i32 id;
            i32 a, b, c, d;
            i32 spaceCount = 0;

            // check if we have quads or triangles
            char *temp = contents;
            while (*temp != '\n') {
                if (*temp == ' ')
                    spaceCount++;
                temp++;
            }

            // check for quads
            if (spaceCount == 4) {
                sscanf(contents, "f %d//%d %d//%d %d//%d %d//%d", &a, &id, &b, &id, &c, &id, &d, &id);
                pushArray(faces, Face(id, a, b, c, d));
            }
            else {
                // triangles
                sscanf(contents, "f %d//%d %d//%d %d//%d", &a, &id, &b, &id, &c, &id);
                pushArray(faces, Face(id, a, b, c));
            }
        }
        contents = moveToNextLine(contents);
    }

    // generate a display list
    i32 num = glGenLists(1);

    glNewList(num, GL_COMPILE);
    for (i32 i = 0; i < countArray(faces); i++) {
        //quads?
        if (faces[i].quad) {
            glBegin(GL_QUADS);
            glNormal3f(normals[faces[i].facenum - 1].x, normals[faces[i].facenum - 1].y, normals[faces[i].facenum - 1].z); //-1 because C index from 0
            glVertex3f(vertex[faces[i].faces[0] - 1].x, vertex[faces[i].faces[0] - 1].y, vertex[faces[i].faces[0] - 1].z);
            glVertex3f(vertex[faces[i].faces[1] - 1].x, vertex[faces[i].faces[1] - 1].y, vertex[faces[i].faces[1] - 1].z);
            glVertex3f(vertex[faces[i].faces[2] - 1].x, vertex[faces[i].faces[2] - 1].y, vertex[faces[i].faces[2] - 1].z);
            glVertex3f(vertex[faces[i].faces[3] - 1].x, vertex[faces[i].faces[3] - 1].y, vertex[faces[i].faces[3] - 1].z);
            glEnd();
        }
        else {
            glBegin(GL_TRIANGLES);
            glNormal3f(normals[faces[i].facenum - 1].x, normals[faces[i].facenum - 1].y, normals[faces[i].facenum - 1].z); //-1 because C index from 0
            glVertex3f(vertex[faces[i].faces[0] - 1].x, vertex[faces[i].faces[0] - 1].y, vertex[faces[i].faces[0] - 1].z);
            glVertex3f(vertex[faces[i].faces[1] - 1].x, vertex[faces[i].faces[1] - 1].y, vertex[faces[i].faces[1] - 1].z);
            glVertex3f(vertex[faces[i].faces[2] - 1].x, vertex[faces[i].faces[2] - 1].y, vertex[faces[i].faces[2] - 1].z);
            glEnd();
        }
    }
    glEndList();

    // cleanup
    freeArray(faces);
    freeArray(normals);
    freeArray(vertex);

    contents = contentsAddress;
    free(contents);
    return num;
}

void model(i32 object)
{
    glCallList(object);
}

//
// Input API
//
b32 keyDown(u32 key)
{
    if (key < 0 || key > KEY_COUNT)
        return false;

    b32 result = input.keys[key];
    return result;
}

b32 keyHit(u32 key)
{
    if (key < 0 || key > KEY_COUNT)
        return false;

    b32 result = input.keys[key] && !input.prevKeys[key];
    return result;
}

b32 keyUp(u32 key)
{
    if (key < 0 || key > KEY_COUNT)
        return false;

    b32 result = input.prevKeys[key] && !input.keys[key];
    return result;
}

b32 mouseClicked(u32 b = MOUSE_LEFT)
{
    b32 result = input.mouseButtons[b].isDown && input.mouseButtons[b].changed;
    return result;
}

b32 mouseReleased(u32 b = MOUSE_LEFT)
{
    b32 result = !input.mouseButtons[b].isDown && input.mouseButtons[b].changed;
    return result;
}

b32 mousePressed(u32 b = MOUSE_LEFT)
{
    b32 result = input.mouseButtons[b].isDown;
    return result;
}

b32 mouseMoved()
{
    b32 result = false;
    if (input.mouseMoved)
        result = true;
    return result;
}

// check if current mouse position is within a rectangle
i32 isMouseInRect(i32 x, i32 y, i32 w, i32 h)
{
    if (input.mouseX < x || input.mouseY < y || input.mouseX >= x + w || input.mouseY >= y + h)
        return 0;

    return 1;
}

b32 gamepadButtonClicked(u32 b)
{
    b32 result = input.gamepadButtons[b].isDown && input.gamepadButtons[b].changed;
    return result;
}

b32 gamepadButtonReleased(u32 b)
{
    b32 result = !input.gamepadButtons[b].isDown && input.gamepadButtons[b].changed;
    return result;
}

b32 gamepadButtonPressed(u32 b)
{
    b32 result = input.gamepadButtons[b].isDown;
    return result;
}

void gamepadRumbleOn(i32 controller, i16 leftMotorSpeed, i16 rightMotorSpeed)
{
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = leftMotorSpeed;
    vibration.wRightMotorSpeed = rightMotorSpeed;
    XInputSetState(controller, &vibration);
}

void gamepadRumbleOff(i32 controller)
{
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = 0;
    vibration.wRightMotorSpeed = 0;
    XInputSetState(controller, &vibration);
}

//
// Timing
//

// returns the number of milliseconds since program start
u32 millis()
{
    return (u32)platformState.milliseconds;
}

// retrieves the number of milliseconds that have elapsed since the system was started,
u32 getTicks()
{
    u32 result = GetTickCount();
    return result;
}

/*
------------------------------------------------------------------------------
This software is available under 2 licenses - you may choose the one you like.
------------------------------------------------------------------------------

ALTERNATIVE A - zlib license
Copyright (c) 2020 Martin Fairbanks
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/