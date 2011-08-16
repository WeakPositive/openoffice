/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cli_ure.hxx"

#pragma warning(push, 1)
#include "windows.h"
#pragma warning(pop)

#include <memory>


#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "uno/sequence2.h"
#include "typelib/typedescription.hxx"
#include "cli_proxy.h"
#include "cli_base.h"
#include "cli_bridge.h"

#using <cli_uretypes.dll>


#undef VOID

namespace css = com::sun::star;

namespace sri = System::Runtime::InteropServices;
namespace sr = System::Reflection;
namespace st = System::Text;
namespace ucss = unoidl::com::sun::star;

using namespace rtl;
using namespace std;


namespace cli_uno
{
System::String* mapUnoPolymorphicName(System::String* unoName);
OUString mapCliTypeName(System::String* typeName);
System::String* mapCliPolymorphicName(System::String* unoName);
System::String* mapPolymorphicName(System::String* unoName, bool bCliToUno);

inline auto_ptr< rtl_mem > seq_allocate( sal_Int32 nElements, sal_Int32 nSize )
{
    auto_ptr< rtl_mem > seq(
        rtl_mem::allocate( SAL_SEQUENCE_HEADER_SIZE + (nElements * nSize) ) );
    uno_Sequence * p = (uno_Sequence *)seq.get();
    p->nRefCount = 1;
    p->nElements = nElements;
    return seq;
}


System::Object* Bridge::map_uno2cli(uno_Interface * pUnoI, typelib_InterfaceTypeDescription *pTD) const
{
    System::Object* retVal= NULL;
// get oid
    rtl_uString * pOid = 0;
    (*m_uno_env->getObjectIdentifier)( m_uno_env, &pOid, pUnoI );
    OSL_ASSERT( 0 != pOid );
    OUString oid(pOid, SAL_NO_ACQUIRE);

	//see if the interface was already mapped
    System::Type* ifaceType= mapUnoType(reinterpret_cast<typelib_TypeDescription*>(pTD));
    System::String* sOid= mapUnoString(oid.pData);
    
	System::Threading::Monitor::Enter( CliEnvHolder::g_cli_env );
    try
    {
		retVal = CliEnvHolder::g_cli_env->getRegisteredInterface(sOid, ifaceType);
        if (retVal)
        {
            // There is already an registered object. It can either be a proxy
            // for the UNO object or a real cli object. In the first case we
            // tell the proxy that it shall also represent the current UNO
            // interface. If it already does that, then it does nothing
            if (srr::RemotingServices::IsTransparentProxy(retVal))
            {
                UnoInterfaceProxy* p = static_cast<UnoInterfaceProxy*>(
                    srr::RemotingServices::GetRealProxy(retVal));
                p->addUnoInterface(pUnoI, pTD);            
            }
        }
        else
        {
            retVal = UnoInterfaceProxy::create(
                (Bridge *) this, pUnoI, pTD, oid );
        }
    }
    __finally
    {
		System::Threading::Monitor::Exit( CliEnvHolder::g_cli_env );
    }
    
    return retVal;
}

uno_Interface* Bridge::map_cli2uno(System::Object* cliObj, typelib_TypeDescription *pTD) const
{
    uno_Interface* retIface = NULL;
    // get oid from dot net environment
	System::String* ds_oid = CliEnvHolder::g_cli_env->getObjectIdentifier( cliObj);
    OUString ousOid = mapCliString(ds_oid);  
    // look if interface is already mapped
    m_uno_env->getRegisteredInterface(m_uno_env, (void**) &retIface, ousOid.pData,
                                      (typelib_InterfaceTypeDescription*) pTD);
    if ( ! retIface)
    {
        System::Threading::Monitor::Enter(__typeof(Cli_environment));
        try
        {
            m_uno_env->getRegisteredInterface(m_uno_env, (void**) &retIface, ousOid.pData,
                                               (typelib_InterfaceTypeDescription*) pTD);
            if ( ! retIface)
            {
                retIface = CliProxy::create((Bridge*)this, cliObj, pTD, ousOid);
            }
        }
        __finally
        {
            System::Threading::Monitor::Exit(__typeof(Cli_environment));
        }
    }
    return retIface;
}

inline System::Type* loadCliType(rtl_uString * unoName)
{
     return loadCliType(mapUnoTypeName(unoName));
}

System::Type* loadCliType(System::String * unoName)
{
    System::Type* retVal= NULL;
    try
    {
        //If unoName denotes a polymorphic type, e.g com.sun.star.beans.Defaulted<System.Char>
        //then we remove the type list, otherwise the type could not be loaded.
        bool bIsPolymorphic = false;
        
        System::String * loadName = unoName;
        int index = unoName->IndexOf('<');
        if (index != -1)
        {
            loadName = unoName->Substring(0, index);
            bIsPolymorphic = true;
        }
        System::AppDomain*  currentDomain = System::AppDomain::CurrentDomain;
        sr::Assembly*  assems[] = currentDomain->GetAssemblies();
        for (int i = 0; i < assems->Length; i++)
        {
            retVal = assems[i]->GetType(loadName, false);
            if (retVal)
                break;
        }

		if (retVal == NULL)
		{
            System::String * msg = new System::String(S"A type could not be loaded: ");
            msg = System::String::Concat(msg, loadName);
			throw BridgeRuntimeError(mapCliString(msg));
		}

        if (bIsPolymorphic)
        {
            retVal = uno::PolymorphicType::GetType(retVal, unoName);
        }
    }
    catch( System::Exception * e)
    {
        rtl::OUString ouMessage(mapCliString(e->get_Message()));
        throw BridgeRuntimeError(ouMessage);
    }
    return retVal;
}


System::Type* mapUnoType(typelib_TypeDescription const * pTD)
{
    return mapUnoType(pTD->pWeakRef);
}

System::Type* mapUnoType(typelib_TypeDescriptionReference const * pTD)
{
    System::Type * retVal = 0;
    switch (pTD->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        retVal= __typeof(void); break;
    case typelib_TypeClass_CHAR:
        retVal= __typeof(System::Char); break;
    case typelib_TypeClass_BOOLEAN:
        retVal= __typeof(System::Boolean); break;
    case typelib_TypeClass_BYTE:
        retVal= __typeof(System::Byte); break;
    case typelib_TypeClass_SHORT:
        retVal= __typeof(System::Int16); break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        retVal= __typeof(System::UInt16); break;
    case typelib_TypeClass_LONG:
        retVal= __typeof(System::Int32); break;
    case typelib_TypeClass_UNSIGNED_LONG:
        retVal= __typeof(System::UInt32); break;
    case typelib_TypeClass_HYPER:
        retVal= __typeof(System::Int64); break;
    case typelib_TypeClass_UNSIGNED_HYPER:
        retVal= __typeof(System::UInt64); break;
    case typelib_TypeClass_FLOAT:
        retVal= __typeof(System::Single); break;
    case typelib_TypeClass_DOUBLE:
        retVal= __typeof(System::Double); break;
    case typelib_TypeClass_STRING:
        retVal= __typeof(System::String); break;
    case typelib_TypeClass_TYPE:
        retVal= __typeof(System::Type); break;
    case typelib_TypeClass_ANY:
        retVal= __typeof(uno::Any); break;
    case typelib_TypeClass_ENUM:
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        retVal= loadCliType(pTD->pTypeName); break;
    case typelib_TypeClass_INTERFACE:
    {
        //special handling for XInterface, since it does not exist in cli.
		rtl::OUString usXInterface(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface"));
		if (usXInterface.equals(pTD->pTypeName))
			retVal= __typeof(System::Object);
		else
            retVal= loadCliType(pTD->pTypeName);
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        css::uno::TypeDescription seqType(
            const_cast<typelib_TypeDescriptionReference*>(pTD));
        typelib_TypeDescriptionReference* pElementTDRef=
            reinterpret_cast<typelib_IndirectTypeDescription*>(seqType.get())->pType;
        switch (pElementTDRef->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArChar)); break;
        case typelib_TypeClass_BOOLEAN:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArBoolean));
            break;
        case typelib_TypeClass_BYTE:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArByte));
            break;
        case typelib_TypeClass_SHORT:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArInt16));
            break;
        case typelib_TypeClass_UNSIGNED_SHORT:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArUInt16));
            break;
        case typelib_TypeClass_LONG:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArInt32));
            break;
        case typelib_TypeClass_UNSIGNED_LONG:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArUInt32));
            break;
        case typelib_TypeClass_HYPER:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArInt64));
            break;
        case typelib_TypeClass_UNSIGNED_HYPER:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArUInt64));
            break;
        case typelib_TypeClass_FLOAT:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArSingle));
            break;
        case typelib_TypeClass_DOUBLE:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArDouble));
            break;
        case typelib_TypeClass_STRING:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArString));
            break;
        case typelib_TypeClass_TYPE:
            retVal= System::Type::GetType(const_cast<System::String*>(Constants::sArType));
            break;
        case typelib_TypeClass_ANY:    
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_INTERFACE:
        case typelib_TypeClass_SEQUENCE:
        {
            retVal= loadCliType(pTD->pTypeName);
            break;
        }
        default:
            //All cases should be handled by the case statements above
            OSL_ASSERT(0);
            break;
        }
        break;
    }
    default:
        OSL_ASSERT(false);
        break;
    }
    return retVal;
}

/** Returns an acquired td.
 */
typelib_TypeDescriptionReference* mapCliType(System::Type* cliType)
{
    typelib_TypeDescriptionReference* retVal= NULL;
    if (cliType == NULL)
    {
        retVal = * typelib_static_type_getByTypeClass(
            typelib_TypeClass_VOID );
        typelib_typedescriptionreference_acquire( retVal );
        return retVal;
    }
    //check for Enum first,
    //because otherwise case System::TypeCode::Int32 applies
    if (cliType->get_IsEnum())
    {
        OUString usTypeName= mapCliTypeName(cliType->get_FullName());
        css::uno::Type unoType(css::uno::TypeClass_ENUM, usTypeName);
        retVal= unoType.getTypeLibType();
        typelib_typedescriptionreference_acquire(retVal);
    }
    else
    {
        switch (System::Type::GetTypeCode(cliType))
        {
        case System::TypeCode::Boolean:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_BOOLEAN );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::Char:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_CHAR );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::Byte:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_BYTE );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::Int16:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_SHORT );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::Int32:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_LONG );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::Int64:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_HYPER );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::UInt16:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_UNSIGNED_SHORT );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::UInt32:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_UNSIGNED_LONG );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::UInt64:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_UNSIGNED_HYPER );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::Single:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_FLOAT );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::Double:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_DOUBLE );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        case System::TypeCode::String:
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_STRING );
            typelib_typedescriptionreference_acquire( retVal );
            break;
        default:
            break;
        }
    }
    if (retVal == NULL)
    {
        System::String* cliTypeName= cliType->get_FullName();
        // Void
        if (const_cast<System::String*>(Constants::sVoid)->Equals(
                cliTypeName))
        {
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_VOID );
            typelib_typedescriptionreference_acquire( retVal );
        }
        // Type
        else if (const_cast<System::String*>(Constants::sType)->Equals(
                     cliTypeName))
        {
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_TYPE );
            typelib_typedescriptionreference_acquire( retVal );
        }
        // Any
        else if (const_cast<System::String*>(Constants::sAny)->Equals(
                     cliTypeName))
        {
            retVal = * typelib_static_type_getByTypeClass(
                typelib_TypeClass_ANY );
            typelib_typedescriptionreference_acquire( retVal );
        }
        //struct, interfaces, sequences 
        else
        {
            OUString usTypeName;
            uno::PolymorphicType * poly = dynamic_cast<uno::PolymorphicType*>(cliType);
            if (poly != NULL)
                usTypeName = mapCliTypeName( poly->PolymorphicName);
            else
                usTypeName = mapCliTypeName(cliTypeName);
            typelib_TypeDescription* td = NULL;
            typelib_typedescription_getByName(&td, usTypeName.pData);
            if (td)
            {
                retVal = td->pWeakRef;
                typelib_typedescriptionreference_acquire(retVal);
                typelib_typedescription_release(td);
            }
        }
    }
    if (retVal == NULL)
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("[cli_uno bridge] mapCliType():"
                                       "could not map type: ") );
        buf.append(mapCliString(cliType->get_FullName()));
        throw BridgeRuntimeError( buf.makeStringAndClear() );
    }
    return retVal;
}

/** 
    Otherwise a leading "unoidl." is removed.
 */
System::String* mapUnoTypeName(rtl_uString const * typeName)
{
    OUString usUnoName( const_cast< rtl_uString * >( typeName ) );
    st::StringBuilder* buf= new st::StringBuilder();
    //determine if the type is a sequence and its dimensions
    int dims= 0;
    if (usUnoName[0] == '[')
    {
        sal_Int32 index= 1;
        while (true)
        {
            if (usUnoName[index++] == ']')
                dims++;
            if (usUnoName[index++] != '[')
                break;
        }
        usUnoName = usUnoName.copy(index - 1);
    }
	System::String * sUnoName = mapUnoString(usUnoName.pData);
	if (sUnoName->Equals(const_cast<System::String*>(Constants::usBool)))
        buf->Append(const_cast<System::String*>(Constants::sBoolean));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usChar)))
        buf->Append(const_cast<System::String*>(Constants::sChar));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usByte)))
        buf->Append(const_cast<System::String*>(Constants::sByte));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usShort)))
        buf->Append(const_cast<System::String*>(Constants::sInt16));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usUShort)))
        buf->Append(const_cast<System::String*>(Constants::sUInt16));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usLong)))
        buf->Append(const_cast<System::String*>(Constants::sInt32));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usULong)))
        buf->Append(const_cast<System::String*>(Constants::sUInt32));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usHyper)))
        buf->Append(const_cast<System::String*>(Constants::sInt64));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usUHyper)))
        buf->Append(const_cast<System::String*>(Constants::sUInt64));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usFloat)))
        buf->Append(const_cast<System::String*>(Constants::sSingle));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usDouble)))
        buf->Append(const_cast<System::String*>(Constants::sDouble));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usString)))
        buf->Append(const_cast<System::String*>(Constants::sString));    
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usVoid)))
        buf->Append(const_cast<System::String*>(Constants::sVoid));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usType)))
        buf->Append(const_cast<System::String*>(Constants::sType));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usXInterface)))
        buf->Append(const_cast<System::String*>(Constants::sObject));
    else if (sUnoName->Equals(const_cast<System::String*>(Constants::usAny)))
    {        
        buf->Append(const_cast<System::String*>(Constants::sAny));
    }
    else
    {
        //put "unoidl." at the beginning
        buf->Append(const_cast<System::String*>(Constants::sUnoidl));
        //for polymorphic struct types remove the brackets, e.g mystruct<bool> -> mystruct
		System::String * sName = mapUnoPolymorphicName(sUnoName);
        buf->Append(sName);
    }
    // apend []
    for (;dims--;)
        buf->Append(const_cast<System::String*>(Constants::sBrackets));

    return buf->ToString();
}




/** For example, there is a uno type
    com.sun.star.Foo<char, long>.
    The values in the type list
    are uno types and are replaced by cli types, such as System.Char,
    System.Int32, etc.
    The pr�fix unoidl is not added.
 */
inline System::String* mapUnoPolymorphicName(System::String* unoName)
{
       return mapPolymorphicName(unoName, false);
}
/** For example, there is a type name such as
    com.sun.star.Foo<System.Char, System.Int32>.
    The values in the type list
    are CLI types and are replaced by uno types, such as char,
    long, etc.
    The pr�fix unoidl remains.
 */
inline System::String* mapCliPolymorphicName(System::String* unoName)
{
    return mapPolymorphicName(unoName, true);
}

System::String* mapPolymorphicName(System::String* unoName, bool bCliToUno)
{
    int index = unoName->IndexOf('<');
    if (index == -1)
        return unoName;
    
    System::Text::StringBuilder * builder = new System::Text::StringBuilder(256);
    builder->Append(unoName->Substring(0, index +1 ));

	//Find the first occurrence of ','
	//If the parameter is a polymorphic struct then we neede to ignore everything
	//between the brackets because it can also contain commas
    //get the type list within < and >
	int endIndex = unoName->Length - 1;
	index++;
	int cur = index;
	int countParams = 0;
	while (cur <= endIndex)
	{
		System::Char c = unoName->Chars[cur];
		if (c == ',' || c == '>')
		{
			//insert a comma if needed
			if (countParams != 0)
				builder->Append(S",");
			countParams++;
			System::String * sParam = unoName->Substring(index, cur - index);
			//skip the comma
			cur++;
			//the the index to the beginning of the next param
			index = cur;
            if (bCliToUno)
            {
                builder->Append(mapCliTypeName(sParam));
            }
            else
            {
                OUString s = mapCliString(sParam);
                builder->Append(mapUnoTypeName(s.pData));
            }
		}
		else if (c == '<')
		{
			cur++;
			//continue until the matching '>'
			int numNested = 0;
			for (;;cur++)
			{
				System::Char curChar = unoName->Chars[cur];
				if (curChar == '<')
				{
					numNested ++;
				}
				else if (curChar == '>')
				{
					if (numNested > 0)
						numNested--;
					else
						break;
				}
			}
		}
		cur++;
	}

    builder->Append((System::Char) '>');
    return builder->ToString();
}

OUString mapCliTypeName(System::String* typeName)
{
    int dims= 0;
    // Array? determine the "rank" (number of "[]")
    // move from the rightmost end to the left, for example
    // unoidl.PolymorphicStruct<System.Char[]>[]
    // has only a "dimension" of 1
	int cur = typeName->Length - 1;
    bool bRightBracket = false;
	while (cur >= 0)
	{
		System::Char c = typeName->Chars[cur];
		if (c == ']')
		{
            bRightBracket = true;
		}
        else if (c == '[')
        {
            if (!bRightBracket)
                throw BridgeRuntimeError(
                    OUSTR("Typename is wrong. No matching brackets for sequence. Name is: ") +
                    mapCliString(typeName));
            bRightBracket = false;
            dims ++;
        }
        else
        {
            if (bRightBracket)
                throw BridgeRuntimeError(
                    OUSTR("Typename is wrong. No matching brackets for sequence. Name is: ") +
                    mapCliString(typeName));
            break;
        }
        cur--;
	}
    
    if (bRightBracket || cur < 0)
        throw BridgeRuntimeError(
            OUSTR("Typename is wrong. ") +
            mapCliString(typeName));

    typeName = typeName->Substring(0, cur + 1);

	System::Text::StringBuilder * buf = new System::Text::StringBuilder(512);

    //Put the "[]" at the beginning of the uno type name
    for (;dims--;)
		buf->Append(const_cast<System::String*>(Constants::usBrackets));

    if (typeName->Equals(const_cast<System::String*>(Constants::sBoolean)))
        buf->Append(const_cast<System::String*>(Constants::usBool));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sChar)))
        buf->Append(const_cast<System::String*>(Constants::usChar));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sByte)))
        buf->Append(const_cast<System::String*>(Constants::usByte));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sInt16)))
        buf->Append(const_cast<System::String*>(Constants::usShort));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sUInt16)))
        buf->Append(const_cast<System::String*>(Constants::usUShort));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sInt32)))
        buf->Append(const_cast<System::String*>(Constants::usLong));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sUInt32)))
        buf->Append(const_cast<System::String*>(Constants::usULong));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sInt64)))
        buf->Append(const_cast<System::String*>(Constants::usHyper));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sUInt64)))
        buf->Append(const_cast<System::String*>(Constants::usUHyper));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sSingle)))
        buf->Append(const_cast<System::String*>(Constants::usFloat));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sDouble)))
        buf->Append(const_cast<System::String*>(Constants::usDouble));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sString)))
        buf->Append(const_cast<System::String*>(Constants::usString));    
    else if (typeName->Equals(const_cast<System::String*>(Constants::sVoid)))
        buf->Append(const_cast<System::String*>(Constants::usVoid));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sType)))
        buf->Append(const_cast<System::String*>(Constants::usType));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sObject)))
        buf->Append(const_cast<System::String*>(Constants::usXInterface));
    else if (typeName->Equals(const_cast<System::String*>(Constants::sAny)))
        buf->Append(const_cast<System::String*>(Constants::usAny));
    else
    {
        System::String * sName = mapCliPolymorphicName(typeName);
        int i= sName->IndexOf(L'.');
        buf->Append(sName->Substring(i + 1));
    }
    return mapCliString(buf->ToString());
}
/** Maps uno types to dot net types.
 *  If uno_data is null then the type description is converted to System::Type
 */
inline System::String* mapUnoString( rtl_uString const * data)
{
    OSL_ASSERT(data);
    return new System::String((__wchar_t*) data->buffer, 0, data->length);
}

OUString mapCliString(System::String const * data)    
{

    if (data != NULL)
    {
        OSL_ASSERT(sizeof(wchar_t) == sizeof(sal_Unicode));
        wchar_t const __pin * pdata= PtrToStringChars(data);
        return OUString(pdata, const_cast<System::String*>(data)->get_Length());
    }
    else
    {
        return OUString();
    }
}

// ToDo convert cli types to expected types, e.g a long to a short where the uno type
// is a sal_Int16. This could be necessary if a scripting language (typeless) is used
// @param assign the uno_data has to be destructed (in/out args)
void Bridge::map_to_uno(void * uno_data, System::Object* cli_data,
                        typelib_TypeDescriptionReference * type, 
                        bool assign) const
{
    try{
        switch (type->eTypeClass)
        {
        case typelib_TypeClass_VOID:
            break;
        case typelib_TypeClass_CHAR:
        {
            System::Char aChar= *__try_cast<System::Char*>(cli_data);
            *(sal_Unicode*) uno_data= aChar;
            break;
        }
        case typelib_TypeClass_BOOLEAN:
        {
            System::Boolean aBool= *__try_cast<System::Boolean*>(cli_data);
            *(sal_Bool*)uno_data= aBool == true ? sal_True : sal_False;
            break;
        }
        case typelib_TypeClass_BYTE:
        {
            System::Byte aByte= *__try_cast<System::Byte*>(cli_data);
            *(sal_Int8*) uno_data= aByte;
            break;
        }
        case typelib_TypeClass_SHORT:
        {
            System::Int16 aShort= *__try_cast<System::Int16*>(cli_data);
            *(sal_Int16*) uno_data= aShort;
            break;
        }
        case typelib_TypeClass_UNSIGNED_SHORT:
        {
            System::UInt16 aUShort= *__try_cast<System::UInt16*>(cli_data);
            *(sal_uInt16*) uno_data= aUShort;
            break;
        }
        case typelib_TypeClass_LONG:
        {
            System::Int32 aLong= *__try_cast<System::Int32*>(cli_data);
            *(sal_Int32*) uno_data= aLong;
            break;
        }
        case typelib_TypeClass_UNSIGNED_LONG:
        {
            System::UInt32 aULong= *__try_cast<System::UInt32*>(cli_data);
            *(sal_uInt32*) uno_data= aULong;
            break;
        }
        case typelib_TypeClass_HYPER:
        {
            System::Int64 aHyper= *__try_cast<System::Int64*>(cli_data);
            *(sal_Int64*) uno_data= aHyper;
            break;
        }
        case typelib_TypeClass_UNSIGNED_HYPER:
        {
            System::UInt64 aLong= *__try_cast<System::UInt64*>(cli_data);
            *(sal_uInt64*) uno_data= aLong;
            break;
        }
        case typelib_TypeClass_FLOAT:
        {
            System::Single aFloat= *__try_cast<System::Single*>(cli_data);
            *(float*) uno_data= aFloat;
            break;
        }
        case typelib_TypeClass_DOUBLE:
        {
            System::Double aDouble= *__try_cast<System::Double*>(cli_data);
            *(double*) uno_data= aDouble;
            break;
        }
        case typelib_TypeClass_STRING:
        {
            if (assign && *(rtl_uString**) uno_data)
                rtl_uString_release(*(rtl_uString**) uno_data);

            *(rtl_uString **)uno_data = 0;
            if (cli_data == NULL)
            {
                 rtl_uString_new((rtl_uString**) uno_data);
            }
            else
            {
                System::String *s= __try_cast<System::String*>(cli_data);
                wchar_t const __pin * pdata= PtrToStringChars(s);
                rtl_uString_newFromStr_WithLength( (rtl_uString**) uno_data,
                                                  pdata, s->get_Length() );
            }
            break;
        }
        case typelib_TypeClass_TYPE:
        {
            typelib_TypeDescriptionReference* td= mapCliType(__try_cast<System::Type*>(
                                                                    cli_data));
            if (assign)
            {
                typelib_typedescriptionreference_release(
                    *(typelib_TypeDescriptionReference **)uno_data );
            }
            *(typelib_TypeDescriptionReference **)uno_data = td;
            break;
        }
        case typelib_TypeClass_ANY:
        {
            uno_Any * pAny = (uno_Any *)uno_data;
            if (cli_data == NULL) // null-ref or uninitialized any maps to empty any
            {
                if (assign)
                    uno_any_destruct( pAny, 0 );
                uno_any_construct( pAny, 0, 0, 0 );
                break;
            }
            uno::Any aAny= *__try_cast<uno::Any*>(cli_data);
            css::uno::Type  value_td( mapCliType(aAny.Type), SAL_NO_ACQUIRE);
        
            if (assign)
                uno_any_destruct( pAny, 0 );

            try
            {
                switch (value_td.getTypeClass())
                {
                case typelib_TypeClass_VOID:
                    pAny->pData = &pAny->pReserved;
                    break;
                case typelib_TypeClass_CHAR:
                    pAny->pData = &pAny->pReserved;
                    *(sal_Unicode*) &pAny->pReserved = *__try_cast<System::Char*>(aAny.Value);
                    break;
                case typelib_TypeClass_BOOLEAN:
                    pAny->pData = &pAny->pReserved;
                    *(sal_Bool *) &pAny->pReserved = *__try_cast<System::Boolean*>(aAny.Value);
                    break;
                case typelib_TypeClass_BYTE:
                    pAny->pData = &pAny->pReserved;
                    *(sal_Int8*) &pAny->pReserved =  *__try_cast<System::Byte*>(aAny.Value);
                    break;
                case typelib_TypeClass_SHORT:
                    pAny->pData = &pAny->pReserved;
                    *(sal_Int16*) &pAny->pReserved =  *__try_cast<System::Int16*>(aAny.Value);
                    break;
                case typelib_TypeClass_UNSIGNED_SHORT:
                    pAny->pData = &pAny->pReserved;
                    *(sal_uInt16*) &pAny->pReserved =  *__try_cast<System::UInt16*>(aAny.Value);
                    break;
                case typelib_TypeClass_LONG:
                    pAny->pData = &pAny->pReserved;
                    *(sal_Int32*) &pAny->pReserved =  *__try_cast<System::Int32*>(aAny.Value);
                    break;                
                case typelib_TypeClass_UNSIGNED_LONG:
                    pAny->pData = &pAny->pReserved;
                    *(sal_uInt32*) &pAny->pReserved =  *__try_cast<System::UInt32*>(aAny.Value);
                    break;
                case typelib_TypeClass_HYPER:
                    if (sizeof (sal_Int64) <= sizeof (void *))
                    {
                        pAny->pData = &pAny->pReserved;
                        *(sal_Int64*) &pAny->pReserved = *__try_cast<System::Int64*>(aAny.Value);
                    }
                    else
                    {
                        auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (sal_Int64) ) );
                        *(sal_Int64 *) mem.get()=  *__try_cast<System::Int64*>(aAny.Value);
                        pAny->pData = mem.release();
                    }
                    break;                
                case typelib_TypeClass_UNSIGNED_HYPER:
                    if (sizeof (sal_uInt64) <= sizeof (void *))
                    {
                        pAny->pData = &pAny->pReserved;
                        *(sal_uInt64*) &pAny->pReserved = *__try_cast<System::UInt64*>(aAny.Value);
                    }
                    else
                    {
                        auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (sal_uInt64) ) );
                        *(sal_uInt64 *) mem.get()=  *__try_cast<System::UInt64*>(aAny.Value);
                        pAny->pData = mem.release();
                    }
                    break;
                case typelib_TypeClass_FLOAT:
                    if (sizeof (float) <= sizeof (void *))
                    {
                        pAny->pData = &pAny->pReserved;
                        *(float*) &pAny->pReserved = *__try_cast<System::Single*>(aAny.Value);
                    }
                    else
                    {
                        auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (float) ) );
                        *(float*) mem.get() = *__try_cast<System::Single*>(aAny.Value);
                        pAny->pData = mem.release();
                    }
                    break;
                case typelib_TypeClass_DOUBLE:
                    if (sizeof (double) <= sizeof (void *))
                    {
                        pAny->pData = &pAny->pReserved;
                        *(double*) &pAny->pReserved= *__try_cast<System::Double*>(aAny.Value);
                    }
                    else
                    {
                        auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (double) ) );
                        *(double*) mem.get()= *__try_cast<System::Double*>(aAny.Value);
                        pAny->pData= mem.release();
                    }
                    break;
                case typelib_TypeClass_STRING: // anies often contain strings; copy string directly
                {
                    pAny->pData= &pAny->pReserved;
                    OUString _s = mapCliString(static_cast<System::String*>(aAny.Value));
                    pAny->pReserved= _s.pData;
                    rtl_uString_acquire(_s.pData);
                    break;
                }
                case typelib_TypeClass_TYPE:
                case typelib_TypeClass_ENUM:  //ToDo copy enum direct
                case typelib_TypeClass_SEQUENCE:
                case typelib_TypeClass_INTERFACE:
                    pAny->pData = &pAny->pReserved;
                    pAny->pReserved = 0;
                    map_to_uno(
                        &pAny->pReserved, aAny.Value, value_td.getTypeLibType(),
                                                  false /* no assign */);
                    break;
                case typelib_TypeClass_STRUCT:
                case typelib_TypeClass_EXCEPTION:
                {
                    css::uno::Type anyType(value_td);
                    typelib_TypeDescription* td= NULL;
                    anyType.getDescription(&td);
                    auto_ptr< rtl_mem > mem(rtl_mem::allocate(td->nSize));
                    typelib_typedescription_release(td);
                    map_to_uno(
                        mem.get(), aAny.Value, value_td.getTypeLibType(),
                        false /* no assign */);
                    pAny->pData = mem.release();
                    break;
                }
                default:
                {
                    OUStringBuffer buf( 128 );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():") );
                    buf.append(value_td.getTypeName());
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] unsupported value type of any!") );
                    throw BridgeRuntimeError( buf.makeStringAndClear() );
                }
                }
            }
            catch(System::InvalidCastException* )
            {
// ToDo check this
                if (assign)
                    uno_any_construct( pAny, 0, 0, 0 ); // restore some valid any
                OUStringBuffer buf( 256 );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():Any") );
                buf.append(value_td.getTypeName());
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("]The Any type "));
                buf.append(value_td.getTypeName());
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" does not correspont  "
                                                            "to its value type: ") );
                if(aAny.Value != NULL)
                {
                    css::uno::Type td(mapCliType(aAny.Value->GetType()), SAL_NO_ACQUIRE);
                    buf.append(td.getTypeName());
                }
                if (assign)
                    uno_any_construct( pAny, 0, 0, 0 ); // restore some valid any
                throw BridgeRuntimeError( buf.makeStringAndClear() );
            }
            catch (BridgeRuntimeError& )
            {
                if (assign)
                    uno_any_construct( pAny, 0, 0, 0 ); // restore some valid any
                throw;
            }
            catch (...)
            {
                if (assign)
                    uno_any_construct( pAny, 0, 0, 0 ); // restore some valid any                
                throw;
            }
            
            pAny->pType = value_td.getTypeLibType();
            typelib_typedescriptionreference_acquire(pAny->pType);
            break;
        }
        case typelib_TypeClass_ENUM:
        {
            // InvalidCastException is caught at the end of this method
            System::Int32 aEnum= System::Convert::ToInt32((cli_data));
            *(sal_Int32*) uno_data = aEnum;
            break;
        }
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            css::uno::TypeDescription td(type);        
            typelib_CompoundTypeDescription * comp_td =
                (typelib_CompoundTypeDescription*) td.get();
            
            typelib_StructTypeDescription * struct_td = NULL;
            if (type->eTypeClass == typelib_TypeClass_STRUCT)
                struct_td = (typelib_StructTypeDescription*) td.get();

            if ( ! ((typelib_TypeDescription*) comp_td)->bComplete)
                ::typelib_typedescription_complete(
                    (typelib_TypeDescription**) & comp_td );
            
            sal_Int32 nMembers = comp_td->nMembers;
            boolean bException= false;
            System::Type* cliType = NULL;
            if (cli_data)
                cliType = cli_data->GetType();

            if (0 != comp_td->pBaseTypeDescription)
            {
                map_to_uno(
                    uno_data, cli_data,
                    ((typelib_TypeDescription *)comp_td->pBaseTypeDescription)->pWeakRef,
                    assign);
            }
            sal_Int32 nPos = 0;
            try
            {
                typelib_TypeDescriptionReference * member_type= NULL;

				rtl::OUString usUnoException(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception"));
                for (; nPos < nMembers; ++nPos)
                {
                    member_type= comp_td->ppTypeRefs[nPos];
#if OSL_DEBUG_LEVEL >= 2
                    System::String* __s;
                    sr::FieldInfo* arFields[];
                    __s = mapUnoString(comp_td->ppMemberNames[nPos]);
                    arFields = cliType != NULL ? cliType->GetFields() : NULL;
#endif
                    System::Object* val= NULL;
                    if (cli_data != NULL)
                    {
                        sr::FieldInfo* aField= cliType->GetField(
                            mapUnoString(comp_td->ppMemberNames[nPos]));
                        // special case for Exception.Message property
                        // The com.sun.star.uno.Exception.Message field is mapped to the
                        // System.Exception property. Type.GetField("Message") returns null
                        if ( ! aField && usUnoException.equals(td.get()->pTypeName))
                        {// get Exception.Message property
							rtl::OUString usMessageMember(RTL_CONSTASCII_USTRINGPARAM("Message"));
                            if (usMessageMember.equals(comp_td->ppMemberNames[nPos]))
                            {
                                sr::PropertyInfo* pi= cliType->GetProperty(
                                    mapUnoString(comp_td->ppMemberNames[nPos]));
                                val= pi->GetValue(cli_data, NULL);
                            }
                            else
                            {
                                OUStringBuffer buf(512);
                                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("[map_to_uno(): Member: "));
                                buf.append(comp_td->ppMemberNames[nPos]);
                                throw BridgeRuntimeError(buf.makeStringAndClear());
                            }
                        }
                        else
                        {
                            val= aField->GetValue(cli_data);
                        }
                    }
                    void * p = (char *) uno_data + comp_td->pMemberOffsets[ nPos ];
                    //When using polymorphic structs then the parameterized members can be null.
                    //Then we set a default value.
                    bool bDefault = ((struct_td != NULL
                                     && struct_td->pParameterizedTypes != NULL
                                     && struct_td->pParameterizedTypes[nPos] == sal_True
                                      && val == NULL)
                                     || cli_data == NULL) ? true : false;
                    switch (member_type->eTypeClass)
                    {
                    case typelib_TypeClass_CHAR:
                        if (bDefault)
                            *(sal_Unicode*) p = 0;
                        else
                            *(sal_Unicode*) p = *__try_cast<System::Char*>(val);
                        break;
                    case typelib_TypeClass_BOOLEAN:
                        if (bDefault)
                            *(sal_Bool*) p = sal_False;
                        else
                            *(sal_Bool*) p = *__try_cast<System::Boolean*>(val);
                        break;
                    case typelib_TypeClass_BYTE:
                        if (bDefault)
                            *(sal_Int8*) p = 0;
                        else
                            *(sal_Int8*) p = *__try_cast<System::Byte*>(val);
                        break;
                    case typelib_TypeClass_SHORT:
                        if (bDefault)
                            *(sal_Int16*) p = 0;
                        else
                            *(sal_Int16*) p = *__try_cast<System::Int16*>(val);
                        break;
                    case typelib_TypeClass_UNSIGNED_SHORT:
                        if (bDefault)
                            *(sal_uInt16*) p = 0;
                        else
                            *(sal_uInt16*) p = *__try_cast<System::UInt16*>(val);
                        break;
                    case typelib_TypeClass_LONG:
                        if (bDefault)
                            *(sal_Int32*) p = 0;
                        else
                            *(sal_Int32*) p = *__try_cast<System::Int32*>(val);
                        break;
                    case typelib_TypeClass_UNSIGNED_LONG:
                        if (bDefault)
                            *(sal_uInt32*) p = 0;
                        else
                            *(sal_uInt32*) p = *__try_cast<System::UInt32*>(val);
                        break;
                    case typelib_TypeClass_HYPER:
                        if (bDefault)
                            *(sal_Int64*) p = 0;
                        else
                            *(sal_Int64*) p = *__try_cast<System::Int64*>(val);
                        break;
                    case typelib_TypeClass_UNSIGNED_HYPER:
                        if (bDefault)
                            *(sal_uInt64*) p = 0;
                        else
                            *(sal_uInt64*) p= *__try_cast<System::UInt64*>(val);
                        break;
                    case typelib_TypeClass_FLOAT:
                        if (bDefault)
                            *(float*) p = 0.;
                        else
                            *(float*) p = *__try_cast<System::Single*>(val);
                        break;
                    case typelib_TypeClass_DOUBLE:
                        if (bDefault)
                            *(double*) p = 0.;
                        else
                            *(double*) p = *__try_cast<System::Double*>(val);
                        break;
                    default:
                    {   // ToDo enum, should be converted here
                         map_to_uno(p, val, member_type, assign);
                        break;
                    }
                    }
                }
            }
            catch (BridgeRuntimeError& e)
            {
                bException= true;
                OUStringBuffer buf(512);
                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("[map_to_uno():"));
                if (cliType)
                {
                    buf.append(mapCliString(cliType->get_FullName()));
                    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("."));
                    buf.append(comp_td->ppMemberNames[nPos]);
                    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" "));
                }
                buf.append(e.m_message);
                throw BridgeRuntimeError(buf.makeStringAndClear());
            }
            catch (System::InvalidCastException* )
            {
                bException= true;
                OUStringBuffer buf( 256 );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():") );
                if (cliType)
                {
                    buf.append(mapCliString(cliType->get_FullName()));
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("."));
                    buf.append(comp_td->ppMemberNames[nPos]);
                }
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] Value has not the required type."));
                throw BridgeRuntimeError(buf.makeStringAndClear());
            }
            catch (...)
            {
                OSL_ASSERT(0);
                bException= true;
                throw;
            }
            __finally            
            {    
                if (bException && !assign) // if assign then caller cleans up
                {
                    // cleanup the members which we have converted so far
                    for ( sal_Int32 nCleanup = 0; nCleanup < nPos; ++nCleanup )
                    {
                        uno_type_destructData(
                            uno_data, comp_td->ppTypeRefs[ nCleanup ], 0 );
                    }
                    if (0 != comp_td->pBaseTypeDescription)
                    {
                        uno_destructData(
                            uno_data, (typelib_TypeDescription *)comp_td->pBaseTypeDescription, 0 );
                    }
                }            
            }
            break;
        }
        case typelib_TypeClass_SEQUENCE:
        {
            TypeDescr td( type );
            typelib_TypeDescriptionReference * element_type =
                ((typelib_IndirectTypeDescription *)td.get())->pType;
        
            auto_ptr< rtl_mem > seq;
            
            System::Array* ar = NULL;
            if (cli_data != NULL)
            {
                ar = __try_cast<System::Array*>(cli_data);
                sal_Int32 nElements = ar->GetLength(0);
            
                try
                {
                    switch (element_type->eTypeClass)
                    {
                    case typelib_TypeClass_CHAR:
                        seq = seq_allocate(nElements, sizeof (sal_Unicode));
                        sri::Marshal::Copy(__try_cast<System::Char[]>(cli_data), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_BOOLEAN:
                        seq = seq_allocate(nElements, sizeof (sal_Bool));
                        sri::Marshal::Copy(__try_cast<System::Boolean[]>(cli_data), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_BYTE:
                        seq = seq_allocate( nElements, sizeof (sal_Int8) );
                    sri::Marshal::Copy(__try_cast<System::Byte[]>(cli_data), 0,
                                       & ((uno_Sequence*) seq.get())->elements, nElements);
                    break;
                    case typelib_TypeClass_SHORT:
                        seq = seq_allocate(nElements, sizeof (sal_Int16));
                        sri::Marshal::Copy(__try_cast<System::Int16[]>(cli_data), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_UNSIGNED_SHORT:
                        seq = seq_allocate( nElements, sizeof (sal_uInt16) );
                        sri::Marshal::Copy(static_cast<System::Int16[]>(
                                               __try_cast<System::UInt16[]>(cli_data)), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_LONG:
                        seq = seq_allocate(nElements, sizeof (sal_Int32));
                        sri::Marshal::Copy(__try_cast<System::Int32[]>(cli_data), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_UNSIGNED_LONG:
                        seq = seq_allocate( nElements, sizeof (sal_uInt32) );
                        sri::Marshal::Copy(static_cast<System::Int32[]>(
                                               __try_cast<System::UInt32[]>(cli_data)), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_HYPER:
                        seq = seq_allocate(nElements, sizeof (sal_Int64));
                        sri::Marshal::Copy(__try_cast<System::Int64[]>(cli_data), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_UNSIGNED_HYPER:
                        seq = seq_allocate(nElements, sizeof (sal_uInt64));
                        sri::Marshal::Copy(static_cast<System::Int64[]>(
                                               __try_cast<System::UInt64[]>(cli_data)), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_FLOAT:
                        seq = seq_allocate(nElements, sizeof (float));
                        sri::Marshal::Copy(__try_cast<System::Single[]>(cli_data), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_DOUBLE:
                        seq = seq_allocate(nElements, sizeof (double));
                        sri::Marshal::Copy(__try_cast<System::Double[]>(cli_data), 0,
                                           & ((uno_Sequence*) seq.get())->elements, nElements);
                        break;
                    case typelib_TypeClass_STRING:
                    {
                        seq = seq_allocate(nElements, sizeof (rtl_uString*));
                        System::String* arStr[]= __try_cast<System::String*[]>(cli_data);
                        for (int i= 0; i < nElements; i++)
                        {
                            wchar_t const __pin * pdata= PtrToStringChars(arStr[i]);					
                            rtl_uString** pStr=  & ((rtl_uString**) &
                                                    ((uno_Sequence*) seq.get())->elements)[i];
                            *pStr= NULL;
                            rtl_uString_newFromStr_WithLength( pStr, pdata,
                                                               arStr[i]->get_Length());
                        }
                        break;
                    }
                    case typelib_TypeClass_ENUM:
                        seq = seq_allocate(nElements, sizeof (sal_Int32));
                        for (int i= 0; i < nElements; i++)
                        {
                            ((sal_Int32*) &((uno_Sequence*) seq.get())->elements)[i]=
                                System::Convert::ToInt32(ar->GetValue(i));
                        }
                        break;            
                    case typelib_TypeClass_TYPE:
                    case typelib_TypeClass_ANY:
                    case typelib_TypeClass_STRUCT:
                    case typelib_TypeClass_EXCEPTION:
                    case typelib_TypeClass_SEQUENCE:
                    case typelib_TypeClass_INTERFACE:
                    {
                        TypeDescr element_td( element_type );
                        seq = seq_allocate( nElements, element_td.get()->nSize );
                        
                        for (sal_Int32 nPos = 0; nPos < nElements; ++nPos)
                        {
                            try
                            {
                                void * p= ((uno_Sequence *) seq.get())->elements +
                                    (nPos * element_td.get()->nSize);
                                System::Object* elemData= dynamic_cast<System::Array*>(cli_data)->GetValue(nPos);
                                map_to_uno(
                                    p, elemData, element_td.get()->pWeakRef,
                                    false /* no assign */);
                            }
                            catch (...)
                            {
                                // cleanup
                                for ( sal_Int32 nCleanPos = 0; nCleanPos < nPos; ++nCleanPos )
                                {
                                    void * p =
                                        ((uno_Sequence *)seq.get())->elements +
                                        (nCleanPos * element_td.get()->nSize);
                                    uno_destructData( p, element_td.get(), 0 );
                                }
                                throw;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        OUStringBuffer buf( 128 );
                        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():") );
                        buf.append( *reinterpret_cast< OUString const * >( &type->pTypeName ) );
                        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] unsupported sequence element type: ") );
                        buf.append( *reinterpret_cast< OUString const * >( &element_type->pTypeName ) );
                        throw BridgeRuntimeError( buf.makeStringAndClear() );
                    }            
                    }
                }
                catch (BridgeRuntimeError& e)
                {
                    OUStringBuffer buf( 128 );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():") );
                    buf.append( *reinterpret_cast< OUString const * >( &type->pTypeName ));
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] conversion failed\n "));
                    buf.append(e.m_message);
                    throw BridgeRuntimeError(buf.makeStringAndClear());
                }
                catch (System::InvalidCastException* )
                {
                    // Ok, checked
                    OUStringBuffer buf( 128 );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():") );
                    buf.append( *reinterpret_cast< OUString const * >( &type->pTypeName) );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] could not convert sequence element type: ") );
                    buf.append( *reinterpret_cast< OUString const * >( &element_type->pTypeName ) );
                    throw BridgeRuntimeError( buf.makeStringAndClear() );
                }
                catch (...)
                {
                    OSL_ASSERT(0);
                    throw;
                }
                __finally
                    {
                        if (assign) 
                            uno_destructData( uno_data, td.get(), 0 );
                    }
            }
            else
            {
                seq = seq_allocate(0, sizeof (sal_Int32));
            }
            *(uno_Sequence **)uno_data = (uno_Sequence *)seq.release();
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            if (assign)
            {
                uno_Interface * p = *(uno_Interface **)uno_data;
                if (0 != p)
                    (*p->release)( p );
            }        
            if (0 == cli_data) // null-ref
            {
                *(uno_Interface **)uno_data = 0;
            }
            else
            {
                TypeDescr td( type );
                uno_Interface * pUnoI = map_cli2uno(cli_data, td.get());
                *(uno_Interface **)uno_data = pUnoI;
            }
            break;
        }    
        default:
        {
            //ToDo check
            OUStringBuffer buf( 128 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():") );
            buf.append( *reinterpret_cast< OUString const * >( &type->pTypeName ) );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] unsupported type!") );
            throw BridgeRuntimeError( buf.makeStringAndClear() );
        }
        }
    }
    // BridgeRuntimeError are allowed to be thrown 
    catch (System::InvalidCastException* )
    {
        //ToDo check
        OUStringBuffer buf( 128 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_uno():") );
        buf.append( *reinterpret_cast< OUString const * >( &type->pTypeName ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] could not convert type!") );
        throw BridgeRuntimeError( buf.makeStringAndClear() );
    }
    catch (System::NullReferenceException * e)
    {
        OUStringBuffer buf(512);
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(
                             "[map_to_uno()] Illegal null reference passed!\n"));
        buf.append(mapCliString(e->get_StackTrace()));
        throw BridgeRuntimeError( buf.makeStringAndClear() );                
    }
    catch (BridgeRuntimeError& )
    {
        throw;
    }
    catch (...)
    {
        OSL_ASSERT(0);
        throw;
    }

}

/** 
   @param info
       The expected target type. Currently info is provdided when this method is called
       to convert the in/out and out parameters of a call from cli to uno. Then info
       is always a byref type, e.g. "System.String&". info is used for Any and Enum conversion.
   @param bDontCreateObj
       false - a new object is created which holds the mapped uno value and is assigned to
       cli_data.
       true - cli_data already contains the newly constructed object. This is the case if
       a struct is converted then on the first call to map_to_cli the new object is created.
       If the struct inherits another struct then this function is called recursivly while the
       newly created object is passed in cli_data.
 */
void Bridge::map_to_cli(
    System::Object* *cli_data, void const * uno_data,
    typelib_TypeDescriptionReference * type, System::Type* info,
    bool bDontCreateObj) const
{
    switch (type->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        *cli_data= __box(*(__wchar_t const*)uno_data);
        break;
    case typelib_TypeClass_BOOLEAN:
        *cli_data = __box((*(bool const*)uno_data) == sal_True ? true : false);
        break;
    case typelib_TypeClass_BYTE:
        *cli_data = __box(*(unsigned char const*) uno_data);
        break;
    case typelib_TypeClass_SHORT:
        *cli_data= __box(*(short const*) uno_data);
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        *cli_data= __box(*(unsigned short const*) uno_data);
        break;
    case typelib_TypeClass_LONG:
        *cli_data= __box(*(int const*) uno_data);
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        *cli_data= __box(*(unsigned int const*) uno_data);
        break;
    case typelib_TypeClass_HYPER:
        *cli_data= __box(*(__int64 const*) uno_data);
        break;
    case typelib_TypeClass_UNSIGNED_HYPER:
        *cli_data= __box(*(unsigned __int64 const*) uno_data);
        break;
    case typelib_TypeClass_FLOAT:
        *cli_data= __box(*(float const*) uno_data);
        break;
    case typelib_TypeClass_DOUBLE:
        *cli_data= __box(*(double const*) uno_data);
        break;
    case typelib_TypeClass_STRING:
    {
        rtl_uString const* sVal= NULL;
        sVal= *(rtl_uString* const*) uno_data;
        *cli_data= new System::String((__wchar_t*) sVal->buffer,0, sVal->length);
        break;
    }
    case typelib_TypeClass_TYPE:
     {
         *cli_data= mapUnoType( *(typelib_TypeDescriptionReference * const *)uno_data );
         break;
     }
    case typelib_TypeClass_ANY:
    {
        uno_Any const * pAny = (uno_Any const *)uno_data;
        if (typelib_TypeClass_VOID != pAny->pType->eTypeClass)
        {
            System::Object* objCli= NULL;
            map_to_cli(
                &objCli, pAny->pData, pAny->pType, 0,
                false);
            
            uno::Any anyVal(mapUnoType(pAny->pType), objCli);
            *cli_data= __box(anyVal);
        }
        else
        { // void any
            *cli_data= __box(uno::Any::VOID);
        }
        break;
    }
    case typelib_TypeClass_ENUM:
     {
         if (info != NULL)
         {
             OSL_ASSERT(info->get_IsByRef());
             info= info->GetElementType();
             *cli_data= System::Enum::ToObject(info, *(System::Int32*) uno_data);
         }
         else
             *cli_data= System::Enum::ToObject(
                 mapUnoType(type), *(System::Int32*) uno_data);
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        TypeDescr td( type );
        typelib_CompoundTypeDescription * comp_td =
            (typelib_CompoundTypeDescription *) td.get();
        if ( ! ((typelib_TypeDescription*) comp_td)->bComplete)
                ::typelib_typedescription_complete(
                    (typelib_TypeDescription**) & comp_td );

        
        //create the type
        System::Type* cliType= loadCliType(td.get()->pTypeName);
        //detect if we recursivly convert inherited structures
        //If this point is reached because of a recursive call during convering a
        //struct then we must not create a new object rather we use the one in
        // cli_data argument. 
        System::Object* cliObj;
        if (bDontCreateObj)
            cliObj = *cli_data; // recursive call
        else
        {
            //Special handling for Exception conversion. We must call constructor System::Exception
            //to pass the message string
            if (__typeof(ucss::uno::Exception)->IsAssignableFrom(cliType))
            {
                //We need to get the Message field. Therefore we must obtain the offset from
                //the typedescription. The base interface of all exceptions is
                //com::sun::star::uno::Exception which contains the message
                typelib_CompoundTypeDescription* pCTD = comp_td;
                while (pCTD->pBaseTypeDescription)
                    pCTD = pCTD->pBaseTypeDescription;
                int nPos = -1;

				rtl::OUString usMessageMember(RTL_CONSTASCII_USTRINGPARAM("Message"));
                for (int i = 0; i < pCTD->nMembers; i ++)
                {
#if OSL_DEBUG_LEVEL >= 2
                    System::String* sMember;
                    sMember = mapUnoString(pCTD->ppMemberNames[i]);
#endif                    
                    if (usMessageMember.equals(pCTD->ppMemberNames[i]))
                    {
                        nPos = i;
                        break;
                    }
                }
                OSL_ASSERT (nPos != -1);
                int offset = pCTD->pMemberOffsets[nPos];
                //Whith the offset within the exception we can get the message string
                System::String* sMessage = mapUnoString(*(rtl_uString**)
                                                        ((char*) uno_data + offset));
                //We need to find a constructor for the exception that takes the message string
                //We assume that the first argument is the message string
                sr::ConstructorInfo* arCtorInfo[] = cliType->GetConstructors();
                sr::ConstructorInfo* ctorInfo = NULL;
                int numCtors = arCtorInfo->get_Length();
                //Constructor must at least have 2 params for the base
                //unoidl.com.sun.star.uno.Exception (String, Object);
                sr::ParameterInfo * arParamInfo[];
                for (int i = 0; i < numCtors; i++)
                {
                    arParamInfo = arCtorInfo[i]->GetParameters();
                    if (arParamInfo->get_Length() < 2)
                        continue;
                    ctorInfo = arCtorInfo[i];
                    break;
                }
                OSL_ASSERT(arParamInfo[0]->get_ParameterType()->Equals(__typeof(System::String))
                    && arParamInfo[1]->get_ParameterType()->Equals(__typeof(System::Object))
                    && arParamInfo[0]->get_Position() == 0
                    && arParamInfo[1]->get_Position() == 1);
                //Prepare parameters for constructor
                int numArgs = arParamInfo->get_Length();
                System::Object * args[] = new System::Object*[numArgs];
                //only initialize the first argument with the message
                args[0] = sMessage;
                cliObj = ctorInfo->Invoke(args);
            }
            else
                cliObj = System::Activator::CreateInstance(cliType);
        }
        sal_Int32 * pMemberOffsets = comp_td->pMemberOffsets;
        
        if (comp_td->pBaseTypeDescription)
        {
            //convert inherited struct
            //cliObj is passed inout (args in_param, out_param are true), hence the passed
            // cliObj is used by the callee instead of a newly created struct
            map_to_cli(
                &cliObj, uno_data,
                ((typelib_TypeDescription *)comp_td->pBaseTypeDescription)->pWeakRef, 0,
                true);
        }
		rtl::OUString usUnoException(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception"));
        for (sal_Int32 nPos = comp_td->nMembers; nPos--; )
        {
            typelib_TypeDescriptionReference * member_type = comp_td->ppTypeRefs[ nPos ];
            System::String* sMemberName= mapUnoString(comp_td->ppMemberNames[nPos]);
            sr::FieldInfo* aField= cliType->GetField(sMemberName);
            // special case for Exception.Message. The field has already been
            // set while constructing cli object
            if ( ! aField && usUnoException.equals(td.get()->pTypeName))
            {
                continue;
            }       
            void const * p = (char const *)uno_data + pMemberOffsets[ nPos ];
            switch (member_type->eTypeClass)
            {
            case typelib_TypeClass_CHAR:
                aField->SetValue(cliObj, __box(*(System::Char*) p));
                break;
            case typelib_TypeClass_BOOLEAN:
                aField->SetValue(cliObj, __box(*(System::Boolean*) p));
                break;
            case typelib_TypeClass_BYTE:
                aField->SetValue(cliObj, __box(*(System::Byte*) p));
                break;
            case typelib_TypeClass_SHORT:
                aField->SetValue(cliObj, __box(*(System::Int16*) p));
                break;
            case typelib_TypeClass_UNSIGNED_SHORT:
                aField->SetValue(cliObj, __box(*(System::UInt16*) p));
                break;
            case typelib_TypeClass_LONG:
                aField->SetValue(cliObj, __box(*(System::Int32*) p));
                break;
            case typelib_TypeClass_UNSIGNED_LONG:
                aField->SetValue(cliObj, __box(*(System::UInt32*) p));
                break;
            case typelib_TypeClass_HYPER:
                aField->SetValue(cliObj, __box(*(System::Int64*) p));
                break;
            case typelib_TypeClass_UNSIGNED_HYPER:
                aField->SetValue(cliObj, __box(*(System::UInt64*) p));
                break;
            case typelib_TypeClass_FLOAT:
                aField->SetValue(cliObj, __box(*(System::Single*) p));
                break;
            case typelib_TypeClass_DOUBLE:
                aField->SetValue(cliObj, __box(*(System::Double*) p));
                break;
            default:
            {
                System::Object* cli_val;
                map_to_cli(
                    &cli_val, p, member_type, 0,
                    false);
                aField->SetValue(cliObj, cli_val);
                break;
            }
            }
        }
        *cli_data= cliObj;
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        sal_Int32 nElements;
        uno_Sequence const * seq = 0;
        seq = *(uno_Sequence * const *)uno_data;
        nElements = seq->nElements;
        
        TypeDescr td( type );
        typelib_TypeDescriptionReference * element_type =
            ((typelib_IndirectTypeDescription *)td.get())->pType;
        
        switch (element_type->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
        {
            System::Char arChar[]= new System::Char[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arChar, 0, nElements);
            *cli_data= arChar;
            break;
        }
        case typelib_TypeClass_BOOLEAN:
        {
            System::Boolean arBool[]= new System::Boolean[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arBool, 0, nElements);
            *cli_data= arBool;
            break;
        }
        case typelib_TypeClass_BYTE:
        {
            System::Byte arByte[]= new System::Byte[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arByte, 0, nElements);
            *cli_data= arByte;
            break;
        }
        case typelib_TypeClass_SHORT:
        {
            System::Int16 arShort[]= new System::Int16[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arShort, 0, nElements);
            *cli_data= arShort;
            break;
        }
        case typelib_TypeClass_UNSIGNED_SHORT:
        {
            System::UInt16 arUInt16[]= new System::UInt16[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, static_cast<System::Int16[]>(arUInt16),
                                0, nElements);
            *cli_data= arUInt16;
            break;
        }
        case typelib_TypeClass_LONG:
        {
            System::Int32 arInt32[]= new System::Int32[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arInt32, 0, nElements);
            *cli_data= arInt32;
            break;
        }
        case typelib_TypeClass_UNSIGNED_LONG:
        {
            System::UInt32 arUInt32[]= new System::UInt32[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, static_cast<System::Int32[]>(arUInt32),
                                0, nElements);
            *cli_data= arUInt32;
            break;
        }
        case typelib_TypeClass_HYPER:
        {
            System::Int64 arInt64[]= new System::Int64[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arInt64, 0, nElements);
            *cli_data= arInt64;
            break;
        }
        case typelib_TypeClass_UNSIGNED_HYPER:
        {
            System::UInt64 arUInt64[]= new System::UInt64[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arUInt64, 0, nElements);
            *cli_data= arUInt64;
            break;
        }
        case typelib_TypeClass_FLOAT:
        {
            System::Single arSingle[]= new System::Single[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arSingle, 0, nElements);
            *cli_data= arSingle;
            break;
        }
        case typelib_TypeClass_DOUBLE:
        {
            System::Double arDouble[]= new System::Double[nElements];
            sri::Marshal::Copy( (void*) &seq->elements, arDouble, 0, nElements);
            *cli_data= arDouble;
            break;
        }
        case typelib_TypeClass_STRING:
        {
            System::String* arString[]= new System::String*[nElements];
            for (int i= 0; i < nElements; i++)
            {
                rtl_uString *aStr= ((rtl_uString**)(&seq->elements))[i];
                arString[i]= new System::String( (__wchar_t *) &aStr->buffer, 0, aStr->length);
            }
            *cli_data= arString;
            break;
        }
        case typelib_TypeClass_TYPE:
        {
            System::Type* arType[]= new System::Type*[nElements];
            for (int i= 0; i < nElements; i++)
            {
                arType[i]=
                    mapUnoType( ((typelib_TypeDescriptionReference**) seq->elements)[i]);
            }
            *cli_data= arType;
            break;
        }
        case typelib_TypeClass_ANY:
        {
            uno::Any arCli[]= new uno::Any[nElements];
            uno_Any const * p = (uno_Any const *)seq->elements;
            for (sal_Int32 nPos = 0; nPos < nElements; ++nPos )
            {
				System::Object* cli_obj = NULL;
                map_to_cli(
                    &cli_obj, &p[ nPos ], element_type, 0, false);
                arCli[nPos]= *__try_cast<__box uno::Any*>(cli_obj);
            }
            *cli_data= arCli;
            break;
        }
        case typelib_TypeClass_ENUM:
        {
            //get the Enum type
            System::Type* enumType= NULL;
            if (info != NULL)
            {
                //info is EnumType[]&, remove &
                OSL_ASSERT(info->IsByRef);
                enumType = info->GetElementType();
                //enumType is EnumType[], remove []
                enumType = enumType->GetElementType();
            }
            else
                enumType= mapUnoType(element_type);

            System::Array* arEnum = System::Array::CreateInstance(
                enumType, nElements);
            for (int i= 0; i < nElements; i++)
            {
                arEnum->SetValue(System::Enum::ToObject(enumType,
                   ((sal_Int32*) seq->elements)[i]), i);
            }
            *cli_data = arEnum;
            break;
        }
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            TypeDescr element_td( element_type );
            System::Array* ar= System::Array::CreateInstance(
                mapUnoType(element_type),nElements);
            if (0 < nElements)
            {
                // ToDo check this
                char * p = (char *) &seq->elements;
                sal_Int32 nSize = element_td.get()->nSize;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                { 
                    System::Object* val;
                    map_to_cli(
                        &val, p + (nSize * nPos), element_type, 0, false);
                    ar->SetValue(val, nPos);
                }
            }
			*cli_data = ar;
            break;
        }
// ToDo, verify
        case typelib_TypeClass_SEQUENCE:
        {
            System::Array *ar= System::Array::CreateInstance(
                mapUnoType(element_type), nElements);
            if (0 < nElements)
            {
                TypeDescr element_td( element_type );
                uno_Sequence ** elements = (uno_Sequence**) seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    System::Object* val;
                    map_to_cli(
                        &val, &elements[nPos], element_type, 0, false);
                    ar->SetValue(val, nPos);
                }
            }
            *cli_data = ar;
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            TypeDescr element_td( element_type );
            System::Type * ifaceType= mapUnoType(element_type);
            System::Array* ar= System::Array::CreateInstance(ifaceType, nElements);

            char * p = (char *)seq->elements;
            sal_Int32 nSize = element_td.get()->nSize;
            for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
            { 
                System::Object* val;
                map_to_cli(
                    &val, p + (nSize * nPos), element_type, NULL, false);
                   
                ar->SetValue(val, nPos);
            }
            *cli_data= ar;
            break;
        }
        default:
        {
            OUStringBuffer buf( 128 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_cli():") );
            buf.append( *reinterpret_cast< OUString const * >( &type->pTypeName ) );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] unsupported element type: ") );
            buf.append( *reinterpret_cast< OUString const * >( &element_type->pTypeName ) );
            throw BridgeRuntimeError( buf.makeStringAndClear() );
        }
        }
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        uno_Interface * pUnoI = *(uno_Interface * const *)uno_data;
        if (0 != pUnoI)
        {
            TypeDescr td( type );
            *cli_data= map_uno2cli( pUnoI, reinterpret_cast<
                                          typelib_InterfaceTypeDescription*>(td.get())) ;
        }
		else
			*cli_data= NULL;
        break;
    }
    default:
    {
        //ToDo check this exception. The String is probably crippled
        OUStringBuffer buf( 128 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[map_to_cli():") );
        buf.append( *reinterpret_cast< OUString const * >( &type->pTypeName ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] unsupported type!") );
        throw BridgeRuntimeError( buf.makeStringAndClear() );        
    }
    }
}
}
