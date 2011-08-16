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
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/AnimationEndSync.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationValueType.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.hxx>
#endif

#include <vcl/vclenum.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <pptexanimations.hxx>
#include <osl/endian.h>

#include <algorithm>

using ::std::map;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::container::XChild;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::lang::XMultiServiceFactory;

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

namespace ppt
{

void ImplTranslateAttribute( rtl::OUString& rString, const TranslateMode eTranslateMode )
{
	if ( eTranslateMode != TRANSLATE_NONE )
	{
		if ( ( eTranslateMode & TRANSLATE_VALUE ) || ( eTranslateMode & TRANSLATE_ATTRIBUTE ) )
		{
			const ImplAttributeNameConversion* p = gImplConversionList;
			while( p->mpAPIName )
			{
				if( rString.compareToAscii( p->mpAPIName ) == 0 )
					break;
				p++;
			}
			if( p->mpMSName )
			{
				if ( eTranslateMode & TRANSLATE_VALUE )
				{
					rString = rtl::OUString( (sal_Unicode)'#' );
					rString += OUString::createFromAscii( p->mpMSName );
				}
				else
					rString = OUString::createFromAscii( p->mpMSName );
			}
		}
		else if ( eTranslateMode & TRANSLATE_MEASURE )
		{
			const sal_Char* pDest[] = { "#ppt_x", "#ppt_y", "#ppt_w", "#ppt_h", NULL };
			const sal_Char* pSource[] = { "x", "y", "width", "height", NULL };
			sal_Int32 nIndex = 0;

			const sal_Char** ps = pSource;
			const sal_Char** pd = pDest;

			while( *ps )
			{
				const OUString aSearch( OUString::createFromAscii( *ps ) );
				while( (nIndex = rString.indexOf( aSearch, nIndex )) != -1  )
				{
					sal_Int32 nLength = aSearch.getLength();
					if( nIndex && (rString.getStr()[nIndex-1] == '#' ) )
					{
						nIndex--;
						nLength++;
					}

					const OUString aNew( OUString::createFromAscii( *pd ) );
					rString = rString.replaceAt( nIndex, nLength, aNew );
					nIndex += aNew.getLength();
				}
				ps++;
				pd++;
			}
		}
	}
}

sal_uInt32 ImplTranslatePresetSubType( const sal_uInt32 nPresetClass, const sal_uInt32 nPresetId, const rtl::OUString& rPresetSubType )
{
	sal_uInt32	nPresetSubType = 0;
	sal_Bool	bTranslated = sal_False;

	if ( ( nPresetClass == (sal_uInt32)EffectPresetClass::ENTRANCE ) || ( nPresetClass == (sal_uInt32)EffectPresetClass::EXIT ) )
	{
		if ( nPresetId != 21 )
		{
			switch( nPresetId )
			{
				case 5 :
				{
					if ( rPresetSubType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "downward" ) ) )
					{
						nPresetSubType = 5;
						bTranslated = sal_True;
					}
					else if ( rPresetSubType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "across" ) ) )
					{
						nPresetSubType = 10;
						bTranslated = sal_True;
					}
				}
				break;
				case 17 :
				{
					if ( rPresetSubType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "across" ) ) )
					{
						nPresetSubType = 10;
						bTranslated = sal_True;
					}
				}
				break;
				case 18 :
				{
					if ( rPresetSubType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "right-to-top" ) ) )
					{
						nPresetSubType = 3;
						bTranslated = sal_True;
					}
					else if ( rPresetSubType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "right-to-bottom" ) ) )
					{
						nPresetSubType = 6;
						bTranslated = sal_True;
					}
					else if ( rPresetSubType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "left-to-top" ) ) )
					{
						nPresetSubType = 9;
						bTranslated = sal_True;
					}
					else if ( rPresetSubType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "left-to-bottom" ) ) )
					{
						nPresetSubType = 12;
						bTranslated = sal_True;
					}
				}
				break;
			}
		}
		if ( !bTranslated )
		{
			const convert_subtype* p = gConvertArray;
			while( p->mpStrSubType )
			{
				if ( rPresetSubType.equalsAscii( p->mpStrSubType ) )
				{
					nPresetSubType = p->mnID;
					bTranslated = sal_True;
					break;
				}
				p++;
			}
		}
	}
	if ( !bTranslated )
		nPresetSubType = (sal_uInt32)rPresetSubType.toInt32();
	return nPresetSubType;
}

const sal_Char* transition::find( const sal_Int16 nType, const sal_Int16 nSubType, const sal_Bool bDirection )
{
	const sal_Char* pRet = NULL;
	int				nFit = 0;

	const transition* p = gTransitions;
	while( p->mpName )
	{
		int nF = 0;
		if ( nType == p->mnType )
			nF += 4;
		if ( nSubType == p->mnSubType )
			nF += 2;
		if ( bDirection == p->mbDirection )
			nF += 1;
		if ( nF > nFit )
		{
			pRet = p->mpName;
			nFit = nF;
		}
		if ( nFit == 7 )	// maximum
			break;
		p++;
	}
	return pRet;
}

SvStream& operator<<(SvStream& rOut, AnimationNode& rNode )
{
	rOut << rNode.mnU1;
	rOut << rNode.mnRestart;
	rOut << rNode.mnGroupType;
	rOut << rNode.mnFill;
	rOut << rNode.mnU3;
	rOut << rNode.mnU4;
	rOut << rNode.mnDuration;
	rOut << rNode.mnNodeType;

	return rOut;
}

AnimationExporter::AnimationExporter( const EscherSolverContainer& rSolverContainer, ppt::ExSoundCollection& rExSoundCollection ) :
	mrSolverContainer	( rSolverContainer ), 
	mrExSoundCollection ( rExSoundCollection ),
	mnCurrentGroup(0)
{
}

// --------------------------------------------------------------------

static sal_Int16 GetFillMode( const Reference< XAnimationNode >& xNode, const sal_Int16 nFillDefault )
{
    sal_Int16 nFill = xNode->getFill();
	if ( ( nFill == AnimationFill::DEFAULT ) ||
		( nFill == AnimationFill::INHERIT ) )
	{
		if ( nFill != AnimationFill::AUTO )
			nFill = nFillDefault;
	}
    if( nFill == AnimationFill::AUTO )
    {
		nFill = AnimationFill::REMOVE;
		sal_Bool bIsIndefiniteTiming = sal_True;
		Any aAny = xNode->getDuration();
        if( aAny.hasValue() )
		{
			Timing eTiming;
			if( aAny >>= eTiming )
				bIsIndefiniteTiming = eTiming == Timing_INDEFINITE;
		}
		if ( bIsIndefiniteTiming )
		{
			aAny = xNode->getEnd();
			if( aAny.hasValue() )
			{
				Timing eTiming;
				if( aAny >>= eTiming )
					bIsIndefiniteTiming = eTiming == Timing_INDEFINITE;
			}
			if ( bIsIndefiniteTiming )
			{
				if ( !xNode->getRepeatCount().hasValue() )
				{
					aAny = xNode->getRepeatDuration();
					if( aAny.hasValue() )
					{
						Timing eTiming;
						if( aAny >>= eTiming )
							bIsIndefiniteTiming = eTiming == Timing_INDEFINITE;
					}
					if ( bIsIndefiniteTiming )
						nFill = AnimationFill::FREEZE;
				}
			}
		}
	}
    return nFill;
}

void AnimationExporter::doexport( const Reference< XDrawPage >& xPage, SvStream& rStrm )
{
	Reference< XAnimationNodeSupplier > xNodeSupplier( xPage, UNO_QUERY );
	if( xNodeSupplier.is() )
	{
		const Reference< XAnimationNode > xRootNode( xNodeSupplier->getAnimationNode() );
		if( xRootNode.is() )
		{
			processAfterEffectNodes( xRootNode );
			exportNode( rStrm, xRootNode, NULL, DFF_msofbtAnimGroup, 1, 0, sal_False, AnimationFill::AUTO );
		}
	}
}

void AnimationExporter::processAfterEffectNodes( const Reference< XAnimationNode >& xRootNode )
{
	try
	{
		Reference< XEnumerationAccess > xEnumerationAccess( xRootNode, UNO_QUERY_THROW );
		Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
		while( xEnumeration->hasMoreElements() )
		{
			Reference< XAnimationNode > xNode( xEnumeration->nextElement(), UNO_QUERY_THROW );

			Reference< XEnumerationAccess > xEnumerationAccess2( xNode, UNO_QUERY );
			if ( xEnumerationAccess2.is() )
			{
				Reference< XEnumeration > xEnumeration2( xEnumerationAccess2->createEnumeration(), UNO_QUERY_THROW );
				while( xEnumeration2->hasMoreElements() )
				{
					Reference< XAnimationNode > xChildNode( xEnumeration2->nextElement(), UNO_QUERY_THROW );

					Reference< XEnumerationAccess > xEnumerationAccess3( xChildNode, UNO_QUERY_THROW );
					Reference< XEnumeration > xEnumeration3( xEnumerationAccess3->createEnumeration(), UNO_QUERY_THROW );
					while( xEnumeration3->hasMoreElements() )
					{
						Reference< XAnimationNode > xChildNode2( xEnumeration3->nextElement(), UNO_QUERY_THROW );

						Reference< XEnumerationAccess > xEnumerationAccess4( xChildNode2, UNO_QUERY_THROW );
						Reference< XEnumeration > xEnumeration4( xEnumerationAccess4->createEnumeration(), UNO_QUERY_THROW );
						while( xEnumeration4->hasMoreElements() )
						{
							Reference< XAnimationNode > xChildNode3( xEnumeration4->nextElement(), UNO_QUERY_THROW );

							switch( xChildNode3->getType() )
							{
							// found an after effect
							case AnimationNodeType::SET:
							case AnimationNodeType::ANIMATECOLOR:
								{
									Reference< XAnimationNode > xMaster;

									Sequence< NamedValue > aUserData( xChildNode3->getUserData() );
									sal_Int32 nLength = aUserData.getLength();
									const NamedValue* p = aUserData.getConstArray();

									while( nLength-- )
									{
										if( p->Name.equalsAscii( "master-element" ) )
										{
											p->Value >>= xMaster;
											break;
										}
										p++;
									}

									AfterEffectNodePtr pAfterEffectNode( new AfterEffectNode( xChildNode3, xMaster ) );
									maAfterEffectNodes.push_back( pAfterEffectNode );
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	catch( Exception& e )
	{
		(void)e;
		DBG_ERROR( "(@CL)AnimationExporter::processAfterEffectNodes(), exception cought!" );
	}
}

bool AnimationExporter::isAfterEffectNode( const Reference< XAnimationNode >& xNode ) const
{
	std::list< AfterEffectNodePtr >::const_iterator aIter( maAfterEffectNodes.begin() );
	const std::list< AfterEffectNodePtr >::const_iterator aEnd( maAfterEffectNodes.end() );
	while( aIter != aEnd )
	{
		if( (*aIter)->mxNode == xNode )
			return true;
		aIter++;
	}

	return false;
}

bool AnimationExporter::hasAfterEffectNode( const Reference< XAnimationNode >& xNode, Reference< XAnimationNode >& xAfterEffectNode ) const
{
	std::list< AfterEffectNodePtr >::const_iterator aIter( maAfterEffectNodes.begin() );
	const std::list< AfterEffectNodePtr >::const_iterator aEnd( maAfterEffectNodes.end() );
	while( aIter != aEnd )
	{
		if( (*aIter)->mxMaster == xNode )
		{
			xAfterEffectNode = (*aIter)->mxNode;
			return true;
		}
		aIter++;
	}

	return false;
}

// check if this group only contain empty groups. this may happen when
// after effect nodes are not exported at theire original position
bool AnimationExporter::isEmptyNode( const Reference< XAnimationNode >& xNode ) const
{
	if( xNode.is() ) switch( xNode->getType() )
	{
	case AnimationNodeType::PAR :
	case AnimationNodeType::SEQ :
	case AnimationNodeType::ITERATE :
		{
			Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
			if( xEnumerationAccess.is() )
			{
				Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
				if( xEnumeration.is() )
				{
					while( xEnumeration->hasMoreElements() )
					{
						Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
						if( xChildNode.is() && !isEmptyNode( xChildNode ) )
							return false;
					}
				}
			}
		}
		break;

	case AnimationNodeType::SET :
	case AnimationNodeType::ANIMATECOLOR :
		return isAfterEffectNode( xNode );
	default:
		return false;
	}

	return true;
}

void AnimationExporter::exportNode( SvStream& rStrm, Reference< XAnimationNode > xNode, const Reference< XAnimationNode >* pParent, const sal_uInt16 nContainerRecType,
									const sal_uInt16 nInstance, const sal_Int32 nGroupLevel, const sal_Bool bTakeBackInteractiveSequenceTiming, const sal_Int16 nFDef )
{
	if( (nGroupLevel == 4) && isEmptyNode( xNode ) )
		return;

	if ( ( nContainerRecType == DFF_msofbtAnimGroup ) && ( nGroupLevel == 2 ) && isEmptyNode( xNode ) )
		return;

	if( nContainerRecType == DFF_msofbtAnimGroup )
		mnCurrentGroup++;

	sal_Bool bTakeBackInteractiveSequenceTimingForChild = sal_False;
	sal_Int16 nFillDefault = GetFillMode( xNode, nFDef );

	bool bSkipChildren = false;
	
	Reference< XAnimationNode > xAudioNode;
	static sal_uInt32 nAudioGroup;

	{
		EscherExContainer aContainer( rStrm, nContainerRecType, nInstance );
		switch( xNode->getType() )
		{
			case AnimationNodeType::CUSTOM :
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, 0 );
				exportAnimValue( rStrm, xNode, sal_False );
			}
			break;

			case AnimationNodeType::PAR :
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );
				sal_Int32 nFlags = nGroupLevel == 2 ? 0x10 : 0;
				if ( bTakeBackInteractiveSequenceTiming )
					nFlags |= 0x40;
				exportAnimEvent( rStrm, xNode, nFlags );
				exportAnimValue( rStrm, xNode, nGroupLevel == 4 );
			}
			break;

			case AnimationNodeType::SEQ :
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				sal_Int16 nNodeType = exportAnimPropertySet( rStrm, xNode );
				sal_Int32 nFlags = 12;
				if ( ( nGroupLevel == 1 ) && ( nNodeType == ::com::sun::star::presentation::EffectNodeType::INTERACTIVE_SEQUENCE ) )
				{
					nFlags |= 0x20;
					bTakeBackInteractiveSequenceTimingForChild = sal_True;
				}
				exportAnimAction( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, nFlags );
				exportAnimValue( rStrm, xNode, sal_False );
			}
			break;

			case AnimationNodeType::ITERATE :
			{
				{
					EscherExAtom aAnimNodeExAtom( rStrm, DFF_msofbtAnimNode );
					AnimationNode aAnim;
					rtl_zeroMemory( &aAnim, sizeof( aAnim ) );
					aAnim.mnGroupType = mso_Anim_GroupType_PAR;
					aAnim.mnNodeType = 1;
					// attribute Restart
					switch( xNode->getRestart() )
					{
						default:
						case AnimationRestart::DEFAULT : aAnim.mnRestart = 0; break;
						case AnimationRestart::ALWAYS  : aAnim.mnRestart = 1; break;
						case AnimationRestart::WHEN_NOT_ACTIVE : aAnim.mnRestart = 2; break;
						case AnimationRestart::NEVER : aAnim.mnRestart = 3; break;
					}
					// attribute Fill
					switch( xNode->getFill() )
					{
						default:
						case AnimationFill::DEFAULT : aAnim.mnFill = 0; break;
						case AnimationFill::REMOVE : aAnim.mnFill = 1; break;
						case AnimationFill::FREEZE : aAnim.mnFill = 2; break;
						case AnimationFill::HOLD : aAnim.mnFill = 3; break;
						case AnimationFill::TRANSITION : aAnim.mnFill = 4; break;
					}
					rStrm << aAnim;
				}
				exportIterate( rStrm, xNode );
				exportAnimPropertySet( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, 0 );
				exportAnimValue( rStrm, xNode, sal_False );

	/*
				EscherExContainer aContainer( rStrm, DFF_msofbtAnimGroup, 1 );
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel + 1, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, 0 );
				exportAnimValue( rStrm, xNode, sal_False );
	*/
			}
			break;

			case AnimationNodeType::ANIMATE :
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, 0 );
				exportAnimValue( rStrm, xNode, sal_False );
				exportAnimate( rStrm, xNode );
			}
			break;

			case AnimationNodeType::SET :
			{
				bool bIsAfterEffectNode( isAfterEffectNode( xNode ) );
				if( (nGroupLevel != 4) || !bIsAfterEffectNode )
				{
					exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
					exportAnimPropertySet( rStrm, xNode );
					exportAnimateSet( rStrm, xNode, bIsAfterEffectNode ? AFTEREFFECT_SET : AFTEREFFECT_NONE );
					exportAnimEvent( rStrm, xNode, 0 );
					exportAnimValue( rStrm, xNode, sal_False );
				}
				else
				{
					bSkipChildren = true;
				}
			}
			break;

			case AnimationNodeType::ANIMATEMOTION :
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );
				exportAnimateMotion( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, 0 );
				exportAnimValue( rStrm, xNode, sal_False );
			}
			break;

			case AnimationNodeType::ANIMATECOLOR :
			{
				bool bIsAfterEffectNode( isAfterEffectNode( xNode ) );
				if( (nGroupLevel != 4) || !bIsAfterEffectNode )
				{
					if( bIsAfterEffectNode )
						xNode = createAfterEffectNodeClone( xNode );

					exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
					exportAnimPropertySet( rStrm, xNode );
					exportAnimateColor( rStrm, xNode, bIsAfterEffectNode ? AFTEREFFECT_COLOR : AFTEREFFECT_NONE );
					exportAnimEvent( rStrm, xNode, 0 );
					exportAnimValue( rStrm, xNode, sal_False );
				}
				else
				{
					bSkipChildren = true;
				}
			}
			break;

			case AnimationNodeType::ANIMATETRANSFORM :
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );
				exportAnimateTransform( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, 0 );
				exportAnimValue( rStrm, xNode, sal_False );
			}
			break;

			case AnimationNodeType::TRANSITIONFILTER :
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );
				exportAnimEvent( rStrm, xNode, 0 );
				exportAnimValue( rStrm, xNode, sal_False );
				exportTransitionFilter( rStrm, xNode );
			}
			break;

			case AnimationNodeType::AUDIO :		// #i58428#
			{
				exportAnimNode( rStrm, xNode, pParent, nGroupLevel, nFillDefault );
				exportAnimPropertySet( rStrm, xNode );

				Reference< XAudio > xAudio( xNode, UNO_QUERY );
				if( xAudio.is() )
				{
					Any aAny( xAudio->getSource() );
					rtl::OUString aURL;

					if ( ( aAny >>= aURL ) && ( aURL.getLength() ) )
					{
						sal_Int32 nU1 = 2;
						sal_Int32 nTrigger = 3;
						sal_Int32 nU3 = nAudioGroup;
						sal_Int32 nBegin = 0;
						{
							EscherExContainer aAnimEvent( rStrm, DFF_msofbtAnimEvent, 1 );
							{
								EscherExAtom aAnimTrigger( rStrm, DFF_msofbtAnimTrigger );
								rStrm << nU1 << nTrigger << nU3 << nBegin;
							}
						}
						nU1 = 1;
						nTrigger = 0xb;
						nU3 = 0;
						{
							EscherExContainer aAnimEvent( rStrm, DFF_msofbtAnimEvent, 2 );
							{
								EscherExAtom aAnimTrigger( rStrm, DFF_msofbtAnimTrigger );
								rStrm << nU1 << nTrigger << nU3 << nBegin;
							}
						}
						EscherExContainer aAnimateTargetElement( rStrm, DFF_msofbtAnimateTargetElement );
						{
							sal_uInt32 nRefMode = 3;
							sal_uInt32 nRefType = 2;
							sal_uInt32 nRefId = mrExSoundCollection.GetId( aURL );
							sal_Int32 begin = -1;
							sal_Int32 end = -1;

							EscherExAtom aAnimReference( rStrm, DFF_msofbtAnimReference );
							rStrm << nRefMode << nRefType << nRefId << begin << end;
						}
					}
				}
				exportAnimValue( rStrm, xNode, sal_False );
			}
			break;
		}
		if( !bSkipChildren )
		{
			// export after effect node if one exists for this node
			Reference< XAnimationNode > xAfterEffectNode;
			if( hasAfterEffectNode( xNode, xAfterEffectNode ) )
			{
				exportNode( rStrm, xAfterEffectNode, &xNode, DFF_msofbtAnimSubGoup, 1, nGroupLevel + 1, bTakeBackInteractiveSequenceTimingForChild, nFillDefault );
			}

			Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
			if( xEnumerationAccess.is() )
			{
				Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
				if( xEnumeration.is() )
				{
					while( xEnumeration->hasMoreElements() )
					{
						Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
						if( xChildNode.is() )
						{
							if ( xChildNode->getType() == AnimationNodeType::AUDIO )
							{
								xAudioNode = xChildNode;
								nAudioGroup = mnCurrentGroup;
							}
							else
								exportNode( rStrm, xChildNode, &xNode, DFF_msofbtAnimGroup, 1, nGroupLevel + 1, bTakeBackInteractiveSequenceTimingForChild, nFillDefault );
						}
					}
				}
			}
		}
	}
	if ( xAudioNode.is() )
		exportNode( rStrm, xAudioNode, &xNode, DFF_msofbtAnimGroup, 1, nGroupLevel, bTakeBackInteractiveSequenceTimingForChild, nFillDefault );

	if( xNode->getType() == AnimationNodeType::ITERATE )
		aTarget = Any();
}

Reference< XAnimationNode > AnimationExporter::createAfterEffectNodeClone( const Reference< XAnimationNode >& xNode ) const
{
	try
	{
		Reference< ::com::sun::star::util::XCloneable > xClonable( xNode, UNO_QUERY_THROW );
		Reference< XAnimationNode > xCloneNode( xClonable->createClone(), UNO_QUERY_THROW );

		Any aEmpty;
		xCloneNode->setBegin( aEmpty );


		return xCloneNode;
	}
	catch( Exception& e )
	{
		(void)e;
		DBG_ERROR("(@CL)sd::ppt::AnimationExporter::createAfterEffectNodeClone(), could not create clone!" );
	}
	return xNode;
}

void AnimationExporter::exportAnimNode( SvStream& rStrm, const Reference< XAnimationNode >& xNode,
		const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >*, const sal_Int32, const sal_Int16 nFillDefault )
{
	EscherExAtom	aAnimNodeExAtom( rStrm, DFF_msofbtAnimNode );
	AnimationNode	aAnim;
	rtl_zeroMemory( &aAnim, sizeof( aAnim ) );

	// attribute Restart
	switch( xNode->getRestart() )
	{
		default:
		case AnimationRestart::DEFAULT : aAnim.mnRestart = 0; break;
		case AnimationRestart::ALWAYS  : aAnim.mnRestart = 1; break;
		case AnimationRestart::WHEN_NOT_ACTIVE : aAnim.mnRestart = 2; break;
		case AnimationRestart::NEVER : aAnim.mnRestart = 3; break;
	}

	// attribute Fill
//	aAnim.mnFill = GetFillMode( xNode, pParent );
	switch( nFillDefault )
	{
		default:
		case AnimationFill::DEFAULT : aAnim.mnFill = 0; break;
		case AnimationFill::REMOVE : aAnim.mnFill = 1; break;
		case AnimationFill::FREEZE : // aAnim.mnFill = 2; break;
		case AnimationFill::HOLD :   aAnim.mnFill = 3; break;
		case AnimationFill::TRANSITION : aAnim.mnFill = 4; break;
	}
	// attribute Duration
	double fDuration = 0.0;
	com::sun::star::animations::Timing eTiming;
	if ( xNode->getDuration() >>= eTiming )
	{
		if ( eTiming == Timing_INDEFINITE )
			aAnim.mnDuration = -1;
	}
	else if ( xNode->getDuration() >>= fDuration )
	{
		aAnim.mnDuration = (sal_Int32)( fDuration * 1000.0 );
	}
	else
		aAnim.mnDuration = -1;

	// NodeType, NodeGroup
	aAnim.mnNodeType = 1;
	aAnim.mnGroupType = mso_Anim_GroupType_SEQ;
	switch( xNode->getType() )
	{
		case AnimationNodeType::PAR :		// PASSTROUGH!!! (as it was intended)
			aAnim.mnGroupType = mso_Anim_GroupType_PAR;
		case AnimationNodeType::SEQ :
		{
			// trying to get the nodetype
			Sequence< NamedValue > aUserData = xNode->getUserData();
			if ( aUserData.getLength() )
			{
				const NamedValue* p = aUserData.getConstArray();
				sal_Int32 nLength = aUserData.getLength();
				while( nLength-- )
				{	
					if( p->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "node-type" ) ) )
					{
						sal_Int16 nType = 0;
						if ( p->Value >>= nType )
						{
							switch( nType )
							{
								case ::com::sun::star::presentation::EffectNodeType::TIMING_ROOT : aAnim.mnNodeType = 0x12; break;
								case ::com::sun::star::presentation::EffectNodeType::MAIN_SEQUENCE : aAnim.mnNodeType = 0x18; break;
		/*
								case ::com::sun::star::presentation::EffectNodeType::ON_CLICK :
								case ::com::sun::star::presentation::EffectNodeType::WITH_PREVIOUS :
								case ::com::sun::star::presentation::EffectNodeType::AFTER_PREVIOUS :
								case ::com::sun::star::presentation::EffectNodeType::INTERACTIVE_SEQUENCE :
								default:
		*/
								}
						}
						break;
					}
				}
			}
		}
		break;

		case AnimationNodeType::ANIMATE :
		case AnimationNodeType::SET :

		case AnimationNodeType::CUSTOM :
		case AnimationNodeType::ITERATE :
		case AnimationNodeType::ANIMATEMOTION :
		case AnimationNodeType::ANIMATECOLOR :
		case AnimationNodeType::ANIMATETRANSFORM :
		{
			aAnim.mnGroupType = mso_Anim_GroupType_NODE;
			aAnim.mnNodeType  = mso_Anim_Behaviour_ANIMATION;
		}
		break;

		case AnimationNodeType::AUDIO :
		{
			aAnim.mnGroupType = mso_Anim_GroupType_MEDIA;
			aAnim.mnNodeType  = mso_Anim_Behaviour_ANIMATION;
		}
		break;

		case AnimationNodeType::TRANSITIONFILTER :
		{
			aAnim.mnGroupType = mso_Anim_GroupType_NODE;
			aAnim.mnNodeType  = mso_Anim_Behaviour_FILTER;
		}
		break;
	}
	rStrm << aAnim;
}

sal_Int16 AnimationExporter::exportAnimPropertySet( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
	sal_Int16 nNodeType = ::com::sun::star::presentation::EffectNodeType::DEFAULT;

	EscherExContainer aAnimPropertySet( rStrm, DFF_msofbtAnimPropertySet );
	const ::com::sun::star::uno::Any* pAny[ DFF_ANIM_PROPERTY_ID_COUNT ];
	rtl_zeroMemory( pAny, sizeof( pAny ) );

	Reference< XAnimationNode > xMaster;

	const Any aTrue( makeAny( (sal_Bool)sal_True ) );
	Any aMasterRel, aOverride, aRunTimeContext;

	// storing user data into pAny, to allow direct access later
	Sequence< NamedValue > aUserData = xNode->getUserData();
	if ( aUserData.getLength() )
	{
		const NamedValue* p = aUserData.getConstArray();
		sal_Int32 nLength = aUserData.getLength();
		while( nLength-- )
		{	
			if( p->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "node-type" ) ) )
			{
				pAny[ DFF_ANIM_NODE_TYPE ] = &(p->Value);
			}
			else if ( p->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "preset-class" ) ) )
			{
				pAny[ DFF_ANIM_PRESET_CLASS ] = &(p->Value);
			}
			else if ( p->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "preset-id" ) ) )
			{
				pAny[ DFF_ANIM_PRESET_ID ] = &(p->Value);
			}
			else if ( p->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "preset-sub-type" ) ) )
			{
				pAny[ DFF_ANIM_PRESET_SUB_TYPE ] = &(p->Value);
			}
			else if ( p->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "master-element" ) ) )
			{
				pAny[ DFF_ANIM_AFTEREFFECT ] = &aTrue;
				p->Value >>= xMaster;
			}
			p++;
		}
	}

	// calculate master-rel
	if( xMaster.is() )
	{
		sal_Int32 nMasterRel = 2;
		Reference< XChild > xNodeChild( xNode, UNO_QUERY );
		Reference< XChild > xMasterChild( xMaster, UNO_QUERY );
		if( xNodeChild.is() && xMasterChild.is() && (xNodeChild->getParent() == xMasterChild->getParent() ) )
			nMasterRel = 0;

		aMasterRel <<= nMasterRel;

		pAny[ DFF_ANIM_MASTERREL ] = &aMasterRel;

		aOverride <<= (sal_Int32)1;
		pAny[ DFF_ANIM_OVERRIDE ] = &aOverride;

		aRunTimeContext <<= (sal_Int32)1;
		pAny[ DFF_ANIM_RUNTIMECONTEXT ] = &aRunTimeContext;
	}

	// the order is important
	if ( pAny[ DFF_ANIM_NODE_TYPE ] )
	{
		if ( *pAny[ DFF_ANIM_NODE_TYPE ] >>= nNodeType )
		{
			sal_uInt32 nPPTNodeType = DFF_ANIM_NODE_TYPE_ON_CLICK;
			switch( nNodeType )
			{
				case ::com::sun::star::presentation::EffectNodeType::ON_CLICK : nPPTNodeType = DFF_ANIM_NODE_TYPE_ON_CLICK;	break;
				case ::com::sun::star::presentation::EffectNodeType::WITH_PREVIOUS : nPPTNodeType = DFF_ANIM_NODE_TYPE_WITH_PREVIOUS; break;
				case ::com::sun::star::presentation::EffectNodeType::AFTER_PREVIOUS : nPPTNodeType = DFF_ANIM_NODE_TYPE_AFTER_PREVIOUS; break;
				case ::com::sun::star::presentation::EffectNodeType::MAIN_SEQUENCE : nPPTNodeType = DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE; break;
				case ::com::sun::star::presentation::EffectNodeType::TIMING_ROOT : nPPTNodeType = DFF_ANIM_NODE_TYPE_TIMING_ROOT; break;
				case ::com::sun::star::presentation::EffectNodeType::INTERACTIVE_SEQUENCE: nPPTNodeType = DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ; break;
			}
			exportAnimPropertyuInt32( rStrm, DFF_ANIM_NODE_TYPE, nPPTNodeType, TRANSLATE_NONE );
		}
	}
	sal_uInt32 nPresetId = 0;
    sal_uInt32 nPresetSubType = 0;
	sal_uInt32 nAPIPresetClass = EffectPresetClass::CUSTOM;
	sal_uInt32 nPresetClass = DFF_ANIM_PRESS_CLASS_USER_DEFINED;
	sal_Bool bPresetClass, bPresetId, bPresetSubType;
	bPresetClass = bPresetId = bPresetSubType = sal_False;

	if ( pAny[ DFF_ANIM_PRESET_CLASS ] )
	{
		if ( *pAny[ DFF_ANIM_PRESET_CLASS ] >>= nAPIPresetClass )
		{
			sal_uInt8 nPPTPresetClass;
			switch( nAPIPresetClass )
			{
				case EffectPresetClass::ENTRANCE : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_ENTRANCE; break;
				case EffectPresetClass::EXIT : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_EXIT; break;
				case EffectPresetClass::EMPHASIS : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_EMPHASIS; break;
				case EffectPresetClass::MOTIONPATH : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_MOTIONPATH; break;
				case EffectPresetClass::OLEACTION : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_OLE_ACTION; break;
				case EffectPresetClass::MEDIACALL : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_MEDIACALL; break;
				default :
					nPPTPresetClass = DFF_ANIM_PRESS_CLASS_USER_DEFINED;
			}
			nPresetClass = nPPTPresetClass;
			bPresetClass = sal_True;
		}
	}
	if ( pAny[ DFF_ANIM_PRESET_ID ] )
	{
		rtl::OUString sPreset;
		if ( *pAny[ DFF_ANIM_PRESET_ID ] >>= sPreset )
		{
			if ( sPreset.match( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ppt_" ) ), 0 ) )
			{
				sal_Int32 nLast = sPreset.lastIndexOf( '_' );
				if ( ( nLast != -1 ) && ( ( nLast + 1 ) < sPreset.getLength() ) )
				{
					rtl::OUString aNumber( sPreset.copy( nLast + 1 ) );
					nPresetId = aNumber.toInt32();
					bPresetId = sal_True;
				}
   			}
			else
			{
				const preset_maping* p = gPresetMaping;
				while( p->mpStrPresetId && ((p->mnPresetClass != (sal_Int32)nAPIPresetClass) || !sPreset.equalsAscii( p->mpStrPresetId )) )
					p++;

				if( p->mpStrPresetId )
				{
					nPresetId = p->mnPresetId;
					bPresetId = sal_True;
				}
			}
		}
	}

	if ( pAny[ DFF_ANIM_PRESET_SUB_TYPE ] )
	{
		rtl::OUString sPresetSubType;
		if ( *pAny[ DFF_ANIM_PRESET_SUB_TYPE ] >>= sPresetSubType )
		{
			nPresetSubType = ImplTranslatePresetSubType( nPresetClass, nPresetId, sPresetSubType );
			bPresetSubType = sal_True;
		}
	}
	if ( bPresetId )
		exportAnimPropertyuInt32( rStrm, DFF_ANIM_PRESET_ID, nPresetId, TRANSLATE_NONE );
	if ( bPresetSubType )
		exportAnimPropertyuInt32( rStrm, DFF_ANIM_PRESET_SUB_TYPE, nPresetSubType, TRANSLATE_NONE );
	if ( bPresetClass )
		exportAnimPropertyuInt32( rStrm, DFF_ANIM_PRESET_CLASS, nPresetClass, TRANSLATE_NONE );

	if ( pAny[ DFF_ANIM_ID ] )
	{
		// TODO DFF_ANIM_ID
	}

	if ( pAny[ DFF_ANIM_AFTEREFFECT ] )
	{
		sal_Bool bAfterEffect = sal_False;
		if ( *pAny[ DFF_ANIM_AFTEREFFECT ] >>= bAfterEffect )
			exportAnimPropertyByte( rStrm, DFF_ANIM_AFTEREFFECT, bAfterEffect, TRANSLATE_NONE );
	}

	if ( pAny[ DFF_ANIM_RUNTIMECONTEXT ] )
	{
		sal_Int32 nRunTimeContext = 0;
		if ( *pAny[ DFF_ANIM_RUNTIMECONTEXT ] >>= nRunTimeContext )
			exportAnimPropertyuInt32( rStrm, DFF_ANIM_RUNTIMECONTEXT, nRunTimeContext, TRANSLATE_NONE );
	}
	if ( pAny[ DFF_ANIM_PATH_EDIT_MODE ] )
	{
		// TODO DFF_ANIM_ID
	}

	if( !xMaster.is() )
	{
		Reference< XAnimateColor > xColor( xNode, UNO_QUERY );
		if( xColor.is() )
		{
//			sal_uInt32 nColorSpace = xColor->getColorSpace() == AnimationColorSpace::RGB ? 0 : 1;
//			exportAnimPropertyuInt32( rStrm, DFF_ANIM_COLORSPACE, nColorSpace, TRANSLATE_NONE );
			
			sal_Bool bDirection = !xColor->getDirection();
			exportAnimPropertyuInt32( rStrm, DFF_ANIM_DIRECTION, bDirection, TRANSLATE_NONE );
		}
	}

	if ( pAny[ DFF_ANIM_OVERRIDE ] )
	{
		sal_Int32 nOverride = 0;
		if ( *pAny[ DFF_ANIM_OVERRIDE ] >>= nOverride )
			exportAnimPropertyuInt32( rStrm, DFF_ANIM_OVERRIDE, nOverride, TRANSLATE_NONE );
	}

	if ( pAny[ DFF_ANIM_MASTERREL ] )
	{
		sal_Int32 nMasterRel = 0;
		if ( *pAny[ DFF_ANIM_MASTERREL ] >>= nMasterRel )
			exportAnimPropertyuInt32( rStrm, DFF_ANIM_MASTERREL, nMasterRel, TRANSLATE_NONE );
	}

/* todo
	Reference< XAudio > xAudio( xNode, UNO_QUERY );
	if( xAudio.is() )
	{
		sal_Int16 nEndAfterSlide = 0;
		nEndAfterSlide = xAudio->getEndAfterSlide();
		exportAnimPropertyuInt32( rStrm, DFF_ANIM_ENDAFTERSLIDE, nEndAfterSlide, TRANSLATE_NONE );
	}
*/
	Reference< XAnimate > xAnim( xNode, UNO_QUERY );
	if( xAnim.is() )
	{
		// TODO: DFF_ANIM_TIMEFILTER
	}
	if ( pAny[ DFF_ANIM_EVENT_FILTER ] )
	{
		// TODO DFF_ANIM_EVENT_FILTER 
	}
	if ( pAny[ DFF_ANIM_VOLUME ] )
	{
		// TODO DFF_ANIM_VOLUME 
	}
	return nNodeType;
}

sal_Bool AnimationExporter::exportAnimProperty( SvStream& rStrm, const sal_uInt16 nPropertyId, const ::com::sun::star::uno::Any& rAny, const TranslateMode eTranslateMode )
{
	sal_Bool bRet = sal_False;
	if ( rAny.hasValue() )
	{
		switch( rAny.getValueType().getTypeClass() )
		{
			case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT :
			case ::com::sun::star::uno::TypeClass_SHORT :
			case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG :
			case ::com::sun::star::uno::TypeClass_LONG :
			{
				sal_Int32 nVal = 0;
				if ( rAny >>= nVal )
				{
					exportAnimPropertyuInt32( rStrm, nPropertyId, nVal, eTranslateMode );
					bRet = sal_True;
				}
			}
			break;

			case ::com::sun::star::uno::TypeClass_DOUBLE :
			{
				double fVal = 0.0;
				if ( rAny >>= fVal )
				{
					exportAnimPropertyFloat( rStrm, nPropertyId, fVal, eTranslateMode );
					bRet = sal_True;
				}
			}
			break;
			case ::com::sun::star::uno::TypeClass_FLOAT :
			{
				float fVal = 0.0;
				if ( rAny >>= fVal )
				{
					if ( eTranslateMode & TRANSLATE_NUMBER_TO_STRING )
					{
						Any aAny;
						rtl::OUString aNumber( rtl::OUString::valueOf( fVal ) );
						aAny <<= aNumber;
						exportAnimPropertyString( rStrm, nPropertyId, aNumber, eTranslateMode );
					}
					else
					{
						exportAnimPropertyFloat( rStrm, nPropertyId, fVal, eTranslateMode );
						bRet = sal_True;
					}
				}
			}
			break;
			case ::com::sun::star::uno::TypeClass_STRING :
			{
				rtl::OUString aStr;
				if ( rAny >>= aStr )
				{
					exportAnimPropertyString( rStrm, nPropertyId, aStr, eTranslateMode );
					bRet = sal_True;
				}
			}
			break;
			default:
				break;
		}
	}
	return bRet;
}
void AnimationExporter::exportAnimPropertyString( SvStream& rStrm, const sal_uInt16 nPropertyId, const rtl::OUString& rVal, const TranslateMode eTranslateMode )
{
	EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
	sal_uInt8 nType = DFF_ANIM_PROP_TYPE_UNISTRING;
	rStrm << nType;
	rtl::OUString aStr( rVal );
	if ( eTranslateMode != TRANSLATE_NONE )
		ImplTranslateAttribute( aStr, eTranslateMode );
	writeZString( rStrm, aStr );
}

void AnimationExporter::exportAnimPropertyFloat( SvStream& rStrm, const sal_uInt16 nPropertyId, const double& rVal, const TranslateMode )
{
	EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
	sal_uInt8 nType = DFF_ANIM_PROP_TYPE_FLOAT;
	float fFloat = (float)rVal;
	rStrm << nType
		  << fFloat;
}

void AnimationExporter::exportAnimPropertyuInt32( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt32 nVal, const TranslateMode )
{
	EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
	sal_uInt8 nType = DFF_ANIM_PROP_TYPE_INT32 ;
	rStrm << nType
		  << nVal;
}

void AnimationExporter::exportAnimPropertyByte( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt8 nVal, const TranslateMode )
{
	EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
	sal_uInt8 nType = DFF_ANIM_PROP_TYPE_BYTE;
	rStrm << nType
		  << nVal;
}

void AnimationExporter::writeZString( SvStream& rStrm, const rtl::OUString& rVal )
{
	sal_Int32 i;
	for ( i = 0; i < rVal.getLength(); i++ )
		rStrm << rVal[ i ];
	rStrm << (sal_Unicode)0;
}

void AnimationExporter::exportAnimAction( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
	EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAction );

	sal_Int32 nConcurrent = 1;
	sal_Int32 nNextAction = 1;
	sal_Int32 nEndSync = 0;
	sal_Int32 nU4 = 0;
	sal_Int32 nU5 = 3;

	sal_Int16 nAnimationEndSync = 0;
	if ( xNode->getEndSync() >>= nAnimationEndSync )
	{
		if ( nAnimationEndSync == AnimationEndSync::ALL )
			nEndSync = 1;
	}
	rStrm << nConcurrent
		  << nNextAction
		  << nEndSync
		  << nU4
		  << nU5;

}

// nFlags Bit 6 = fixInteractiveSequenceTiming (for child)
// nFlags Bit 5 = fixInteractiveSequenceTiming (for root)
// nFlags Bit 4 = first node of main sequence -> begin event next has to be replaced to indefinite
void AnimationExporter::exportAnimEvent( SvStream& rStrm, const Reference< XAnimationNode >& xNode, const sal_Int32 nFlags )
{
	sal_uInt16 i;
	for ( i = 0; i < 4; i++ )
	{
		sal_Int32 nU1 = 0;
		sal_Int32 nTrigger = 0;
		sal_Int32 nU3 = 0;
		sal_Int32 nBegin = 0;

		sal_Bool bCreateEvent = sal_False;
		Any aSource;

		switch( i )
		{
			case 0 : 
			case 1 :
			{
				Any aAny;
				Event aEvent;
				com::sun::star::animations::Timing eTiming;
				if ( i == 0 )
				{
					if ( nFlags & 0x20 )
					{
						// taking the first child
						Reference< XEnumerationAccess > xEA( xNode, UNO_QUERY_THROW );
						Reference< XEnumeration > xE( xEA->createEnumeration(), UNO_QUERY_THROW );
						if ( xE.is() && xE->hasMoreElements() )
						{
//							while( xE->hasMoreElements() )
							{
								Reference< XAnimationNode > xClickNode( xE->nextElement(), UNO_QUERY );
								aAny = xClickNode->getBegin();
							}
						}
					}
					else if ( nFlags & 0x40 )
					{
						// begin has to be replaced with void, so don't do anything
					}
					else
					{
						aAny = xNode->getBegin();
						if ( nFlags & 0x10 )	// replace ON_NEXT with IDEFINITE
						{
							if ( ( aAny >>= aEvent ) && ( aEvent.Trigger == EventTrigger::ON_NEXT ) )
							{
								eTiming = Timing_INDEFINITE;
								aAny <<= eTiming;
							}
						}
					}
				}
				else
					aAny = xNode->getEnd();

				double fTiming = 0.0;
				if ( aAny >>= aEvent )
				{
					bCreateEvent = sal_True;
					switch( aEvent.Trigger )
					{
						case EventTrigger::NONE : nTrigger = 0; break;
						case EventTrigger::ON_BEGIN : nTrigger = 1; break;
						case EventTrigger::ON_END : nTrigger = 2; break;
						case EventTrigger::BEGIN_EVENT : nTrigger = 3; break;
						case EventTrigger::END_EVENT : nTrigger = 4; nU1 = 2; nU3 = mnCurrentGroup; break;
						case EventTrigger::ON_CLICK : nTrigger = 5; break;
						case EventTrigger::ON_DBL_CLICK : nTrigger = 6; break;
						case EventTrigger::ON_MOUSE_ENTER : nTrigger = 7; break;
						case EventTrigger::ON_MOUSE_LEAVE : nTrigger = 8; break;
						case EventTrigger::ON_NEXT : nTrigger = 9; break;
						case EventTrigger::ON_PREV : nTrigger = 10; break;
						case EventTrigger::ON_STOP_AUDIO : nTrigger = 11; break;
					}
					if ( aEvent.Offset.hasValue() )
					{
						if ( aEvent.Offset >>= eTiming )
						{
							if ( eTiming == Timing_INDEFINITE )
								nBegin = -1;
						}
						else if ( aEvent.Offset >>= fTiming )
							nBegin = (sal_Int32)( fTiming * 1000.0 );
					}
					aSource = aEvent.Source;
				}
				else if ( aAny >>= eTiming )
				{
					bCreateEvent = sal_True;
					if ( eTiming == Timing_INDEFINITE )
						nBegin = -1;
				}
				else if ( aAny >>= fTiming )
				{
					bCreateEvent = sal_True;
					if ( eTiming == Timing_INDEFINITE )
						nBegin = (sal_Int32)( fTiming * 1000.0 );
				}
			}
			break;

			case 2 :
			{
				if ( nFlags & ( 1 << i ) )
				{
					bCreateEvent = sal_True;
					nU1 = 1;
					nTrigger = 9;
				}
			}
			break;
			case 3 :
			{
				if ( nFlags & ( 1 << i ) )
				{
					bCreateEvent = sal_True;
					nU1 = 1;
					nTrigger = 10;
				}
			}
			break;
		};
		if ( bCreateEvent )
		{
			EscherExContainer aAnimEvent( rStrm, DFF_msofbtAnimEvent, i + 1 );
			{
				EscherExAtom aAnimTrigger( rStrm, DFF_msofbtAnimTrigger );
				rStrm << nU1
					  << nTrigger
					  << nU3
					  << nBegin;
			}
			exportAnimateTargetElement( rStrm, aSource, ( nFlags & ( 1 << i ) ) != 0 );
		}
	}
}

Any AnimationExporter::convertAnimateValue( const Any& rSourceValue, const rtl::OUString& rAttributeName ) const
{
	rtl::OUString aDest;
	if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "X" ) )
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Y" ) )
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) )
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Height" ) )
		)
	{
		rtl::OUString aStr;
		if ( rSourceValue >>= aStr )
		{
			ImplTranslateAttribute( aStr, TRANSLATE_MEASURE );
			aDest += aStr;
		}
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Rotate" ) )			// "r" or "style.rotation" ?
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SkewX" ) )
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Opacity" ) )
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CharHeight" ) )
		)
	{
		double fNumber = 0.0;
		if ( rSourceValue >>= fNumber )
			aDest += rtl::OUString::valueOf( fNumber );
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Color" ) )
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FillColor" ) )		// "Fillcolor" or "FillColor" ?
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LineColor" ) )
			|| rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CharColor" ) )
		)
	{
		sal_Int32 nColor = 0;
		Sequence< double > aHSL( 3 );
		rtl::OUString aP( RTL_CONSTASCII_USTRINGPARAM( "," ) );
		if ( rSourceValue >>= aHSL )
		{
			aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hsl(" ) );
			aDest += rtl::OUString::valueOf( (sal_Int32)( aHSL[ 0 ] / ( 360.0 / 255 ) ) );
			aDest += aP;
			aDest += rtl::OUString::valueOf( (sal_Int32)( aHSL[ 1 ] * 255.0 ) );
			aDest += aP;
			aDest += rtl::OUString::valueOf( (sal_Int32)( aHSL[ 2 ] * 255.0 ) );
			aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ")" ) );
		}
		else if ( rSourceValue >>= nColor )
		{
			aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "rgb(" ) );
			aDest += rtl::OUString::valueOf( (sal_Int32)( (sal_Int8)nColor ) );
			aDest += aP;
			aDest += rtl::OUString::valueOf( (sal_Int32)( (sal_Int8)( nColor >> 8 ) ) );
			aDest += aP;
			aDest += rtl::OUString::valueOf( (sal_Int32)( (sal_Int8)( nColor >> 16 ) ) );
			aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ")" ) );
		}
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FillStyle" ) ) )
	{
		::com::sun::star::drawing::FillStyle eFillStyle;
		if ( rSourceValue >>= eFillStyle )
		{
			if ( eFillStyle == ::com::sun::star::drawing::FillStyle_NONE )
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "none" ) );	// ?
			else
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "solid" ) );
		}
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LineStyle" ) ) )
	{
		::com::sun::star::drawing::LineStyle eLineStyle;
		if ( rSourceValue >>= eLineStyle )
		{
			if ( eLineStyle == ::com::sun::star::drawing::LineStyle_NONE )
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) );
			else
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) );
		}
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CharWeight" ) ) )
	{
		float fFontWeight = 0.0;
		if ( rSourceValue >>= fFontWeight )
		{
			if ( fFontWeight == com::sun::star::awt::FontWeight::BOLD )
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "bold" ) );
			else
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "normal" ) );
		}
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CharUnderline" ) ) )
	{
		sal_Int16 nFontUnderline = 0;
		if ( rSourceValue >>= nFontUnderline )
		{
			if ( nFontUnderline == com::sun::star::awt::FontUnderline::NONE )
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) );
			else
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) );
		}
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CharPosture" ) ) )
	{
		::com::sun::star::awt::FontSlant eFontSlant;
		if ( rSourceValue >>= eFontSlant )
		{
			if ( eFontSlant == com::sun::star::awt::FontSlant_ITALIC )
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "italic" ) );
			else
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "normal" ) );	// ?
		}
	}
	else if ( rAttributeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Visibility" ) ) )
	{
		sal_Bool bVisible = sal_True;
		if ( rSourceValue >>= bVisible )
		{
			if ( bVisible )
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "visible" ) );
			else
				aDest += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hidden" ) );
		}
	}
	Any aRet;
	if ( aDest.getLength() )
		aRet <<= aDest;
	else
		aRet = rSourceValue;
	return aRet;
}

void AnimationExporter::exportAnimateSet( SvStream& rStrm, const Reference< XAnimationNode >& xNode, int nAfterEffectType )
{
	Reference< XAnimateSet > xSet( xNode, UNO_QUERY );
	if( xSet.is() )
	{
		EscherExContainer aAnimateSet( rStrm, DFF_msofbtAnimateSet, 0 );
		{
			EscherExAtom aAnimateSetData( rStrm, DFF_msofbtAnimateSetData );
			sal_uInt32 nId1 = 1;			// ??
			sal_uInt32 nId2 = 1;			// ??
			rStrm << nId1 << nId2;
		}
		Any aConvertedValue( convertAnimateValue( xSet->getTo(), xSet->getAttributeName() ) );
		if ( aConvertedValue.hasValue() )
			exportAnimProperty( rStrm, 1, aConvertedValue, TRANSLATE_NONE );
		exportAnimateTarget( rStrm, xNode, 0, nAfterEffectType );
	}
}

sal_uInt32 GetValueTypeForAttributeName( const rtl::OUString& rAttributeName )
{
	sal_uInt32 nValueType = 0;

/*
	AnimationValueType::STRING == 0;
	AnimationValueType::NUMBER == 1;
	AnimationValueType::COLOR  == 2;
*/

	struct Entry
	{
		const sal_Char* pName;
		sal_uInt8		nType;
	};
	static const Entry lcl_attributeMap[] =
    {
        { "charcolor", 2 },
        { "charfontname", 0 },
        { "charheight", 1 },
        { "charposture", 0 },
        // TODO(Q1): This should prolly be changed in PPT import
        // { "charrotation", ATTRIBUTE_CHAR_ROTATION },
        { "charrotation", 1 },
        { "charunderline", 0 },
        { "charweight", 0 },
        { "color", 2 },
        { "dimcolor", 2 },
        { "fillcolor", 2 },
        { "fillstyle", 0 },
        { "height", 1 },
        { "linecolor", 2 },
        { "linestyle", 0 },
        { "opacity", 0 },
        { "rotate", 1 },
        { "skewx", 1 },
        { "skewy", 1 },
        { "visibility", 1 },
        { "width", 1 },
        { "x", 1 },
        { "y", 1 },
		{ NULL, 0 }
	};
	const Entry* pPtr = &lcl_attributeMap[ 0 ];
	while( pPtr->pName )
	{
		if ( rAttributeName.equalsIgnoreAsciiCaseAscii( pPtr->pName ) )
		{
			nValueType = pPtr->nType;
			break;
		}
		pPtr++;
	}
	DBG_ASSERT( pPtr->pName, "GetValueTypeForAttributeName, unknown property value!" );
	return nValueType;
}

void AnimationExporter::exportAnimate( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
	Reference< XAnimate > xAnimate( xNode, UNO_QUERY );
	if ( xAnimate.is() )
	{
		Any aBy  ( xAnimate->getBy() );
		Any aFrom( xAnimate->getFrom() );
		Any aTo  ( xAnimate->getTo() );

		EscherExContainer aContainer( rStrm, DFF_msofbtAnimate, 0 );
		{
			EscherExAtom	aAnimateData( rStrm, DFF_msofbtAnimateData );
			sal_uInt32 nBits = 0x38;
			sal_Int16 nTmp = xAnimate->getCalcMode();
			sal_uInt32 nCalcMode = /* (nTmp == AnimationCalcMode::FORMULA) ? 2 : */ (nTmp == AnimationCalcMode::LINEAR) ? 1 : 0;
			nTmp = xAnimate->getValueType();
			sal_uInt32 nValueType = GetValueTypeForAttributeName( xAnimate->getAttributeName() );

			if ( aBy.hasValue() )
				nBits |= 1;
			if ( aFrom.hasValue() )
				nBits |= 2;
			if ( aTo.hasValue() )
				nBits |= 4;

			rStrm << nCalcMode
				  << nBits
				  << nValueType;
		}
		if ( aBy.hasValue() )
			exportAnimProperty( rStrm, 1, aBy, TRANSLATE_NUMBER_TO_STRING | TRANSLATE_MEASURE );
		if ( aFrom.hasValue() )
			exportAnimProperty( rStrm, 2, aFrom, TRANSLATE_NUMBER_TO_STRING | TRANSLATE_MEASURE );
		if ( aTo.hasValue() )
			exportAnimProperty( rStrm, 3, aTo, TRANSLATE_NUMBER_TO_STRING | TRANSLATE_MEASURE );

		exportAnimateKeyPoints( rStrm, xAnimate );
		exportAnimateTarget( rStrm, xNode );
	}
}

void AnimationExporter::exportAnimateTarget( SvStream& rStrm, const Reference< XAnimationNode >& xNode, const sal_uInt32 nForceAttributeNames, int nAfterEffectType )
{
	EscherExContainer aAnimateTarget( rStrm, DFF_msofbtAnimateTarget, 0 );
	Reference< XAnimate > xAnimate( xNode, UNO_QUERY );
	if ( xAnimate.is() )
	{
		{
			EscherExAtom aAnimateTargetSettings( rStrm, DFF_msofbtAnimateTargetSettings, 0 );
			// nBits %0001: additive, %0010: accumulate, %0100: attributeName, %1000: transformtype
			// nAdditive 0 = base, 1 = sum, 2 = replace, 3 = multiply, 4 = none
			// nAccumulate 0 = none, 1 = always
			// nTransformType 0: "property" else "image"
			sal_uInt32 nBits = 0;
			sal_uInt32 nAdditive = 0;
			sal_uInt32 nAccumulate = 0;
			sal_uInt32 nTransformType = 0;
			if ( xAnimate.is() )
			{
				if ( xAnimate->getAttributeName().getLength() )
					nBits |= 4;		// what is attributeName ?, maybe this is set if a DFF_msofbtAnimateAttributeNames is written
				sal_Int16 nAdditiveMode = xAnimate->getAdditive();
				if ( nAdditiveMode != AnimationAdditiveMode::BASE )
				{
					nBits |= 1;
					switch( nAdditiveMode )
					{
						case AnimationAdditiveMode::SUM : nAdditive = 1; break;
						case AnimationAdditiveMode::REPLACE : nAdditive = 2; break;
						case AnimationAdditiveMode::MULTIPLY : nAdditive = 3; break;
						case AnimationAdditiveMode::NONE : nAdditive = 4; break;
					}
				}
				if ( xAnimate->getAccumulate() )
				{
					nBits  |= 2;
					nAccumulate = 1;
				}
			}
			rStrm << nBits
				<< nAdditive
				<< nAccumulate
				<< nTransformType;
		}
		if ( xAnimate->getAttributeName().getLength() || nForceAttributeNames )
		{
			EscherExContainer aAnimateAttributeNames( rStrm, DFF_msofbtAnimateAttributeNames, 1 );
			rtl::OUString aAttributeName( xAnimate->getAttributeName() );
			if ( nForceAttributeNames )
			{
				switch( nForceAttributeNames )
				{
					case 1 : aAttributeName = rtl::OUString::createFromAscii( "r" ); break;
				}
			}
			sal_Int32 nIndex = 0;
			do
			{
				OUString aToken( aAttributeName.getToken( 0, ';', nIndex ) );
				exportAnimPropertyString( rStrm, 0, aToken, TRANSLATE_ATTRIBUTE );
			}
			while ( nIndex >= 0 );
		}

		if( nAfterEffectType != AFTEREFFECT_NONE )
		{
			EscherExContainer aAnimPropertySet( rStrm, DFF_msofbtAnimPropertySet );
			exportAnimPropertyuInt32( rStrm, 6, 1, TRANSLATE_NONE );
			if( nAfterEffectType == AFTEREFFECT_COLOR )
			{
				exportAnimPropertyuInt32( rStrm, 4, 0, TRANSLATE_NONE );
				exportAnimPropertyuInt32( rStrm, 5, 0, TRANSLATE_NONE );
			}
		}
		exportAnimateTargetElement( rStrm, aTarget.hasValue() ? aTarget : xAnimate->getTarget(), sal_False );
	}
}

void AnimationExporter::exportAnimateTargetElement( SvStream& rStrm, const Any aAny, const sal_Bool bCreate2b01Atom )
{
	Reference< XShape > xShape;
	aAny >>= xShape;
	sal_uInt32 nRefMode = 0;	// nRefMode == 2 -> Paragraph
	sal_Int32 begin = -1;
	sal_Int32 end = -1;

	if( !xShape.is() )
	{
		ParagraphTarget aParaTarget;
		if( aAny >>= aParaTarget )
			xShape = aParaTarget.Shape;
		if ( xShape.is() )
		{
			// now calculating the character range for the paragraph
			sal_Int16 nParagraph = aParaTarget.Paragraph;
			Reference< XSimpleText > xText( xShape, UNO_QUERY );
			if ( xText.is() )
			{
				nRefMode = 2;
				Reference< XEnumerationAccess > xTextParagraphEnumerationAccess( xText, UNO_QUERY );
				if ( xTextParagraphEnumerationAccess.is() )
				{
					Reference< XEnumeration > xTextParagraphEnumeration( xTextParagraphEnumerationAccess->createEnumeration() );
					if ( xTextParagraphEnumeration.is() )
					{
						sal_Int16 nCurrentParagraph;
						begin = end = nCurrentParagraph = 0;
						while ( xTextParagraphEnumeration->hasMoreElements() )
						{
							Reference< XTextRange > xTextRange( xTextParagraphEnumeration->nextElement(), UNO_QUERY );
							if ( xTextRange.is() )
							{
								rtl::OUString aParaText( xTextRange->getString() );
								sal_Int32 nLength = aParaText.getLength() + 1;
								end += nLength;
								if ( nCurrentParagraph == nParagraph )
									break;
								nCurrentParagraph++;
								begin += nLength;
							}
						}
					}
				}
			}
		}
	}
	if ( xShape.is() || bCreate2b01Atom )
	{
		EscherExContainer aAnimateTargetElement( rStrm, DFF_msofbtAnimateTargetElement );
		if ( xShape.is() )
		{
			EscherExAtom aAnimReference( rStrm, DFF_msofbtAnimReference );

			sal_uInt32 nRefType = 1;	// TODO: nRefType == 2 -> Sound;
			sal_uInt32 nRefId = ((EscherSolverContainer&)mrSolverContainer).GetShapeId( xShape );

			rStrm << nRefMode
				  << nRefType
				  << nRefId
				  << begin
				  << end;
		}
		if ( bCreate2b01Atom )
		{
			EscherExAtom a2b01Atom( rStrm, 0x2b01 );
			rStrm << (sal_uInt32)1;		// ?
		}
	}
}

void AnimationExporter::exportAnimateKeyPoints( SvStream& rStrm, const Reference< XAnimate >& xAnimate )
{
	Sequence< double > aKeyTimes( xAnimate->getKeyTimes() );
	Sequence< Any > aValues( xAnimate->getValues() );
	OUString aFormula( xAnimate->getFormula() );
	if ( aKeyTimes.getLength() )
	{
		EscherExContainer aAnimKeyPoints( rStrm, DFF_msofbtAnimKeyPoints );
		sal_Int32 i;
		for ( i = 0; i < aKeyTimes.getLength(); i++ )
		{
			{
				EscherExAtom aAnimKeyTime( rStrm, DFF_msofbtAnimKeyTime );
				sal_Int32 nKeyTime = (sal_Int32)( aKeyTimes[ i ] * 1000.0 );
				rStrm << nKeyTime;
			}
			Any aAny[ 2 ];
			if ( aValues[ i ].hasValue() )
			{
				ValuePair aPair;
				if ( aValues[ i ] >>= aPair )
				{
					aAny[ 0 ] = convertAnimateValue( aPair.First, xAnimate->getAttributeName() );
					aAny[ 1 ] = convertAnimateValue( aPair.Second, xAnimate->getAttributeName() );
				}
				else
				{
					aAny[ 0 ] = convertAnimateValue( aValues[ i ], xAnimate->getAttributeName() );
				}
				if ( !i && aFormula.getLength() )
				{
					ImplTranslateAttribute( aFormula, TRANSLATE_MEASURE );
					aAny[ 1 ] <<= aFormula;
				}
				exportAnimProperty( rStrm, 0, aAny[ 0 ], TRANSLATE_NONE );
				exportAnimProperty( rStrm, 1, aAny[ 1 ], TRANSLATE_NONE );
			}
		}
	}
}

void AnimationExporter::exportAnimValue( SvStream& rStrm, const Reference< XAnimationNode >& xNode, const sal_Bool bExportAlways )
{
	Any aAny;
	// repeat count (0)
	double fRepeat = 0.0;
	float fRepeatCount = 0.0;
	com::sun::star::animations::Timing eTiming;
	aAny = xNode->getRepeatCount();
	if ( aAny >>= eTiming )
	{
		if ( eTiming == Timing_INDEFINITE )
			fRepeatCount = ((float)3.40282346638528860e+38);
	}
	else if ( aAny >>= fRepeat )
		fRepeatCount = (float)fRepeat;
	if ( fRepeatCount != 0.0 )
	{
		EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
		sal_uInt32 nType = 0;
		rStrm << nType
			  << fRepeatCount;
	}
	// accelerate (3)
	float fAccelerate = (float)xNode->getAcceleration();
	if ( bExportAlways || ( fAccelerate != 0.0 ) )
	{
		EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
		sal_uInt32 nType = 3;
		rStrm << nType
			  << fAccelerate;
	}

	// decelerate (4)
	float fDecelerate = (float)xNode->getDecelerate();
	if ( bExportAlways || ( fDecelerate != 0.0 ) )
	{
		EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
		sal_uInt32 nType = 4;
		rStrm << nType
			  << fDecelerate;
	}

	// autoreverse (5)
	sal_Bool bAutoReverse = xNode->getAutoReverse();
	if ( bExportAlways || bAutoReverse )
	{
		EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
		sal_uInt32 nType = 5;
		sal_uInt32 nVal  = bAutoReverse ? 1 : 0;
		rStrm << nType
			  << nVal;
	}
}

void AnimationExporter::exportTransitionFilter( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
	Reference< XTransitionFilter > xFilter( xNode, UNO_QUERY );
	if ( xFilter.is() )
	{
		EscherExContainer aAnimateFilter( rStrm, DFF_msofbtAnimateFilter );
		{
			EscherExAtom aAnimateFilterData( rStrm, DFF_msofbtAnimateFilterData );
			sal_uInt32 nBits = 3;		// bit 0 -> use AnimAttributeValue
										// bit 1 -> use nTransition

			sal_uInt32 nTransition = xFilter->getMode() ? 0 : 1;
			rStrm << nBits
				  << nTransition;
		}
		const sal_Char* pFilter = transition::find( xFilter->getTransition(), xFilter->getSubtype(), xFilter->getDirection() );
		if ( pFilter )
		{
			const OUString aStr( OUString::createFromAscii( pFilter ) );
			exportAnimPropertyString( rStrm, 1, aStr, TRANSLATE_NONE );
		}
		exportAnimateTarget( rStrm, xNode );
	}
}

void AnimationExporter::exportAnimateMotion( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
	Reference< XAnimateMotion > xMotion( xNode, UNO_QUERY );
	if ( xMotion.is() )
	{
		EscherExContainer aAnimateMotion( rStrm, DFF_msofbtAnimateMotion );
		{
			{	//SJ: Ignored from import filter
				EscherExAtom aAnimateMotionData( rStrm, DFF_msofbtAnimateMotionData ); 
				sal_uInt32 nBits = 0x98;
				sal_uInt32 nOrigin = 0x2;
				float fByX = 100.0;	// nBits&1
				float fByY = 100.0; // nBits&1
				float fFromX = 0.0; // nBits&2
				float fFromY = 0.0; // nBits&2
				float fToX = 100.0; // nBits&4
				float fToY = 100.0; // nBits&4
				rStrm << nBits << fByX << fByY << fFromX << fFromY << fToX << fToY << nOrigin;
			}
/*			?
			{
				EscherExAtom aF137( rStrm, 0xf137 );
			}
*/
			OUString aStr;
			if ( xMotion->getPath() >>= aStr )
			{
				if ( aStr.getLength() )
					exportAnimPropertyString( rStrm, 1, aStr, TRANSLATE_NONE );
			}
			exportAnimateTarget( rStrm, xNode );
		}
	}
}

void AnimationExporter::exportAnimateTransform( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
	Reference< XAnimateTransform > xTransform( xNode, UNO_QUERY );
	if ( xTransform.is() )
	{
		if ( xTransform->getTransformType() ==  AnimationTransformType::SCALE )
		{
			EscherExContainer aAnimateScale( rStrm, DFF_msofbtAnimateScale );
			{
				EscherExAtom aAnimateScaleData( rStrm, DFF_msofbtAnimateScaleData );
				sal_uInt32 nBits = 0;
				sal_uInt32 nZoomContents = 1;
				float fByX = 100.0;
				float fByY = 100.0;
				float fFromX = 0.0;
				float fFromY = 0.0;
				float fToX = 100.0;
				float fToY = 100.0;

				double fX = 0.0, fY = 0.0;
				ValuePair aPair;
				if ( xTransform->getBy() >>= aPair )
				{
					if ( ( aPair.First >>= fX ) && ( aPair.Second >>= fY ) )
					{
						nBits |= 1;
						fByX = (float)( fX * 100 );
						fByY = (float)( fY * 100 );
					}
				}
				if ( xTransform->getFrom() >>= aPair )
				{
					if ( ( aPair.First >>= fX ) && ( aPair.Second >>= fY ) )
					{
						nBits |= 2;
						fFromX = (float)( fX * 100 );
						fFromY = (float)( fY * 100 );
					}
				}
				if( xTransform->getTo() >>= aPair )
				{
					if ( ( aPair.First >>= fX ) && ( aPair.Second >>= fY ) )
					{
						nBits |= 4;
						fToX = (float)( fX * 100 );
						fToY = (float)( fY * 100 );
					}
				}

				// TODO: ZoomContents:
				//if( nBits & 8 )
				//(	fprintf( mpFile, " zoomContents=\"%s\"", nZoomContents ? "true" : "false" );

				rStrm << nBits << fByX << fByY << fFromX << fFromY << fToX << fToY << nZoomContents;
			}
			exportAnimateTarget( rStrm, xNode );
		}
		else if ( xTransform->getTransformType() ==  AnimationTransformType::ROTATE )
		{
			EscherExContainer aAnimateRotation( rStrm, DFF_msofbtAnimateRotation );
			{
				EscherExAtom aAnimateRotationData( rStrm, DFF_msofbtAnimateRotationData );
				sal_uInt32 nBits = 0;
				sal_uInt32 nU1 = 0;
				float fBy = 360.0;
				float fFrom = 0.0;
				float fTo = 360.0;

				double fVal = 0.0;
				if ( xTransform->getBy() >>= fVal )
				{
					nBits |= 1;
					fBy = (float)fVal;
				}
				if ( xTransform->getFrom() >>= fVal )
				{
					nBits |= 2;
					fFrom = (float)fVal;
				}
				if ( xTransform->getTo() >>= fVal )
				{
					nBits |= 4;
					fTo = (float)fVal;
				}
				rStrm << nBits << fBy << fFrom << fTo << nU1;
			}
			exportAnimateTarget( rStrm, xNode, 1 );
		}
	}
}

sal_Bool AnimationExporter::getColorAny( const Any& rAny, const sal_Int16 nColorSpace, sal_Int32& rMode, sal_Int32& rA, sal_Int32& rB, sal_Int32& rC ) const
{
	sal_Bool bIsColor = sal_True;

	rMode = 0;
	if ( nColorSpace == AnimationColorSpace::HSL )
		rMode = 1;

	sal_Int32 nColor = 0;
	Sequence< double > aHSL( 3 );
	if ( rAny >>= nColor )		// RGB color
	{
		rA = (sal_uInt8)( nColor >> 24 );
		rB = (sal_uInt8)( nColor >> 8 );
		rC = (sal_uInt8)( nColor );
	}
	else if ( rAny >>= aHSL )	// HSL
	{
		rA = (sal_Int32) ( aHSL[ 0 ] * 255.0 / 360.0 );
		rB = (sal_Int32) ( aHSL[ 1 ] * 255.0 );
		rC = (sal_Int32) ( aHSL[ 2 ] * 255.0 );
	}
	else
		bIsColor = sal_False;
	return bIsColor;
}

void AnimationExporter::exportAnimateColor( SvStream& rStrm, const Reference< XAnimationNode >& xNode, int nAfterEffectType )
{
	Reference< XAnimateColor > xColor( xNode, UNO_QUERY );
	if ( xColor.is() )
	{
		EscherExContainer aAnimateColor( rStrm, DFF_msofbtAnimateColor );
		{
			EscherExAtom aAnimateColorData( rStrm, DFF_msofbtAnimateColorData );
			sal_uInt32 nBits = 8;

			sal_Int32 nByMode, nByA, nByB, nByC;
			nByMode = nByA = nByB = nByC = 0;

			sal_Int32 nFromMode, nFromA, nFromB, nFromC;
			nFromMode = nFromA = nFromB = nFromC = 0;

			sal_Int32 nToMode, nToA, nToB, nToC;
			nToMode = nToA = nToB = nToC = 0;

			sal_Int16 nColorSpace = xColor->getColorInterpolation();

			Any aAny( xColor->getBy() );
			if ( aAny.hasValue() )
			{
				if ( getColorAny( aAny, nColorSpace, nByMode, nByA, nByB, nByC ) )
					nBits |= 0x11;
			}
			aAny = xColor->getFrom();
			if ( aAny.hasValue() )
			{
				if ( getColorAny( aAny, nColorSpace, nFromMode, nFromA, nFromB, nFromC ) )
					nBits |= 0x12;
			}
			aAny = xColor->getTo();
			if ( aAny.hasValue() )
			{
				if ( getColorAny( aAny, nColorSpace, nToMode, nToA, nToB, nToC ) )
					nBits |= 0x14;
			}
			rStrm   << nBits
					<< nByMode << nByA << nByB << nByC
					<< nFromMode << nFromA << nFromB << nFromC
					<< nToMode << nToA << nToB << nToC;
		}
		exportAnimateTarget( rStrm, xNode, 0, nAfterEffectType );
	}
}

void AnimationExporter::exportIterate( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
	Reference< XIterateContainer > xIterate( xNode, UNO_QUERY );
	if ( xIterate.is() )
	{
		EscherExAtom aAnimIteration( rStrm, DFF_msofbtAnimIteration );

		float		fInterval = 10.0;
		sal_Int32	nTextUnitEffect = 0;
		sal_Int32	nU1 = 1;
		sal_Int32	nU2 = 1;
		sal_Int32	nU3 = 0xe;

		sal_Int16 nIterateType = xIterate->getIterateType();
		switch( nIterateType )
		{
			case TextAnimationType::BY_WORD : nTextUnitEffect = 1; break;
			case TextAnimationType::BY_LETTER : nTextUnitEffect = 2; break;
		}

		fInterval = (float)xIterate->getIterateInterval();

		// convert interval from absolute to percentage
		double fDuration = 0.0;

		Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
		if( xEnumerationAccess.is() )
		{
			Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
			if( xEnumeration.is() )
			{
				while( xEnumeration->hasMoreElements() )
				{
					Reference< XAnimate > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
					if( xChildNode.is() )
					{
						double fChildBegin = 0.0;
						double fChildDuration = 0.0;
						xChildNode->getBegin() >>= fChildBegin;
						xChildNode->getDuration() >>= fChildDuration;

						fChildDuration += fChildBegin;
						if( fChildDuration > fDuration )
							fDuration = fChildDuration;
					}
				}
			}
		}

		if( fDuration )
			fInterval = (float)(100.0 * fInterval / fDuration);

		rStrm << fInterval << nTextUnitEffect << nU1 << nU2 << nU3;
		aTarget = xIterate->getTarget();
	}
}

} // namespace ppt;

