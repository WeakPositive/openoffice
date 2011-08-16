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

// MARKER(update_precomp.py): Generated on 2006-09-01 17:49:33.249162

#ifdef PRECOMPILED_HEADERS
//---MARKER---
#include "sal/config.h"
#include "sal/types.h"

#include "com/sun/star/awt/CharSet.hpp"
#include "com/sun/star/awt/FontFamily.hpp"
#include "com/sun/star/awt/FontPitch.hpp"
#include "com/sun/star/awt/FontSlant.hpp"
#include "com/sun/star/awt/FontUnderline.hpp"
#include "com/sun/star/awt/FontWeight.hpp"
#include "com/sun/star/awt/Gradient.hpp"
#include "com/sun/star/awt/Point.hpp"
#include "com/sun/star/awt/Rectangle.hpp"
#include "com/sun/star/awt/Size.hpp"
#include "com/sun/star/awt/XBitmap.hpp"
#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XFastPropertySet.hpp"
#include "com/sun/star/beans/XMultiPropertySet.hpp"
#include "com/sun/star/beans/XMultiPropertyStates.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertyState.hpp"
#include "com/sun/star/chart2/data/XDataProvider.hpp"
#include "com/sun/star/chart2/data/XDataReceiver.hpp"
#include "com/sun/star/chart2/data/XDataSequence.hpp"
#include "com/sun/star/chart2/data/XDataSink.hpp"
#include "com/sun/star/chart2/data/XDataSource.hpp"
#include "com/sun/star/chart2/data/XNumericalDataSequence.hpp"
#include "com/sun/star/chart2/data/XTextualDataSequence.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XContentEnumerationAccess.hpp"
#include "com/sun/star/container/XEnumeration.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"
#include "com/sun/star/container/XIndexContainer.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/document/XDocumentInfoSupplier.hpp"
#include "com/sun/star/document/XEventBroadcaster.hpp"
#include "com/sun/star/document/XEventsSupplier.hpp"
#include "com/sun/star/document/XViewDataSupplier.hpp"
#include "com/sun/star/drawing/BitmapMode.hpp"
#include "com/sun/star/drawing/CircleKind.hpp"
#include "com/sun/star/drawing/Direction3D.hpp"
#include "com/sun/star/drawing/DoubleSequence.hpp"
#include "com/sun/star/drawing/FillStyle.hpp"
#include "com/sun/star/drawing/FlagSequence.hpp"
#include "com/sun/star/drawing/Hatch.hpp"
#include "com/sun/star/drawing/HomogenMatrix.hpp"
#include "com/sun/star/drawing/HomogenMatrix3.hpp"
#include "com/sun/star/drawing/LineDash.hpp"
#include "com/sun/star/drawing/LineJoint.hpp"
#include "com/sun/star/drawing/LineStyle.hpp"
#include "com/sun/star/drawing/NormalsKind.hpp"
#include "com/sun/star/drawing/PointSequence.hpp"
#include "com/sun/star/drawing/PointSequenceSequence.hpp"
#include "com/sun/star/drawing/PolyPolygonBezierCoords.hpp"
#include "com/sun/star/drawing/PolyPolygonShape3D.hpp"
#include "com/sun/star/drawing/PolygonKind.hpp"
#include "com/sun/star/drawing/Position3D.hpp"
#include "com/sun/star/drawing/ProjectionMode.hpp"
#include "com/sun/star/drawing/RectanglePoint.hpp"
#include "com/sun/star/drawing/ShadeMode.hpp"
#include "com/sun/star/drawing/TextFitToSizeType.hpp"
#include "com/sun/star/drawing/TextHorizontalAdjust.hpp"
#include "com/sun/star/drawing/TextVerticalAdjust.hpp"
#include "com/sun/star/drawing/XDrawPage.hpp"
#include "com/sun/star/drawing/XDrawPageSupplier.hpp"
#include "com/sun/star/drawing/XDrawPagesSupplier.hpp"
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/embed/Aspects.hpp"
#include "com/sun/star/embed/EmbedStates.hpp"
#include "com/sun/star/embed/XComponentSupplier.hpp"
#include "com/sun/star/embed/XEmbedObjectCreator.hpp"
#include "com/sun/star/embed/XEmbedPersist.hpp"
#include "com/sun/star/embed/XEmbeddedClient.hpp"
#include "com/sun/star/embed/XEmbeddedObject.hpp"
#include "com/sun/star/embed/XVisualObject.hpp"
#include "com/sun/star/frame/XController.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XLoadable.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XStorable.hpp"
#include "com/sun/star/frame/XSynchronousFrameLoader.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/IndexOutOfBoundsException.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XTypeProvider.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/sheet/XRangeSelection.hpp"
#include "com/sun/star/sheet/XSheetCellRangeContainer.hpp"
#include "com/sun/star/sheet/XSpreadsheet.hpp"
#include "com/sun/star/sheet/XSpreadsheetDocument.hpp"
#include "com/sun/star/sheet/XSpreadsheets.hpp"
#include "com/sun/star/style/CaseMap.hpp"
#include "com/sun/star/style/ParagraphAdjust.hpp"
#include "com/sun/star/style/XStyle.hpp"
#include "com/sun/star/style/XStyleFamiliesSupplier.hpp"
#include "com/sun/star/text/ControlCharacter.hpp"
#include "com/sun/star/text/FontEmphasis.hpp"
#include "com/sun/star/text/FontRelief.hpp"
#include "com/sun/star/text/RubyAdjust.hpp"
#include "com/sun/star/text/WritingMode.hpp"
#include "com/sun/star/text/WritingMode2.hpp"
#include "com/sun/star/text/XText.hpp"
#include "com/sun/star/text/XTextCursor.hpp"
#include "com/sun/star/text/XTextRange.hpp"
#include "com/sun/star/ui/XContextMenuInterception.hpp"
#include "com/sun/star/uno/Any.h"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.h"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.h"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XAggregation.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/XWeak.hpp"
#include "com/sun/star/util/CloseVetoException.hpp"
#include "com/sun/star/util/URL.hpp"
#include "com/sun/star/util/XCloneable.hpp"
#include "com/sun/star/util/XCloseListener.hpp"
#include "com/sun/star/util/XCloseable.hpp"
#include "com/sun/star/util/XModifiable.hpp"
#include "com/sun/star/util/XNumberFormatsSupplier.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
#include "com/sun/star/view/XPrintable.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"

#include "comphelper/broadcasthelper.hxx"
#include "comphelper/processfactory.hxx"
#include "comphelper/propertycontainer.hxx"
#include "comphelper/uno3.hxx"

#include "cppu/macros.hxx"
#include "cppu/unotype.hxx"

#include "cppuhelper/component_context.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/interfacecontainer.hxx"
#include "cppuhelper/propshlp.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "cppuhelper/weak.hxx"
#include "cppuhelper/weakref.hxx"

#include "i18npool/mslangid.hxx"

#include "osl/conditn.hxx"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/getglobalmutex.hxx"
#include "osl/mutex.hxx"

#include "rtl/math.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "rtl/uuid.h"

#include "sfx2/sfxdefs.hxx"

#include "sot/clsids.hxx"

#include "svl/eitem.hxx"
#include "svl/intitem.hxx"
#include "svl/itemiter.hxx"
#include "svl/itempool.hxx"
#include "svl/itemset.hxx"
#include "unotools/pathoptions.hxx"
#include "svl/poolitem.hxx"
#include "svl/stritem.hxx"
#include "unotools/syslocale.hxx"
#include "svl/urihelper.hxx"
#include "svl/whiter.hxx"

#include "editeng/brshitem.hxx"
#include "editeng/crsditem.hxx"
#include "editeng/eeitem.hxx"
#include "editeng/fhgtitem.hxx"
#include "svx/flagsdef.hxx"
#include "editeng/flstitem.hxx"
#include "editeng/forbiddencharacterstable.hxx"
#include "svx/ofaitem.hxx"
#include "editeng/postitem.hxx"
#include "svx/svddef.hxx"
#include "svx/svditer.hxx"
#include "svx/svdtypes.hxx"
#include "editeng/udlnitem.hxx"
#include "svx/unomodel.hxx"
#include "editeng/unoprnms.hxx"
#include "editeng/wghtitem.hxx"
#include "svx/xdef.hxx"
#include "svx/xflbmtit.hxx"
#include "svx/xflbstit.hxx"
#include "svx/xflclit.hxx"
#include "svx/xflftrit.hxx"
#include "svx/xflgrit.hxx"
#include "svx/xflhtit.hxx"
#include "svx/xgrscit.hxx"
#include "svx/xlnclit.hxx"
#include "svx/xlndsit.hxx"
#include "editeng/xmlcnitm.hxx"
#include "svx/xpoly.hxx"


#include "tools/color.hxx"
#include "tools/debug.hxx"
#include "tools/link.hxx"
#include "tools/resid.hxx"
#include "tools/resmgr.hxx"
#include "tools/solar.h"
#include "tools/string.hxx"
#include "tools/table.hxx"
#include "tools/urlobj.hxx"

#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "unotools/configitem.hxx"
#include "unotools/eventlisteneradapter.hxx"

#include "tools/fldunit.hxx"

#include "vos/mutex.hxx"
//---MARKER---
#endif
