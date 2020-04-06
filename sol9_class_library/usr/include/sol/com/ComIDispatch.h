/******************************************************************************
 *
 * Copyright (c) 2009 Antillia.com TOSHIYUKI ARAI. ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer.
 *  
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  ComIDispatch.h
 *
 *****************************************************************************/

//2009/12/04

#pragma once

#include <sol/com/ComIUnknown.h>
#include <comutil.h>
#include <objbase.h>

#include <sol/COMTypeConverter.h>
//2009/12/22
#include <sol/Writer.h>
#include <sol/FileWriter.h>
#include <sol/ConsoleWriter.h>

//2009/12/22
#include <sol/LocalDateTime.h>
#include <sol/HTMLEncoder.h>

#pragma comment(lib, "oleaut32.lib")


namespace SOL {

class ComIDispatch :public ComIUnknown {

public:
  ComIDispatch(IDispatch* pDisp=NULL)
  :ComIUnknown(pDisp)
  {

  }

public:
  ~ComIDispatch() 
  {
  }


public:
  virtual void display()
  {
    Writer console;
    write(console);
  }

public:
  virtual void write(Writer& writerr)
  {
    printf("ComIDispatch::display\n");
  }

public:
  //If needed, please redefine this method in your own sublcass;
  virtual HRESULT createInstance()
  {
    HRESULT hr = E_FAIL;
    return hr;
  }

public:
  HRESULT createInstance(__in REFCLSID rclsId,  __in REFIID riid, 
      __in_opt DWORD dwClsContext=CLSCTX_INPROC_SERVER,   
      __in_opt LPUNKNOWN pUnkOuter=NULL)
  {
    IDispatch* pDisp =NULL;

    HRESULT hr = E_FAIL;
    if (SUCCEEDED(hr = CoCreateInstance(
      rclsId,     
      pUnkOuter,      
      dwClsContext, 
      riid, 
      (void**)&pDisp))) {
      //OK. 
      set(pDisp);
    } else {
      //If failed, throw hr;
      printf("Failed to createInstance\n");
      throw Exception(E_FAIL, "%s: %s", "E_FAIL",__FUNCTION__);
    }
    return hr;
  }

public:
  IDispatch* getIDispatch()
  {
    return (IDispatch*)getIUnknown();
  }

public:
  operator IDispatch*() 
  {
    return getIDispatch();
  }


public:
  HRESULT getProperty(__in OLECHAR* ptName, __out VARIANT* pvResult) 
  {
    if (ptName == NULL || pvResult == NULL) {
      throw E_POINTER;
    }

    IDispatch* pDisp = getIDispatch();

    HRESULT hr = E_FAIL;

    DISPID dispID = 0;


      if (FAILED(hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, 
                                &dispID)) ) { 
      throw Exception(E_FAIL, "%s: %s", "E_FAIL",__FUNCTION__);
    }

    DISPPARAMS dispParams;
    memset(&dispParams, 0, sizeof(dispParams));

    return pDisp->Invoke(dispID, IID_NULL,
          LOCALE_USER_DEFAULT,
          DISPATCH_PROPERTYGET,
          &dispParams, 
          pvResult,
          NULL, NULL);
  }

public:
  _variant_t getProperty(__in _bstr_t name)
  {  
    OLECHAR* ptName = (OLECHAR*)name;
    VARIANT var;
    VariantInit(&var);
  
    if (ptName == NULL) {
      throw E_POINTER;
    }

    IDispatch* pDisp = getIDispatch();

    HRESULT hr = E_FAIL;

    DISPID dispID = 0;

      if (FAILED(hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, 
                                &dispID)) ) { 
      throw Exception(E_FAIL, "%s: %s", "E_FAIL",__FUNCTION__);
    }

    DISPPARAMS dispParams;
    memset(&dispParams, 0, sizeof(dispParams));

    if (FAILED(hr = pDisp->Invoke(dispID, IID_NULL,
          LOCALE_USER_DEFAULT,
          DISPATCH_PROPERTYGET,
          &dispParams, 
          &var,
          NULL, NULL) ) ) {
      throw Exception(E_FAIL, "%s: %s", "E_FAIL",__FUNCTION__);
    }

    return _variant_t(var, false);
  }

public:
  HRESULT putProperty(__in OLECHAR* ptName, __in const VARIANT* pvValue)
  {
    if (ptName == NULL || pvValue == NULL) {
      throw E_POINTER;
    }

    IDispatch* pDisp = getIDispatch();

    DISPID dispID = 0;
    
    HRESULT hr = E_FAIL;

      if (FAILED(hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, 
        &dispID)) ) {
      throw Exception(E_FAIL, "%s: %s", "E_FAIL",__FUNCTION__);
    }

    VARIANTARG vArgs[1];
    VariantInit(&vArgs[0]);
    VariantCopy(&vArgs[0], pvValue);

    DISPID dispIDNamedArgs[1] = {DISPID_PROPERTYPUT};

    DISPPARAMS dispParams;
    memset(&dispParams, 0, sizeof(dispParams));

    dispParams.rgvarg = vArgs;
    dispParams.rgdispidNamedArgs = dispIDNamedArgs;
    dispParams.cArgs = 1;
    dispParams.cNamedArgs = 1;

    hr = pDisp->Invoke(dispID, IID_NULL,
          LOCALE_USER_DEFAULT,
          DISPATCH_PROPERTYPUT,
          &dispParams, NULL,
          NULL, NULL);
    VariantClear(&vArgs[0]);

    return hr;
  }

public:
  HRESULT invoke(__in OLECHAR* method, __out VARIANT* pvResult, __in int nArgs, ...)
  {
    if (method == NULL || pvResult == NULL) {
      throw E_POINTER;
    }

    HRESULT hr = E_FAIL;

    IDispatch* pDisp = getIDispatch();
    DISPID dispID = 0;

    if (FAILED(hr = pDisp->GetIDsOfNames(IID_NULL, &method, 1, LOCALE_USER_DEFAULT, 
        &dispID)) ) {
      throw Exception(E_FAIL, "%s: %s", "E_FAIL",__FUNCTION__);
    }

    VARIANTARG *pvArgs = NULL;
    va_list ap;
    
    if(nArgs > 0){        
      pvArgs = new VARIANTARG[nArgs];
      va_start(ap, nArgs);
      for(int i=0;i<nArgs;i++){
        VariantInit(&pvArgs[i]);
        VariantCopy(&pvArgs[i], &va_arg(ap, VARIANT));
      }
      va_end(ap);
    }
    
    DISPPARAMS dispParams;
    memset(&dispParams, 0, sizeof(dispParams));

    dispParams.rgvarg =  pvArgs;
    dispParams.rgdispidNamedArgs = NULL;
    dispParams.cArgs = nArgs;
    dispParams.cNamedArgs = 0;

    hr = pDisp->Invoke(
        dispID, IID_NULL,
        LOCALE_USER_DEFAULT,
        DISPATCH_METHOD,
        &dispParams, pvResult,
        NULL, NULL);

    for(int i=0; i<nArgs; i++){
      VariantClear(&pvArgs[i]);
    }

    if(pvArgs){
      delete[] pvArgs;
    }
    return hr;
  }

};


}
