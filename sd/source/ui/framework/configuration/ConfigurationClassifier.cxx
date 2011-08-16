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

#include "precompiled_sd.hxx"

#include "ConfigurationClassifier.hxx"

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#undef VERBOSE
//#define VERBOSE 2


namespace sd { namespace framework {

ConfigurationClassifier::ConfigurationClassifier (
    const Reference<XConfiguration>& rxConfiguration1,
    const Reference<XConfiguration>& rxConfiguration2)
    : mxConfiguration1(rxConfiguration1),
      mxConfiguration2(rxConfiguration2),
      maC1minusC2(),
      maC2minusC1(),
      maC1andC2()
{
}




bool ConfigurationClassifier::Partition (void)
{
    maC1minusC2.clear();
    maC2minusC1.clear();
    maC1andC2.clear();

    PartitionResources(
        mxConfiguration1->getResources(NULL, OUString(), AnchorBindingMode_DIRECT),
        mxConfiguration2->getResources(NULL, OUString(), AnchorBindingMode_DIRECT));

    return !maC1minusC2.empty() || !maC2minusC1.empty();
}




const ConfigurationClassifier::ResourceIdVector& ConfigurationClassifier::GetC1minusC2 (void) const
{
    return maC1minusC2;
}




const ConfigurationClassifier::ResourceIdVector& ConfigurationClassifier::GetC2minusC1 (void) const
{
    return maC2minusC1;
}



const ConfigurationClassifier::ResourceIdVector& ConfigurationClassifier::GetC1andC2 (void) const
{
    return maC1andC2;
}


void ConfigurationClassifier::PartitionResources (
    const ::com::sun::star::uno::Sequence<Reference<XResourceId> >& rS1,
    const ::com::sun::star::uno::Sequence<Reference<XResourceId> >& rS2)
{
    ResourceIdVector aC1minusC2;
    ResourceIdVector aC2minusC1;
    ResourceIdVector aC1andC2;

    // Classify the resources in the configurations that are not bound to
    // other resources.
    ClassifyResources(
        rS1,
        rS2,
        aC1minusC2,
        aC2minusC1,
        aC1andC2);

#if defined VERBOSE && VERBOSE >= 2
    OSL_TRACE("copying resource ids to C1-C2\r");
#endif
    CopyResources(aC1minusC2, mxConfiguration1, maC1minusC2);
#if defined VERBOSE && VERBOSE >= 2
    OSL_TRACE("copying resource ids to C2-C1\r");
#endif
    CopyResources(aC2minusC1, mxConfiguration2, maC2minusC1);
    
    // Process the unique resources that belong to both configurations.
    ResourceIdVector::const_iterator iResource;
    for (iResource=aC1andC2.begin(); iResource!=aC1andC2.end(); ++iResource)
    {
        maC1andC2.push_back(*iResource);
        PartitionResources(
            mxConfiguration1->getResources(*iResource, OUString(), AnchorBindingMode_DIRECT),
            mxConfiguration2->getResources(*iResource, OUString(), AnchorBindingMode_DIRECT));
    }
}




void ConfigurationClassifier::ClassifyResources (
    const ::com::sun::star::uno::Sequence<Reference<XResourceId> >& rS1,
    const ::com::sun::star::uno::Sequence<Reference<XResourceId> >& rS2,
    ResourceIdVector& rS1minusS2,
    ResourceIdVector& rS2minusS1,
    ResourceIdVector& rS1andS2)
{
    // Get arrays from the sequences for faster iteration.
    const Reference<XResourceId>* aA1 = rS1.getConstArray();
    const Reference<XResourceId>* aA2 = rS2.getConstArray();
    sal_Int32 nL1 (rS1.getLength());
    sal_Int32 nL2 (rS2.getLength());

    // Find all elements in rS1 and place them in rS1minusS2 or rS1andS2
    // depending on whether they are in rS2 or not.
    for (sal_Int32 i=0; i<nL1; ++i)
    {
        bool bFound (false);
        for (sal_Int32 j=0; j<nL2 && !bFound; ++j)
            if (aA1[i]->getResourceURL().equals(aA2[j]->getResourceURL()))
                bFound = true;
        
        if (bFound)
            rS1andS2.push_back(aA1[i]);
        else
            rS1minusS2.push_back(aA1[i]);
    }

    // Find all elements in rS2 that are not in rS1.  The elements that are
    // in both rS1 and rS2 have been handled above and are therefore ignored
    // here.
    for (sal_Int32 j=0; j<nL2; ++j)
    {
        bool bFound (false);
        for (sal_Int32 i=0; i<nL1 && !bFound; ++i)
            if (aA2[j]->getResourceURL().equals(aA1[i]->getResourceURL()))
                bFound = true;
        
        if ( ! bFound)
            rS2minusS1.push_back(aA2[j]);
    }
}




void ConfigurationClassifier::CopyResources (
    const ResourceIdVector& rSource,
    const Reference<XConfiguration>& rxConfiguration,
    ResourceIdVector& rTarget)
{
    // Copy all resources bound to the ones in aC1minusC2Unique to rC1minusC2.
    ResourceIdVector::const_iterator iResource (rSource.begin());
    ResourceIdVector::const_iterator iEnd(rSource.end());
    for ( ; iResource!=iEnd; ++iResource)
    {
        const Sequence<Reference<XResourceId> > aBoundResources (
            rxConfiguration->getResources(
                *iResource,
                OUString(),
                AnchorBindingMode_INDIRECT));
        const sal_Int32 nL (aBoundResources.getLength());
        
        rTarget.reserve(rTarget.size() + 1 + nL);
        rTarget.push_back(*iResource);

#if defined VERBOSE && VERBOSE >= 2
        OSL_TRACE("    copying %s\r",
            OUStringToOString(FrameworkHelper::ResourceIdToString(*iResource),
                RTL_TEXTENCODING_UTF8).getStr());
#endif
        
        const Reference<XResourceId>* aA = aBoundResources.getConstArray();
        for (sal_Int32 i=0; i<nL; ++i)
        {
            rTarget.push_back(aA[i]);
#if defined VERBOSE && VERBOSE >= 2
            OSL_TRACE("    copying %s\r",
                OUStringToOString(FrameworkHelper::ResourceIdToString(aA[i]),
                    RTL_TEXTENCODING_UTF8).getStr());
#endif
        }
    }
}


void ConfigurationClassifier::TraceResourceIdVector (
    const sal_Char* pMessage,
    const ResourceIdVector& rResources) const
{

    OSL_TRACE(pMessage);
    ResourceIdVector::const_iterator iResource;
    for (iResource=rResources.begin(); iResource!=rResources.end(); ++iResource)
    {
        OUString sResource (FrameworkHelper::ResourceIdToString(*iResource));
        OSL_TRACE("    %s\r",
            OUStringToOString(sResource, RTL_TEXTENCODING_UTF8).getStr());
    }
}


} } // end of namespace sd::framework
