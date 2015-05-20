//////////////////////////////////////////////////////////////////////
// Win32Error.h:    
//      Interface and Implementation for the 
//      CWin32Error class.
//
//      Written by and copyright (c) 2001 Ajit Jadhav.
//      All rights reserved by the author.
//
// Legal Notice:
// * The material in this source file can be used for any 
// purpose, commercial or noncommercial, without paying any 
// charge.
// * However, use this material, in full or in part, at your
// own risk. The author assumes no responsibility--implied 
// or otherwise--arising from any direct or indirect damages 
// which may result by using the material presented here. 
// * If you decide to use or reuse this material, please make
// sure to keep the this copyright notice intact in your 
// source-files.
// * However, it is NOT necessary to acknowledge the copyright 
// outside of your project source files. (For example, it is 
// NOT necessary to pop up a message box acknowledging this 
// copyright in your application.)
//////////////////////////////////////////////////////////////////////
#ifndef WIN32ERROR_H__
#define WIN32ERROR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
//  About class CWin32Error
//  =======================
//      - Is an incredibly small but enourmously useful C++ thingie.
//      - Encapsulates the two basic Win32 APIs:
//          - ::GetLastError()  and
//          - ::FormatMessage().
//      - Employs smart copy methods to minimize the overheads of 
//        mem allocation, retrieving system error strings, and copying.
//      - Has all member-functions inline. Just #include this header.
//      - Does not depend on MFC. Use freely with any C++ library.
//      - Compiles under both UNICODE and MBCS.
//      - Is very convenient and simple to use--in fact, the objects
//        of this class are *best* created on the stack, and can 
//        be passed by value without undue performance hit. 
//      - Provides automatic conversion to const TCHAR* type. Pass 
//        objects, as they are, to MessageBox() or AfxMessageBox().
//      - Supports the MFC TRACE macro. Pass objects to TRACE as they 
//        are--you don't have to call object.c_str() method or so.
//        e.g. TRACE( "%s\n", e ); and not TRACE( "%s\n", e.c_str() );
//      - Helps in taking advantage of the exception-handling features
//        of the C++ language.
//
//  Intended usages:
//  ===============
//      - As an exception class whose instances can be thrown and 
//        and caught by value without undue performance hit. 
//      - As a convenient encapsulation of strings with error codes.
//      - As a debugger convenience for error messages.
//
//  Example usage, technical notes, and suggestions for improvements 
//        appear towards the end of this file.
//
//  Well thought out comments are welcome. 
//
//  Thanks for using, and happy debugging ;)
//
//      -- Ajit Jadhav (ajit_jadhav@hotmail.com)
//
////////////////////////////////////////////////////////////////////////
#include <TCHAR.H>
#include <Windows.h>

class CWin32Error  
{
public:
//----- Construction and destruction -----------------------------------
    
    // The default constructor calls ::GetLastError() as well 
    // as ::FormatMessage(). BUT the copy constructor and the 
    // overloaded assignment ('=') operator DO NOT. 
    // Thus, object-copies carry the original error code and message.
    // This avoids the possible confusion due to intermittant
    // SetLastError() called by some other Win32 API functions.
    CWin32Error() 
        : m_szErrMsg( NULL )
    {
        unsigned int dwLastErr = ::GetLastError();
        doFormatMessage( dwLastErr );
    }

    // Smart copy
    CWin32Error( const CWin32Error& rRHS )
        : m_szErrMsg( NULL )
    {
        addRef( rRHS );
    }
    
    // Use this form of construction if you yourself
    // call ::GetLastError() just before instantiation.
    // Also useful for returning success status (arg = 0 ).
    CWin32Error( unsigned int dwLastError )
        : m_szErrMsg( NULL )
    {
        doFormatMessage( dwLastError );
    }

    // Smart copy.
    const CWin32Error& operator=( const CWin32Error& rRHS )
    {
        if( this != &rRHS )
        {
            releaseRef();
            addRef( rRHS );
        }
        return *this;
    }

    // Automatically frees the internal buffer once 
    // the internal reference count drops to zero.
    // TODO: Make dtor virtual, if deriving from this.
    ~CWin32Error() 
    {
        releaseRef();
    }

//----- Returning the state without modification -----------------------

    // The Win32 error code currently held in this object.
    unsigned int ErrorCode() const
    {
        return metaMem() ? metaMem()->m_dwErrCode : -1;
    }

    // The Win32 error code currently held in this object.
    operator unsigned int() const
    {
        return metaMem() ? metaMem()->m_dwErrCode : -1;
    }

    // TCHAR count including the terminating NULL TCHAR. That is not
    // necessarily equal to byte count. (Depends on UNICODE/MBCS).
    int MessageLength() const
    {
        return metaMem() ? metaMem()->m_nMsgStrLen : 0;
    }

    // Internal buffer returned!! Do not modify, delete[] or free.
    // You can directly pass this object to any function that 
    // accepts a const char* argument, e.g. to AfxMessageBox().
    // Makes available the Win32 API error string held internally.
    operator const TCHAR* () const
    {
        return m_szErrMsg;
    }

    // The function form for returning the error string.
    // Internal buffer returned!! Do not modify, delete[] or free.
    const TCHAR* Description() const
    {
        return m_szErrMsg;
    }

//----- Resetting the internal state -----------------------------------

    // Note: GetLastError() is being called for the n+1 time
    // now. (The first time was in construction.) The two values
    // may differ if ::SetLastError() was called in between.
    // This can happen if either you call SetLastError yourself
    // of if you call some Win32 API (such as CreateFile() )
    // that on its own calls SetLastError()
    void ReGetLastError()
    {
        ///ASSERT( metaMem() );

        // Get the error code of the current thread
        unsigned int dwNewErr = ::GetLastError();

        // A simple check for performance: Reformatting is only 
        // necessary if the new code value is different.
        if( dwNewErr != metaMem()->m_dwErrCode )
        {
            releaseRef();
            doFormatMessage( dwNewErr );
        }
    }
    

//----- Private members ----------------------------------------------
private:

    // struct for smart copying. Meta info is prepended to 
    // the actual string, something like in BASIC strings (BSTRs).
    struct SMetaMem
    {
        SMetaMem() 
            : m_nMsgStrLen(0), m_dwErrCode(0), m_nRefCnt(0) {}
    
        long m_nRefCnt;             // Including this
        unsigned int m_dwErrCode;   // Win32 error code
        int m_nMsgStrLen;           // i.e. _tcslen() + 1.
        
        TCHAR* getString() { return (TCHAR*) (this+1); }
    };

    void addRef( const CWin32Error& rOrig )
    {
        ::InterlockedIncrement( &rOrig.metaMem()->m_nRefCnt );
        m_szErrMsg = rOrig.m_szErrMsg;
    }
    
    void releaseRef()
    {
        if( ! metaMem() )
        {
            return;
        }
        if( ! ::InterlockedDecrement( &metaMem()->m_nRefCnt ) )
        {
            freeBuffer();
        }
    }
    
    void doFormatMessage( unsigned int dwLastErr )
    {
        m_szErrMsg = NULL;  
            
        TCHAR* pTemp = NULL;
        int nLen = ::FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                        FORMAT_MESSAGE_IGNORE_INSERTS |
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL, 
                        dwLastErr,
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPTSTR)&pTemp, 
                        1, 
                        NULL );
        allocCopyData( pTemp, nLen, dwLastErr);
        ::LocalFree( pTemp );
        pTemp = NULL;
    }

    void allocCopyData( TCHAR* pTemp, int nLen, unsigned int dwLastErr )
    {
        SMetaMem* pSM = NULL;
        pSM = (SMetaMem*) new unsigned char [ sizeof(SMetaMem) + (nLen+1)*sizeof(TCHAR) ];
        if( ! pSM )
        {
            return;
        }
        pSM->m_dwErrCode = dwLastErr;
        pSM->m_nRefCnt = 1;
        pSM->m_nMsgStrLen = nLen;
        m_szErrMsg = pSM->getString();
        ::memcpy( m_szErrMsg, pTemp, nLen*sizeof(TCHAR) );
        m_szErrMsg[ nLen ] = NULL;
    }

    void freeBuffer()
    {
        if( metaMem() )
        {
            delete [] (unsigned char*) metaMem();
            m_szErrMsg = NULL; 
        }
    }

    SMetaMem* metaMem() const
    {
        return m_szErrMsg ? (SMetaMem*)m_szErrMsg - 1 : NULL;
    }
    // The error message given by ::FormatMessage()
    TCHAR* m_szErrMsg;  
};
//======================================================================
// class CWin32Error Interface and Implementation ends here.
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//
//  CWin32Error Example Usages:
//  ========================== 
// 
//  Example 1: Simplified Exception Handling For Win32 API Errors
//             This was my main purpose in writing this class.
//             --------------------------------------------------
//
//  void MyFunction( void ) throw CWin32Error // see client below
//  {
//      if( ! SomeWin32API() ) 
//      {
//          // ::GetLastError() and ::FormatMessage() 
//          // automatically get called during construction.
//
//          // Catch by value or by ref--the code stays the same.
//
//          // Smart copying means you can rethrow the object by 
//          // value as many times as you like!
//
//          throw CWin32Error(); 
//      }
//  }
//  
//  void ClientToMyFunction( void )
//  {
//      try
//      {
//          MyFunction();
//      }
//      catch( CWin32Error e ) // catching by value is OK (smart copying)
//      {
//          // Note: Automatic conversion to const TCHAR* type.
//
//          ::OutputDebugTrace( e ); 
//      }
//  }
//  
// 
//  Example 2: "Vanilla" usage (No Exception Handling)
//             ---------------------------------------
//
//  // This function returns a CWin32Error object. Smart copy
//  // means you can return objects even during normal 
//  // (non-exception-related) execution, without having to
//  // take undue performance hit.
//
//  CWin32Error MyFunction( void ) 
//  {
//      if( ! SomeWin32API() )
//      {
//          // Constructor gets last error and keeps the text 
//          // of the error message ready.
//
//          CWin32Error e; 
//
//          // Don't have to call a member function even in 
//          // MFC trace macro [i.e. no e.c_str() or (const char*) e]
//
//          TRACE( "%s\n", e );
//
//          return e;
//      }
//      // In Win32 API, the '0' value corresponds to the 
//      // error string: "Operation completed successfully"
//      // The error string is automatically taken from the OS 
//      // in the returned CWin32Error object.
//
//      return 0; // ctor taking unsigned int called here
//  }
//
//
//  Example 3:  Simplest: Neither exception-handling nor returning 
//              errors. Just a help for getting the message-strings
//              formatted from the OS.
//              --------------------------------------------------
//
//  void MyFunction( void ) 
//  {
//      if( ! SomeWin32API() )
//      {
//          // If you want to retrieve the error code yourself...
//
//          DWORD dwErr = ::GetLastError();
//
//          // ...perhaps for some check on the code like this...
//
//          if( dwErr is really bad and 
//              user should know about it )
//          {
//              // This form of CWin32Error ctor does NOT call 
//              // ::GetLastError().
//
//              CWin32Error e = dwErr; 
//              
//              // CWin32Error supplies const char* conversion
//
//              AfxMessageBox( e );
//          }
//          // Else, forget prompting the user. 
//          // Just return from the function...
//
//          return;
//      }
//      // other code ...
//  }
//
////////////////////////////////////////////////////////////////////////
//
// Technical Notes for CWin32Error:
// ===============================
// 
// On-Stack- Vs. On-Heap-Instantiation:
// -----------------------------------
// This class was designed so that error objects can be freely created
// on the stack and returned by value across exception frames/blocks.
//
// Creating objects of this class dynamically (i.e on the heap)
// offers no particular advantage. Usually, people tend to 
// reuse such heap-allocated objects via pointers. Heap is not harmful 
// by itself, but there are object reuse issues, as given below.
//
// Reusing the same instance (object) at runtime.
// ---------------------------------------------
// (i) You have to remember to call CWin32Error::ReGetLastError() 
// (ii) You have to provide thread-protection in multi-threaded
// apps--the single global object may grab error code from place in 
// another thread. 
//
// Of course, if you do not *reuse* the same instance, then it's 
// perfectly OK to create CWin32Error objects on the heap.
//
// Remember not to delete [] Description() return value
// ----------------------------------------------------
// - The only reason I didn't return an MFC CString or an STL string or 
// wstring object is because I didn't want to create dependencies on
// other libraries right in this small utility class. 
// - On the minus side of this decision is exposing the internal buffer 
// (even if as a const). 
// - On the plus side, you can use this class intact in almost any
// kind of Win32 development--whether in performance-critical sockets
// apps; or in ATL COM components; or in console app using STL; or
// in a database project using some third-party ODBC library, or in 
// the AppWizard generated MFC MDI App.
////////////////////////////////////////////////////////////////////////
//
//  Suggestions for improvements to the class
//  =========================================
//  - Enhancements/checks for handling Value-collision of error codes:
//      - MAPI errors vs Win32
//      - Win32 vs custom MC-dll
//  - Provide support for ::FormatMessage() arguments:
//      - Formatting messages from strings.
//      - Inserts and argument arrays.
//  - Using a custom message (MC-compiled) dll for messages. 
//      - Use its instance module handle to format messages. 
//      - Add/Alter member functions suitably.
//      Caution:
//          - Who loads the dll? Manages its in-proc-time?
//          - Who sets the thread error number? When?
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
#endif // #ifndef WIN32ERROR_H__

