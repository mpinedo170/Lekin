#pragma once

/////////////////////////////////////////////////////////////////////////////
// Global macro definitions

// private and undefined copy constructor and assignment for any class
#define DEFINE_COPY_AND_ASSIGN(T) \
    T(const T&) = delete;         \
    T& operator=(const T&) = delete;

// definitions for TStringBuffer
namespace AFLibIO {
class TStringBufferA;
class TStringBufferW;
}  // namespace AFLibIO

#ifdef _UNICODE
#define TStringBuffer TStringBufferW
#else
#define TStringBuffer TStringBufferA
#endif
