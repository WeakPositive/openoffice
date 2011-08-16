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
#include "precompiled_extensions.hxx"
#include "optiongrouplayouter.hxx"
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include "controlwizard.hxx"
#include "groupboxwiz.hxx"
#include "dbptools.hxx"

//.........................................................................
namespace dbp
{
//.........................................................................

#define BUTTON_HEIGHT		300
#define TOP_HEIGHT			300
#define HEIGHT				450
#define OFFSET				300
#define MIN_WIDTH			600

	using namespace ::com::sun::star::uno;
	using namespace ::com::sun::star::drawing;
	using namespace ::com::sun::star::beans;
	using namespace ::com::sun::star::awt;
	using namespace ::com::sun::star::container;
	using namespace ::com::sun::star::lang;
	using namespace ::com::sun::star::text;
	using namespace ::com::sun::star::view;

	//=====================================================================
	//= OOptionGroupLayouter
	//=====================================================================
	//---------------------------------------------------------------------
	OOptionGroupLayouter::OOptionGroupLayouter(const Reference< XMultiServiceFactory >& _rxORB)
		:m_xORB(_rxORB)
	{
	}

	//---------------------------------------------------------------------
	void OOptionGroupLayouter::doLayout(const OControlWizardContext& _rContext, const OOptionGroupSettings& _rSettings)
	{
		Reference< XShapes > xPageShapes(_rContext.xDrawPage, UNO_QUERY);
		if (!xPageShapes.is())
		{
			DBG_ERROR("OOptionGroupLayouter::OOptionGroupLayouter: missing the XShapes interface for the page!");
			return;
		}

		Reference< XMultiServiceFactory > xDocFactory(_rContext.xDocumentModel, UNO_QUERY);
		if (!xDocFactory.is())
		{
			DBG_ERROR("OOptionGroupLayouter::OOptionGroupLayouter: no document service factory!");
			return;
		}

		// no. of buttons to create
		sal_Int32 nRadioButtons = _rSettings.aLabels.size();

		sal_Int32 nTopSpace = 0;

		// the shape of the groupbox
		::com::sun::star::awt::Size aControlShapeSize = _rContext.xObjectShape->getSize();
		// maybe need to adjust the size if the control shapes
		sal_Int32 nMinShapeHeight = BUTTON_HEIGHT*(nRadioButtons+1) + BUTTON_HEIGHT + BUTTON_HEIGHT/4;
		if (aControlShapeSize.Height < nMinShapeHeight)
			aControlShapeSize.Height = nMinShapeHeight;
		if (aControlShapeSize.Width < MIN_WIDTH)
			aControlShapeSize.Width = MIN_WIDTH;
		_rContext.xObjectShape->setSize(aControlShapeSize);

		// if we're working on a writer document, we need to anchor the shape
		implAnchorShape(Reference< XPropertySet >(_rContext.xObjectShape, UNO_QUERY));

		// shape collection (for grouping the shapes)
		Reference< XShapes > xButtonCollection(m_xORB->createInstance(
				::rtl::OUString::createFromAscii("com.sun.star.drawing.ShapeCollection")),
			UNO_QUERY);
		// first member : the shape of the control
		xButtonCollection->add(_rContext.xObjectShape.get());

		sal_Int32 nTempHeight = (aControlShapeSize.Height - BUTTON_HEIGHT/4) / (nRadioButtons + 1);

		::com::sun::star::awt::Point aShapePosition = _rContext.xObjectShape->getPosition();

		::com::sun::star::awt::Size aButtonSize(aControlShapeSize);
		aButtonSize.Width = aControlShapeSize.Width - OFFSET;
		aButtonSize.Height = HEIGHT;
		::com::sun::star::awt::Point aButtonPosition;
		aButtonPosition.X = aShapePosition.X + OFFSET;

		::rtl::OUString sElementsName = ::rtl::OUString::createFromAscii("RadioGroup");
		disambiguateName(Reference< XNameAccess >(_rContext.xForm, UNO_QUERY), sElementsName);

		StringArray::const_iterator aLabelIter = _rSettings.aLabels.begin();
		StringArray::const_iterator aValueIter = _rSettings.aValues.begin();
		for (sal_Int32 i=0; i<nRadioButtons; ++i, ++aLabelIter, ++aValueIter)
		{
			aButtonPosition.Y = aShapePosition.Y + (i+1) * nTempHeight + nTopSpace;

			Reference< XPropertySet > xRadioModel(
					xDocFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.component.RadioButton")),
				UNO_QUERY);

			// the label
			xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("Label"), makeAny(rtl::OUString(*aLabelIter)));
			// the value
			xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("RefValue"), makeAny(rtl::OUString(*aValueIter)));

			// default selection
			if (_rSettings.sDefaultField == *aLabelIter)
				xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("DefaultState"), makeAny(sal_Int16(1)));

			// the connection to the database field
			if (0 != _rSettings.sDBField.Len())
				xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("DataField"), makeAny(::rtl::OUString(_rSettings.sDBField)));

			// the name for the model
			xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("Name"), makeAny(sElementsName));

			// create a shape for the radio button
			Reference< XControlShape > xRadioShape(
					xDocFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.drawing.ControlShape")),
				UNO_QUERY);
			Reference< XPropertySet > xShapeProperties(xRadioShape, UNO_QUERY);

			// if we're working on a writer document, we need to anchor the shape
			implAnchorShape(xShapeProperties);

			// position it
			xRadioShape->setSize(aButtonSize);
			xRadioShape->setPosition(aButtonPosition);
			// knitting with the model
			xRadioShape->setControl(Reference< XControlModel >(xRadioModel, UNO_QUERY));

			// the name of the shape
			if (xShapeProperties.is())
				xShapeProperties->setPropertyValue(::rtl::OUString::createFromAscii("Name"), makeAny(sElementsName));

			// add to the page
			xPageShapes->add(xRadioShape.get());
			// add to the collection (for the later grouping)
			xButtonCollection->add(xRadioShape.get());

			// set the GroupBox as "LabelControl" for the RadioButton
			// (_after_ having inserted the model into the page!)
			xRadioModel->setPropertyValue(::rtl::OUString::createFromAscii("LabelControl"), makeAny(_rContext.xObjectModel));
		}

		// group the shapes
		try
		{
			Reference< XShapeGrouper > xGrouper(_rContext.xDrawPage, UNO_QUERY);
			if (xGrouper.is())
			{
				Reference< XShapeGroup > xGroupedOptions = xGrouper->group(xButtonCollection);
				Reference< XSelectionSupplier > xSelector(_rContext.xDocumentModel->getCurrentController(), UNO_QUERY);
				if (xSelector.is())
					xSelector->select(makeAny(xGroupedOptions));
			}
		}
		catch(Exception&)
		{
			DBG_ERROR("OOptionGroupLayouter::doLayout: caught an exception while grouping the shapes!");
		}
	}

	//---------------------------------------------------------------------
	void OOptionGroupLayouter::implAnchorShape(const Reference< XPropertySet >& _rxShapeProps)
	{
		static const ::rtl::OUString s_sAnchorPropertyName = ::rtl::OUString::createFromAscii("AnchorType");
		Reference< XPropertySetInfo > xPropertyInfo;
		if (_rxShapeProps.is())
			xPropertyInfo = _rxShapeProps->getPropertySetInfo();
		if (xPropertyInfo.is() && xPropertyInfo->hasPropertyByName(s_sAnchorPropertyName))
			_rxShapeProps->setPropertyValue(s_sAnchorPropertyName, makeAny(TextContentAnchorType_AT_PAGE));
	}

//.........................................................................
}	// namespace dbp
//.........................................................................

