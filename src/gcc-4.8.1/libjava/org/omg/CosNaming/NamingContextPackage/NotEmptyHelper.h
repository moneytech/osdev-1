
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_CosNaming_NamingContextPackage_NotEmptyHelper__
#define __org_omg_CosNaming_NamingContextPackage_NotEmptyHelper__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class Any;
          class TypeCode;
        namespace portable
        {
            class InputStream;
            class OutputStream;
        }
      }
      namespace CosNaming
      {
        namespace NamingContextPackage
        {
            class NotEmpty;
            class NotEmptyHelper;
        }
      }
    }
  }
}

class org::omg::CosNaming::NamingContextPackage::NotEmptyHelper : public ::java::lang::Object
{

public:
  NotEmptyHelper();
  static ::org::omg::CosNaming::NamingContextPackage::NotEmpty * extract(::org::omg::CORBA::Any *);
  static ::java::lang::String * id();
  static void insert(::org::omg::CORBA::Any *, ::org::omg::CosNaming::NamingContextPackage::NotEmpty *);
  static ::org::omg::CosNaming::NamingContextPackage::NotEmpty * read(::org::omg::CORBA::portable::InputStream *);
  static ::org::omg::CORBA::TypeCode * type();
  static void write(::org::omg::CORBA::portable::OutputStream *, ::org::omg::CosNaming::NamingContextPackage::NotEmpty *);
private:
  static ::java::lang::String * _id;
public:
  static ::java::lang::Class class$;
};

#endif // __org_omg_CosNaming_NamingContextPackage_NotEmptyHelper__
