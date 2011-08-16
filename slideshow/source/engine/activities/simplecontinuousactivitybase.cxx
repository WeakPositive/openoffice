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
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <simplecontinuousactivitybase.hxx>


namespace slideshow
{
    namespace internal
    {
        SimpleContinuousActivityBase::SimpleContinuousActivityBase(
            const ActivityParameters& rParms ) :
            ActivityBase( rParms ),
            maTimer( rParms.mrActivitiesQueue.getTimer() ),
            mnMinSimpleDuration( rParms.mnMinDuration ),
            mnMinNumberOfFrames( rParms.mnMinNumberOfFrames ),
            mnCurrPerformCalls( 0 )
        {
        }

        void SimpleContinuousActivityBase::startAnimation()
        {
            // init timer. We measure animation time only when we're 
            // actually started.
            maTimer.reset();
        }

        double SimpleContinuousActivityBase::calcTimeLag() const
        {
            ActivityBase::calcTimeLag();
            if (! isActive())
                return 0.0;
            
            // retrieve locally elapsed time
            const double nCurrElapsedTime( maTimer.getElapsedTime() );

            // log time
            VERBOSE_TRACE( "SimpleContinuousActivityBase::calcTimeLag(): "
                           "next step is based on time: %f", nCurrElapsedTime );

            // go to great length to ensure a proper animation
            // run. Since we don't know how often we will be called
            // here, try to spread the animator calls uniquely over
            // the [0,1] parameter range. Be aware of the fact that
            // perform will be called at least mnMinNumberOfTurns
            // times. 

            // fraction of time elapsed
            const double nFractionElapsedTime(
                nCurrElapsedTime / mnMinSimpleDuration );

            // fraction of minimum calls performed
            const double nFractionRequiredCalls(
                double(mnCurrPerformCalls) / mnMinNumberOfFrames );

            // okay, so now, the decision is easy:
            //
            // If the fraction of time elapsed is smaller than the
            // number of calls required to be performed, then we calc
            // the position on the animation range according to
            // elapsed time. That is, we're so to say ahead of time.
            //
            // In contrary, if the fraction of time elapsed is larger,
            // then we're lagging, and we thus calc the position on
            // the animation time line according to the fraction of
            // calls performed. Thus, the animation is forced to slow
            // down, and take the required minimal number of steps,
            // sufficiently equally distributed across the animation
            // time line.
            if( nFractionElapsedTime < nFractionRequiredCalls )
            {
                VERBOSE_TRACE( "SimpleContinuousActivityBase::calcTimeLag(): "
                               "t=%f is based on time", nFractionElapsedTime );
                return 0.0;
            }
            else
            {
                VERBOSE_TRACE( "SimpleContinuousActivityBase::perform(): "
                               "t=%f is based on number of calls",
                               nFractionRequiredCalls );
                
                // lag global time, so all other animations lag, too:
                return ((nFractionElapsedTime - nFractionRequiredCalls)
                        * mnMinSimpleDuration);
            }
        }
    
        bool SimpleContinuousActivityBase::perform()
        {
            // call base class, for start() calls and end handling
            if( !ActivityBase::perform() )
                return false; // done, we're ended
            

            // get relative animation position
            // ===============================
            
            const double nCurrElapsedTime( maTimer.getElapsedTime() );
            double nT( nCurrElapsedTime / mnMinSimpleDuration );
            

            // one of the stop criteria reached?
            // =================================

            // will be set to true below, if one of the termination criteria
            // matched.
            bool bActivityEnding( false );

            if( isRepeatCountValid() )
            {
                // Finite duration
                // ===============

                // When we've autoreverse on, the repeat count
                // doubles
                const double nRepeatCount( getRepeatCount() );
                const double nEffectiveRepeat( isAutoReverse() ?
                                               2.0*nRepeatCount :
                                               nRepeatCount );

                // time (or frame count) elapsed?
                if( nEffectiveRepeat <= nT )
                {
                    // okee. done for now. Will not exit right here,
                    // to give animation the chance to render the last
                    // frame below
                    bActivityEnding = true;
                            
                    // clamp animation to max permissible value
                    nT = nEffectiveRepeat;
                }
            }


            // need to do auto-reverse?
            // ========================

            double nRepeats;
            double nRelativeSimpleTime;

            // TODO(Q3): Refactor this mess
            if( isAutoReverse() )
            {
                // divert active duration into repeat and
                // fractional part.
                const double nFractionalActiveDuration( modf(nT, &nRepeats) );

                // for auto-reverse, map ranges [1,2), [3,4), ...
                // to ranges [0,1), [1,2), etc.
                if( ((int)nRepeats) % 2 )
                {
                    // we're in an odd range, reverse sweep
                    nRelativeSimpleTime = 1.0 - nFractionalActiveDuration;
                }
                else
                {
                    // we're in an even range, pass on as is
                    nRelativeSimpleTime = nFractionalActiveDuration;
                }

                // effective repeat count for autoreverse is half of
                // the input time's value (each run of an autoreverse
                // cycle is half of a repeat)
                nRepeats /= 2;
            }
            else
            {
                // determine repeat
                // ================

                // calc simple time and number of repeats from nT
                // Now, that's easy, since the fractional part of
                // nT gives the relative simple time, and the 
                // integer part the number of full repeats:
                nRelativeSimpleTime = modf(nT, &nRepeats);

                // clamp repeats to max permissible value (maRepeats.getValue() - 1.0)
                if( isRepeatCountValid() &&
                    nRepeats >= getRepeatCount() )
                {
                    // Note that this code here only gets
                    // triggered if maRepeats.getValue() is an
                    // _integer_. Otherwise, nRepeats will never
                    // reach nor exceed
                    // maRepeats.getValue(). Thus, the code below
                    // does not need to handle cases of fractional
                    // repeats, and can always assume that a full
                    // animation run has ended (with
                    // nRelativeSimpleTime=1.0 for
                    // non-autoreversed activities).

                    // with modf, nRelativeSimpleTime will never
                    // become 1.0, since nRepeats is incremented and
                    // nRelativeSimpleTime set to 0.0 then.
                    //
                    // For the animation to reach its final value,
                    // nRepeats must although become
                    // maRepeats.getValue()-1.0, and
                    // nRelativeSimpleTime=1.0.
                    nRelativeSimpleTime = 1.0;
                    nRepeats -= 1.0;
                }
            }

            // actually perform something
            // ==========================

            simplePerform( nRelativeSimpleTime,
                           // nRepeats is already integer-valued
                           static_cast<sal_uInt32>( nRepeats ) );


            // delayed endActivity() call from end condition check
            // below. Issued after the simplePerform() call above, to
            // give animations the chance to correctly reach the
            // animation end value, without spurious bail-outs because
            // of isActive() returning false.
            if( bActivityEnding )
                endActivity();

            // one more frame successfully performed
            ++mnCurrPerformCalls;

            return isActive();
        }
    }
}
