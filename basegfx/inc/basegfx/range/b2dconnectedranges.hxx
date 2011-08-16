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

#ifndef _BGFX_RANGE_B2DCONNECTEDRANGES_HXX
#define _BGFX_RANGE_B2DCONNECTEDRANGES_HXX

#include <osl/diagnose.h>
#include <basegfx/range/b2drange.hxx>
#include <list>
#include <utility>
#include <algorithm>


namespace basegfx
{
    /** Calculate connected ranges from input ranges.

    	This template constructs a list of connected ranges from the
    	given input ranges. That is, the output will contain a set of
    	ranges, itself containing a number of input ranges, which will
    	be mutually non-intersecting.

        Example:
        <code>
        -------------------
        |          -------|
        |          |     ||
        | ---      |     ||
        | | |      -------|        --------
        | | +---------    |        |      |
        | --+        |    |        |      |
        |   |        |    |        --------
        |   ----------    |
        -------------------
        </code

        Here, the outer rectangles represent the output
        ranges. Contained are the input rectangles that comprise these
        output ranges.

        @tpl UserData
        User data to be stored along with the range, to later identify
        which range went into which connected component. Must be
        assignable, default- and copy-constructible.
     */
    template< typename UserData > class B2DConnectedRanges
    {
    public:
        /// Type of the basic entity (rect + user data)
        typedef ::std::pair< B2DRange, UserData > ComponentType;
        typedef ::std::list< ComponentType >	  ComponentListType;

		/// List of (intersecting) components, plus overall bounds
        struct ConnectedComponents
        {
            ComponentListType	maComponentList;
            B2DRange			maTotalBounds;
        };

        typedef ::std::list< ConnectedComponents > ConnectedComponentsType;	


        /// Create the range calculator
        B2DConnectedRanges() :
            maDisjunctAggregatesList(),
            maTotalBounds()
        {
        }

        /** Query total bounds of all added ranges.

        	@return the union bound rect over all added ranges.
         */
        B2DRange getBounds() const
        {
            return maTotalBounds;
        }

        /** Add an additional range.

			This method integrates a new range into the connected
			ranges lists. The method has a worst-case time complexity
			of O(n^2), with n denoting the number of already added
			ranges (typically, for well-behaved input, it is O(n)
			though).
         */
        void addRange( const B2DRange& 	rRange,
                       const UserData&	rUserData )
        {
            // check whether fast path is possible: if new range is
            // outside accumulated total range, can add it as a
            // separate component right away.
            const bool bNotOutsideEverything( 
                maTotalBounds.overlaps( rRange ) );
            
            // update own global bounds range
            maTotalBounds.expand( rRange );

            // assemble anything intersecting with rRange into
            // this new connected component
            ConnectedComponents aNewConnectedComponent;

            // as at least rRange will be a member of
            // aNewConnectedComponent (will be added below), can
            // preset the overall bounds here.
            aNewConnectedComponent.maTotalBounds = rRange;


            //
            //  STAGE 1: Search for intersecting maDisjunctAggregatesList entries
            //  =================================================================
            //

            // if rRange is empty, it will intersect with no
            // maDisjunctAggregatesList member. Thus, we can safe us
            // the check.
            // if rRange is outside all other rectangle, skip here,
            // too
            if( bNotOutsideEverything &&
                !rRange.isEmpty() )
            {
                typename ConnectedComponentsType::iterator aCurrAggregate;
                typename ConnectedComponentsType::iterator aLastAggregate;

                // flag, determining whether we touched one or more of
                // the maDisjunctAggregatesList entries. _If_ we did,
                // we have to repeat the intersection process, because
                // these changes might have generated new
                // intersections.
                bool bSomeAggregatesChanged;

                // loop, until bSomeAggregatesChanged stays false
                do
                {
                    // only continue loop if 'intersects' branch below was hit
                    bSomeAggregatesChanged = false;

                    // iterate over all current members of maDisjunctAggregatesList
                    for( aCurrAggregate=maDisjunctAggregatesList.begin(),
                             aLastAggregate=maDisjunctAggregatesList.end();
                         aCurrAggregate != aLastAggregate; )
                    {
                        // first check if current component's bounds
                        // are empty. This ensures that distinct empty
                        // components are not merged into one
                        // aggregate. As a matter of fact, they have
                        // no position and size.

                        if( !aCurrAggregate->maTotalBounds.isEmpty() &&
                            aCurrAggregate->maTotalBounds.overlaps( 
                                aNewConnectedComponent.maTotalBounds ) )
                        {
                            // union the intersecting
                            // maDisjunctAggregatesList element into
                            // aNewConnectedComponent

                            // calc union bounding box
                            aNewConnectedComponent.maTotalBounds.expand( aCurrAggregate->maTotalBounds );
                        
                            // extract all aCurrAggregate components
                            // to aNewConnectedComponent
                            aNewConnectedComponent.maComponentList.splice( 
                                aNewConnectedComponent.maComponentList.end(), 
                                aCurrAggregate->maComponentList );
                        
                            // remove and delete aCurrAggregate entry
                            // from list (we've gutted it's content
                            // above). list::erase() will update our
                            // iterator with the predecessor here.
                            aCurrAggregate = maDisjunctAggregatesList.erase( aCurrAggregate );

                            // at least one aggregate changed, need to rescan everything
                            bSomeAggregatesChanged = true;
                        }
                        else
                        {
                            aCurrAggregate++;
                        }
                    }
                }
                while( bSomeAggregatesChanged );
            }

            //
            //  STAGE 2: Add newly generated connected component list element
            //  =============================================================
            //

            // add new component to the end of the component list
            aNewConnectedComponent.maComponentList.push_back( 
                ComponentType( rRange, rUserData ) );
                               
            // do some consistency checks (aka post conditions)
            OSL_ENSURE( !aNewConnectedComponent.maComponentList.empty(),
                        "B2DConnectedRanges::addRange(): empty aggregate list" );
            OSL_ENSURE( !aNewConnectedComponent.maTotalBounds.isEmpty() || 
                        (aNewConnectedComponent.maTotalBounds.isEmpty() && 
                         aNewConnectedComponent.maComponentList.size() == 1),
                        "B2DConnectedRanges::addRange(): empty ranges must be solitary");
            
            // add aNewConnectedComponent as a new entry to
            // maDisjunctAggregatesList
            maDisjunctAggregatesList.push_back( aNewConnectedComponent );
        }

        /** Apply a functor to each of the disjunct component
            aggregates.

            @param aFunctor
            Functor to apply. Must provide an operator( const ConnectedComponents& ).

            @return a copy of the functor, as applied to all aggregates.
         */
        template< typename UnaryFunctor > UnaryFunctor forEachAggregate( UnaryFunctor aFunctor ) const
        {
            return ::std::for_each( maDisjunctAggregatesList.begin(),
                                    maDisjunctAggregatesList.end(),
                                    aFunctor );
        }
        
    private:
        // default: disabled copy/assignment
        B2DConnectedRanges(const B2DConnectedRanges&);
        B2DConnectedRanges& operator=( const B2DConnectedRanges& );

        /** Current list of disjunct sets of connected components

        	Each entry corresponds to one of the top-level rectangles
        	in the drawing above.
         */
        ConnectedComponentsType	maDisjunctAggregatesList;
        
        /** Global bound rect over all added ranges.
         */
        B2DRange				maTotalBounds;
    };
}

#endif /* _BGFX_RANGE_B2DCONNECTEDRANGES_HXX */
