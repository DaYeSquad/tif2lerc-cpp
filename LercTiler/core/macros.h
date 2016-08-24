// macros.h
//
// Copyright (c) 2016 Frank Lin (lin.xiaoe.f@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef LERC_CORE_MACROS_H_
#define LERC_CORE_MACROS_H_

#include <assert.h>

// platform enum
#define SKR_PLATFORM_UNKNOWN            0
#define SKR_PLATFORM_IOS                1
#define SKR_PLATFORM_ANDROID            2
#define SKR_PLATFORM_MAC                3
#define SKR_PLATFORM_WIN32              5

// DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&); \
void operator=(const TypeName&)

// namespace Gago {}
#ifdef __cplusplus
#define NS_GAGO_BEGIN                     namespace gago {
#define NS_GAGO_END                       }
#define USING_NS_GAGO                     using namespace gago
#else
#define NS_GAGO_BEGIN
#define NS_GAGO_END
#define USING_NS_GAGO
#endif

// namespace sakura {}
#ifdef __cplusplus
#define NS_SKR_BEGIN                  namespace sakura {
#define NS_SKR_END                    }
#define USING_NS_SKR                  using namespace sakura
#else
#define NS_SKR_BEGIN
#define NS_SKR_END
#define USING_NS_SKR
#endif

// SKR_DLL
#if SKR_PLATFORM == SKR_PLATFORM_WIN32
#if defined(SKR_STATIC)
#define SKR_DLL
#else
#if defined(_USRDLL)
#define SKR_DLL     __declspec(dllexport)
#else
#define SKR_DLL     __declspec(dllimport)
#endif
#endif
#else
#define SKR_DLL
#endif // SKR_PLATFORM == SKR_PLATFORM_WIN32

// assert
#ifdef SKR_USING_ASSERT
#include <assert.h>
#define SKR_ASSERT(p) assert(p)
#else
#define SKR_ASSERT(p)
#endif

// delete/free
#define SKR_SAFE_DELETE(p) do { if(p) { delete (p); (p) = nullptr; } } while(0)
#define SKR_SAFE_DELETE_ARRAY(p) do { if(p) { delete[] (p); (p) = nullptr; } } while(0)
#define SKR_SAFE_FREE(p) do { if(p) { free(p); (p) = 0; } } while(0)

#define SKR_SAFE_RELEASE(p) do { if(p) { (p)->Release(); (p)=nullptr; } } while(0)

// Break if
#define SKR_BREAK_IF(XXXX) if(XXXX)break;

#endif /* LERC_CORE_MACROS_H_ */
