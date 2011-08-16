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
#include "precompiled_svx.hxx"
#include <svx/sdr/overlay/overlaytriangle.hxx>
#include <tools/poly.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
	namespace overlay
	{
		drawinglayer::primitive2d::Primitive2DSequence OverlayTriangle::createOverlayObjectPrimitive2DSequence()
		{
			basegfx::B2DPolygon aPolygon;

			aPolygon.append(getBasePosition());
			aPolygon.append(getSecondPosition());
			aPolygon.append(getThirdPosition());
			aPolygon.setClosed(true);
            
            const drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
					basegfx::B2DPolyPolygon(aPolygon),
					getBaseColor().getBColor()));

            return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
		}

		OverlayTriangle::OverlayTriangle(
			const basegfx::B2DPoint& rBasePos,
			const basegfx::B2DPoint& rSecondPos,
			const basegfx::B2DPoint& rThirdPos,
			Color aTriangleColor)
		:	OverlayObjectWithBasePosition(rBasePos, aTriangleColor),
			maSecondPosition(rSecondPos),
			maThirdPosition(rThirdPos)
		{
		}
		
		OverlayTriangle::~OverlayTriangle()
		{
		}

        void OverlayTriangle::setSecondPosition(const basegfx::B2DPoint& rNew)
		{
			if(rNew != maSecondPosition)
			{
				// remember new value
				maSecondPosition = rNew;

				// register change (after change)
				objectChange();
			}
		}

		void OverlayTriangle::setThirdPosition(const basegfx::B2DPoint& rNew)
		{
			if(rNew != maThirdPosition)
			{
				// remember new value
				maThirdPosition = rNew;

				// register change (after change)
				objectChange();
			}
		}
	} // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
