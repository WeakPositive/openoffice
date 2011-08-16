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

#if ! defined INCLUDED_SLIDESHOW_WATERFALLWIPE_HXX
#define INCLUDED_SLIDESHOW_WATERFALLWIPE_HXX

#include "parametricpolypolygon.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>


namespace slideshow {
namespace internal {

/// Generate a vertical left waterfall wipe
class WaterfallWipe : public ParametricPolyPolygon
{
public:
    WaterfallWipe( sal_Int32 nElements, bool flipOnYAxis = false );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    bool m_flipOnYAxis;
    ::basegfx::B2DPolygon m_waterfall;
};

}
}

#endif /* INCLUDED_SLIDESHOW_WATERFALLWIPE_HXX */
