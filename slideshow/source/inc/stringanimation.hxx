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

#ifndef INCLUDED_SLIDESHOW_STRINGANIMATION_HXX
#define INCLUDED_SLIDESHOW_STRINGANIMATION_HXX

#include "animation.hxx"
#include <rtl/ustring.hxx>


/* Definition of StringAnimation interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface defining a string animation.

        	This interface is a specialization of the Animation
        	interface, and is used to animate attributes representable
        	by a discrete character string (e.g. font names)
         */
        class StringAnimation : public Animation
        {
        public:
            typedef ::rtl::OUString ValueType;

            /** Set the animation to value rStr

                @param rStr
                Current animation value.
             */
            virtual bool operator()( const ValueType& rStr ) = 0;

            /** Request the underlying value for this animation.

            	This is necessary for pure To or By animations, as the
            	Activity cannot determine a sensible start value
            	otherwise.

                @attention Note that you are only permitted to query
                for the underlying value, if the animation has actually
                been started (via start() call).
             */
            virtual ValueType getUnderlyingValue() const = 0;
        };

        typedef ::boost::shared_ptr< StringAnimation > StringAnimationSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_STRINGANIMATION_HXX */
