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
#include "precompiled_sd.hxx"

#include "undo/undoobjects.hxx"
#include "sdpage.hxx"
#include "CustomAnimationEffect.hxx"
#include "drawdoc.hxx"
#include "undoanim.hxx"

using namespace sd;

///////////////////////////////////////////////////////////////////////

UndoRemovePresObjectImpl::UndoRemovePresObjectImpl( SdrObject& rObject )
: mpUndoUsercall(0)
, mpUndoAnimation(0)
, mpUndoPresObj(0)
{
	SdPage* pPage = dynamic_cast< SdPage* >( rObject.GetPage() );
	if( pPage )
	{
		if( pPage->IsPresObj(&rObject) )
			mpUndoPresObj = new UndoObjectPresentationKind( rObject );
		if( rObject.GetUserCall() )
			mpUndoUsercall = new UndoObjectUserCall(rObject);

		if( pPage->hasAnimationNode() )
		{
			com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape( rObject.getUnoShape(), com::sun::star::uno::UNO_QUERY );
			if( pPage->getMainSequence()->hasEffect( xShape ) )
			{
				mpUndoAnimation = new UndoAnimation( static_cast< SdDrawDocument* >( pPage->GetModel() ), pPage );
			}
		}
	}
}

//---------------------------------------------------------------------

UndoRemovePresObjectImpl::~UndoRemovePresObjectImpl()
{
	delete mpUndoAnimation;
	delete mpUndoPresObj;
	delete mpUndoUsercall;
}

//---------------------------------------------------------------------

void UndoRemovePresObjectImpl::Undo()
{
	if( mpUndoUsercall )
		mpUndoUsercall->Undo();
	if( mpUndoPresObj )
		mpUndoPresObj->Undo();
	if( mpUndoAnimation )
		mpUndoAnimation->Undo();
}

//---------------------------------------------------------------------

void UndoRemovePresObjectImpl::Redo()
{
	if( mpUndoAnimation )
		mpUndoAnimation->Redo();
	if( mpUndoPresObj )
		mpUndoPresObj->Redo();
	if( mpUndoUsercall )
		mpUndoUsercall->Redo();
}

///////////////////////////////////////////////////////////////////////


UndoRemoveObject::UndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
: SdrUndoRemoveObj( rObject, bOrdNumDirect ), UndoRemovePresObjectImpl( rObject )
, mxSdrObject(&rObject)
{
}

//---------------------------------------------------------------------

void UndoRemoveObject::Undo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoRemoveObject::Undo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		SdrUndoRemoveObj::Undo();
		UndoRemovePresObjectImpl::Undo();
	}
}

//---------------------------------------------------------------------

void UndoRemoveObject::Redo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoRemoveObject::Redo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		UndoRemovePresObjectImpl::Redo();
		SdrUndoRemoveObj::Redo();
	}
}

///////////////////////////////////////////////////////////////////////

UndoDeleteObject::UndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect )
: SdrUndoDelObj( rObject, bOrdNumDirect )
, UndoRemovePresObjectImpl( rObject )
, mxSdrObject(&rObject)
{
}

//---------------------------------------------------------------------

void UndoDeleteObject::Undo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoDeleteObject::Undo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		SdrUndoDelObj::Undo();
		UndoRemovePresObjectImpl::Undo();
	}
}

//---------------------------------------------------------------------

void UndoDeleteObject::Redo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoDeleteObject::Redo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		UndoRemovePresObjectImpl::Redo();
		SdrUndoDelObj::Redo();
	}
}

///////////////////////////////////////////////////////////////////////

UndoReplaceObject::UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
: SdrUndoReplaceObj( rOldObject, rNewObject, bOrdNumDirect )
, UndoRemovePresObjectImpl( rOldObject )
, mxSdrObject( &rOldObject )
{
}

//---------------------------------------------------------------------

void UndoReplaceObject::Undo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoReplaceObject::Undo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		SdrUndoReplaceObj::Undo();
		UndoRemovePresObjectImpl::Undo();
	}
}

//---------------------------------------------------------------------

void UndoReplaceObject::Redo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoReplaceObject::Redo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		UndoRemovePresObjectImpl::Redo();
		SdrUndoReplaceObj::Redo();
	}
}

///////////////////////////////////////////////////////////////////////

UndoObjectSetText::UndoObjectSetText( SdrObject& rObject, sal_Int32 nText )
: SdrUndoObjSetText( rObject, nText )
, mpUndoAnimation(0)
, mbNewEmptyPresObj(false)
, mxSdrObject( &rObject )
{
	SdPage* pPage = dynamic_cast< SdPage* >( rObject.GetPage() );
	if( pPage && pPage->hasAnimationNode() )
	{
		com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape( rObject.getUnoShape(), com::sun::star::uno::UNO_QUERY );
		if( pPage->getMainSequence()->hasEffect( xShape ) )
		{
			mpUndoAnimation = new UndoAnimation( static_cast< SdDrawDocument* >( pPage->GetModel() ), pPage );
		}
	}
}

//---------------------------------------------------------------------

UndoObjectSetText::~UndoObjectSetText()
{
	delete mpUndoAnimation;
}

//---------------------------------------------------------------------

void UndoObjectSetText::Undo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoObjectSetText::Undo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		mbNewEmptyPresObj = mxSdrObject->IsEmptyPresObj() ? true : false;
		SdrUndoObjSetText::Undo();
		if( mpUndoAnimation )
			mpUndoAnimation->Undo();
	}
}

//---------------------------------------------------------------------

void UndoObjectSetText::Redo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoObjectSetText::Redo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		if( mpUndoAnimation )
			mpUndoAnimation->Redo();
		SdrUndoObjSetText::Redo();
		mxSdrObject->SetEmptyPresObj(mbNewEmptyPresObj ? sal_True : sal_False );
	}
}

//////////////////////////////////////////////////////////////////////////////
// Undo for SdrObject::SetUserCall()

UndoObjectUserCall::UndoObjectUserCall(SdrObject& rObject)
:	SdrUndoObj(rObject)
,	mpOldUserCall((SdPage*)rObject.GetUserCall())
,	mpNewUserCall(0)
,	mxSdrObject( &rObject )
{
}

//---------------------------------------------------------------------

void UndoObjectUserCall::Undo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoObjectUserCall::Undo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		mpNewUserCall = mxSdrObject->GetUserCall();
		mxSdrObject->SetUserCall(mpOldUserCall);
	}
}

//---------------------------------------------------------------------

void UndoObjectUserCall::Redo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoObjectUserCall::Redo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		mxSdrObject->SetUserCall(mpNewUserCall);
	}
}

//////////////////////////////////////////////////////////////////////////////
// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

UndoObjectPresentationKind::UndoObjectPresentationKind(SdrObject& rObject)
:	SdrUndoObj(rObject)
,	meOldKind(PRESOBJ_NONE)
,	meNewKind(PRESOBJ_NONE)
,	mxPage( rObject.GetPage() )
,	mxSdrObject( &rObject )
{
	DBG_ASSERT( mxPage.is(), "sd::UndoObjectPresentationKind::UndoObjectPresentationKind(), does not work for shapes without a slide!" );

	if( mxPage.is() )
		meOldKind = static_cast< SdPage* >( mxPage.get() )->GetPresObjKind( &rObject );
}

//---------------------------------------------------------------------

void UndoObjectPresentationKind::Undo()
{
	if( mxPage.is() && mxSdrObject.is() )
	{
		SdPage* pPage = static_cast< SdPage* >( mxPage.get() );
		meNewKind =  pPage->GetPresObjKind( mxSdrObject.get() );
		if( meNewKind != PRESOBJ_NONE )
			pPage->RemovePresObj( mxSdrObject.get() );
		if( meOldKind != PRESOBJ_NONE )
			pPage->InsertPresObj( mxSdrObject.get(), meOldKind );
	}
}

//---------------------------------------------------------------------

void UndoObjectPresentationKind::Redo()
{
	if( mxPage.is() && mxSdrObject.is() )
	{
		SdPage* pPage = static_cast< SdPage* >( mxPage.get() );
		if( meOldKind != PRESOBJ_NONE )
			pPage->RemovePresObj( mxSdrObject.get() );
		if( meNewKind != PRESOBJ_NONE )
			pPage->InsertPresObj( mxSdrObject.get(), meNewKind );
	}
}

//////////////////////////////////////////////////////////////////////////////

UndoAutoLayoutPosAndSize::UndoAutoLayoutPosAndSize( SdPage& rPage )
: mxPage( &rPage )
{
}

//---------------------------------------------------------------------

void UndoAutoLayoutPosAndSize::Undo()
{
	// do nothing
}

//---------------------------------------------------------------------

void UndoAutoLayoutPosAndSize::Redo()
{
	SdPage* pPage = static_cast< SdPage* >( mxPage.get() );
	if( pPage )
		pPage->SetAutoLayout( pPage->GetAutoLayout(), sal_False, sal_False );
}

//////////////////////////////////////////////////////////////////////////////

UndoGeoObject::UndoGeoObject( SdrObject& rNewObj )
: SdrUndoGeoObj( rNewObj )
, mxPage( rNewObj.GetPage() )
, mxSdrObject( &rNewObj )
{
}

//---------------------------------------------------------------------

void UndoGeoObject::Undo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoGeoObject::Undo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		if( mxPage.is() )
		{
			ScopeLockGuard aGuard( static_cast< SdPage* >( mxPage.get() )->maLockAutoLayoutArrangement );
			SdrUndoGeoObj::Undo();
		}
		else
		{
			SdrUndoGeoObj::Undo();
		}
	}
}

//---------------------------------------------------------------------

void UndoGeoObject::Redo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoGeoObject::Redo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		if( mxPage.is() )
		{
			ScopeLockGuard aGuard( static_cast< SdPage* >(mxPage.get())->maLockAutoLayoutArrangement );
			SdrUndoGeoObj::Redo();
		}
		else
		{
			SdrUndoGeoObj::Redo();
		}
	}
}

//---------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////

UndoAttrObject::UndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
: SdrUndoAttrObj( rObject, bStyleSheet1 ? sal_True : sal_False, bSaveText ? sal_True : sal_False )
, mxPage( rObject.GetPage() )
, mxSdrObject( &rObject )
{
}

//---------------------------------------------------------------------

void UndoAttrObject::Undo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoAttrObject::Undo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		if( mxPage.is() )
		{
			ScopeLockGuard aGuard( static_cast< SdPage* >( mxPage.get() )->maLockAutoLayoutArrangement );
			SdrUndoAttrObj::Undo();
		}
		else
		{
			SdrUndoAttrObj::Undo();
		}
	}
}

//---------------------------------------------------------------------

void UndoAttrObject::Redo()
{
	DBG_ASSERT( mxSdrObject.is(), "sd::UndoAttrObject::Redo(), object already dead!" );
	if( mxSdrObject.is() )
	{
		if( mxPage.is() )
		{
			ScopeLockGuard aGuard( static_cast< SdPage* >( mxPage.get() )->maLockAutoLayoutArrangement );
			SdrUndoAttrObj::Redo();
		}
		else
		{
			SdrUndoAttrObj::Redo();
		}
	}
}
