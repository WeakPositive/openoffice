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

#include "framework/ConfigurationController.hxx"

#include "framework/Configuration.hxx"
#include "framework/FrameworkHelper.hxx"
#include "ConfigurationUpdater.hxx"
#include "ConfigurationControllerBroadcaster.hxx"
#include "ConfigurationTracer.hxx"
#include "GenericConfigurationChangeRequest.hxx"
#include "ResourceFactoryManager.hxx"
#include "UpdateRequest.hxx"
#include "ChangeRequestQueueProcessor.hxx"
#include "ConfigurationClassifier.hxx"
#include "ViewShellBase.hxx"
#include "UpdateLockManager.hxx"
#include "DrawController.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/stl_types.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using rtl::OUString;
using ::sd::framework::FrameworkHelper;

#undef VERBOSE
//#define VERBOSE 3


namespace sd { namespace framework {

Reference<XInterface> SAL_CALL ConfigurationController_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    (void)rxContext;
    return static_cast<XWeak*>(new ConfigurationController());
}




OUString ConfigurationController_getImplementationName (void) throw(RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Draw.framework.configuration.ConfigurationController"));
}




Sequence<rtl::OUString> SAL_CALL ConfigurationController_getSupportedServiceNames (void)
    throw (RuntimeException)
{
	static const OUString sServiceName(OUString::createFromAscii(
        "com.sun.star.drawing.framework.ConfigurationController"));
	return Sequence<rtl::OUString>(&sServiceName, 1);
}




//----- ConfigurationController::Implementation -------------------------------

class ConfigurationController::Implementation
{
public:
    Implementation (
        ConfigurationController& rController,
        const Reference<frame::XController>& rxController);
    ~Implementation (void);

    Reference<XControllerManager> mxControllerManager;

    /** The Broadcaster class implements storing and calling of listeners.
    */
    ::boost::shared_ptr<ConfigurationControllerBroadcaster> mpBroadcaster;

    /** The requested configuration which is modifed (asynchronously) by
        calls to requestResourceActivation() and
        requestResourceDeactivation().  The mpConfigurationUpdater makes the
        current configuration reflect the content of this one.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfiguration> mxRequestedConfiguration;

    ViewShellBase* mpBase;
    
    ::boost::shared_ptr<ResourceFactoryManager> mpResourceFactoryContainer;

    ::boost::shared_ptr<ConfigurationControllerResourceManager> mpResourceManager;

    ::boost::shared_ptr<ConfigurationUpdater> mpConfigurationUpdater;

    /** The queue processor ownes the queue of configuration change request
        objects and processes the objects.
    */
    ::boost::scoped_ptr<ChangeRequestQueueProcessor> mpQueueProcessor;

    ::boost::shared_ptr<ConfigurationUpdaterLock> mpConfigurationUpdaterLock;

    sal_Int32 mnLockCount;
};




//===== ConfigurationController::Lock =========================================

ConfigurationController::Lock::Lock (const Reference<XConfigurationController>& rxController)
    : mxController(rxController)
{
    OSL_ASSERT(mxController.is());
    
    if (mxController.is())
        mxController->lock();
}

 
 
  
ConfigurationController::Lock::~Lock (void)
{
    if (mxController.is())
        mxController->unlock();
}




//===== ConfigurationController ===============================================

ConfigurationController::ConfigurationController (void) throw()
    : ConfigurationControllerInterfaceBase(MutexOwner::maMutex),
      mpImplementation(),
      mbIsDisposed(false)
{
}




ConfigurationController::~ConfigurationController (void) throw()
{
}




void SAL_CALL ConfigurationController::disposing (void)
{
    if (mpImplementation.get() == NULL)
        return;

#if defined VERBOSE && VERBOSE>=1
    OSL_TRACE("ConfigurationController::disposing\n");
    OSL_TRACE("    requesting empty configuration\n");
#endif
    // To destroy all resources an empty configuration is requested and then,
    // synchronously, all resulting requests are processed.
    mpImplementation->mpQueueProcessor->Clear();
    restoreConfiguration(new Configuration(this,false));
    mpImplementation->mpQueueProcessor->ProcessUntilEmpty();
#if defined VERBOSE && VERBOSE>=1
    OSL_TRACE("    all requests processed\n");
#endif

    // Now that all resources have been deactivated, mark the controller as
    // disposed.
    mbIsDisposed = true;

    // Release the listeners.
	lang::EventObject aEvent;
	aEvent.Source = uno::Reference<uno::XInterface>((cppu::OWeakObject*)this);

    {
        const ::vos::OGuard aSolarGuard (Application::GetSolarMutex());
        mpImplementation->mpBroadcaster->DisposeAndClear();
    }

    mpImplementation->mpQueueProcessor.reset();
    mpImplementation->mxRequestedConfiguration = NULL;
    mpImplementation.reset();
}




void ConfigurationController::ProcessEvent (void)
{
    if (mpImplementation.get() != NULL)
    {
        OSL_ASSERT(mpImplementation->mpQueueProcessor.get()!=NULL);

        mpImplementation->mpQueueProcessor->ProcessOneEvent();
    }
}




void ConfigurationController::RequestSynchronousUpdate (void)
{
    if (mpImplementation.get() == NULL)
        return;
    if (mpImplementation->mpQueueProcessor.get() == 0)
        return;
    mpImplementation->mpQueueProcessor->ProcessUntilEmpty();
}




//----- XConfigurationControllerBroadcaster -----------------------------------

void SAL_CALL ConfigurationController::addConfigurationChangeListener (
    const Reference<XConfigurationChangeListener>& rxListener,
    const ::rtl::OUString& rsEventType,
    const Any& rUserData)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

	ThrowIfDisposed();
    OSL_ASSERT(mpImplementation.get()!=NULL);
    mpImplementation->mpBroadcaster->AddListener(rxListener, rsEventType, rUserData);
}




void SAL_CALL ConfigurationController::removeConfigurationChangeListener (
    const Reference<XConfigurationChangeListener>& rxListener)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

	ThrowIfDisposed();
    mpImplementation->mpBroadcaster->RemoveListener(rxListener);
}




void SAL_CALL ConfigurationController::notifyEvent (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
	ThrowIfDisposed();
    mpImplementation->mpBroadcaster->NotifyListeners(rEvent);
}





//----- XConfigurationController ----------------------------------------------
    
void SAL_CALL ConfigurationController::lock (void)
    throw (RuntimeException)
{
    OSL_ASSERT(mpImplementation.get()!=NULL);
    OSL_ASSERT(mpImplementation->mpConfigurationUpdater.get()!=NULL);

    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    
    ++mpImplementation->mnLockCount;
    if (mpImplementation->mpConfigurationUpdaterLock.get()==NULL)
        mpImplementation->mpConfigurationUpdaterLock
            = mpImplementation->mpConfigurationUpdater->GetLock();
}




void SAL_CALL ConfigurationController::unlock (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Allow unlocking while the ConfigurationController is being disposed
    // (but not when that is done and the controller is disposed.)
    if (rBHelper.bDisposed)
        ThrowIfDisposed();

    OSL_ASSERT(mpImplementation->mnLockCount>0);
    --mpImplementation->mnLockCount;
    if (mpImplementation->mnLockCount == 0)
        mpImplementation->mpConfigurationUpdaterLock.reset();
}




void SAL_CALL ConfigurationController::requestResourceActivation (
    const Reference<XResourceId>& rxResourceId,
    ResourceActivationMode eMode)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
   	ThrowIfDisposed();

    // Check whether we are being disposed.  This is handled differently
    // then being completely disposed because the first thing disposing()
    // does is to deactivate all remaining resources.  This is done via
    // regular methods which must not throw DisposedExceptions.  Therefore
    // we just return silently during that stage.
    if (rBHelper.bInDispose)
    {
#if defined VERBOSE && VERBOSE>=1
        OSL_TRACE("ConfigurationController::requestResourceActivation(): ignoring %s\n",
            OUStringToOString(
                FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif
        return;
    }

#if defined VERBOSE && VERBOSE>=2
    OSL_TRACE("ConfigurationController::requestResourceActivation() %s\n",
        OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif

    if (rxResourceId.is())
    {
        if (eMode == ResourceActivationMode_REPLACE)
        {
            // Get a list of the matching resources and create deactivation
            // requests for them.
            Sequence<Reference<XResourceId> > aResourceList (
                mpImplementation->mxRequestedConfiguration->getResources(
                    rxResourceId->getAnchor(),
                    rxResourceId->getResourceTypePrefix(),
                    AnchorBindingMode_DIRECT));

            for (sal_Int32 nIndex=0; nIndex<aResourceList.getLength(); ++nIndex)
            {
                // Do not request the deactivation of the resource for which
                // this method was called.  Doing it would not change the
                // outcome but would result in unnecessary work.
                if (rxResourceId->compareTo(aResourceList[nIndex]) == 0)
                    continue;

                // Request the deactivation of a resource and all resources
                // linked to it.
                requestResourceDeactivation(aResourceList[nIndex]);
            }
        }
    
        Reference<XConfigurationChangeRequest> xRequest(
            new GenericConfigurationChangeRequest(
                rxResourceId,
                GenericConfigurationChangeRequest::Activation));
        postChangeRequest(xRequest);
    }
}




void SAL_CALL ConfigurationController::requestResourceDeactivation (
    const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

#if defined VERBOSE && VERBOSE>=2
    OSL_TRACE("ConfigurationController::requestResourceDeactivation() %s\n",
            OUStringToOString(
                FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif

    if (rxResourceId.is())
    {
        // Request deactivation of all resources linked to the specified one
        // as well.
        const Sequence<Reference<XResourceId> > aLinkedResources (
            mpImplementation->mxRequestedConfiguration->getResources(
                rxResourceId,
                OUString(),
                AnchorBindingMode_DIRECT));
        const sal_Int32 nCount (aLinkedResources.getLength());
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            // We do not add deactivation requests directly but call this
            // method recursively, so that when one time there are resources
            // linked to linked resources, these are handled correctly, too.
            requestResourceDeactivation(aLinkedResources[nIndex]);
        }

        // Add a deactivation request for the specified resource.
        Reference<XConfigurationChangeRequest> xRequest(
            new GenericConfigurationChangeRequest(
                rxResourceId,
                GenericConfigurationChangeRequest::Deactivation));
        postChangeRequest(xRequest);
    }
}




Reference<XResource> SAL_CALL ConfigurationController::getResource (
    const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    ConfigurationControllerResourceManager::ResourceDescriptor aDescriptor (
        mpImplementation->mpResourceManager->GetResource(rxResourceId));
    return aDescriptor.mxResource;
}




void SAL_CALL ConfigurationController::update (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if (mpImplementation->mpQueueProcessor->IsEmpty())
    {
        // The queue is empty.  Add another request that does nothing but
        // asynchronously trigger a request for an update.
        mpImplementation->mpQueueProcessor->AddRequest(new UpdateRequest());
    }
    else
    {
        // The queue is not empty, so we rely on the queue processor to
        // request an update automatically when the queue becomes empty.
    }
}




sal_Bool SAL_CALL ConfigurationController::hasPendingRequests (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();
    
    return ! mpImplementation->mpQueueProcessor->IsEmpty();
}





void SAL_CALL ConfigurationController::postChangeRequest (
    const Reference<XConfigurationChangeRequest>& rxRequest)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    mpImplementation->mpQueueProcessor->AddRequest(rxRequest);
}




Reference<XConfiguration> SAL_CALL ConfigurationController::getRequestedConfiguration (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if (mpImplementation->mxRequestedConfiguration.is())
        return Reference<XConfiguration>(
            mpImplementation->mxRequestedConfiguration->createClone(), UNO_QUERY);
    else
        return Reference<XConfiguration>();
}




Reference<XConfiguration> SAL_CALL ConfigurationController::getCurrentConfiguration (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    Reference<XConfiguration> xCurrentConfiguration(
        mpImplementation->mpConfigurationUpdater->GetCurrentConfiguration());
    if (xCurrentConfiguration.is())
        return Reference<XConfiguration>(xCurrentConfiguration->createClone(), UNO_QUERY);
    else
        return Reference<XConfiguration>();
}




/** The given configuration is restored by generating the appropriate set of
    activation and deactivation requests.
*/
void SAL_CALL ConfigurationController::restoreConfiguration (
    const Reference<XConfiguration>& rxNewConfiguration)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    // We will probably be making a couple of activation and deactivation
    // requests so lock the configuration controller and let it later update
    // all changes at once.
    ::boost::shared_ptr<ConfigurationUpdaterLock> pLock (
        mpImplementation->mpConfigurationUpdater->GetLock());

    // Get lists of resources that are to be activated or deactivated.
    Reference<XConfiguration> xCurrentConfiguration (mpImplementation->mxRequestedConfiguration);
#if defined VERBOSE && VERBOSE>=1
    OSL_TRACE("ConfigurationController::restoreConfiguration(\n");
    ConfigurationTracer::TraceConfiguration(rxNewConfiguration, "requested configuration");
    ConfigurationTracer::TraceConfiguration(xCurrentConfiguration, "current configuration");
#endif
    ConfigurationClassifier aClassifier (rxNewConfiguration, xCurrentConfiguration);
    aClassifier.Partition();
#if defined VERBOSE && VERBOSE>=3
    aClassifier.TraceResourceIdVector(
        "requested but not current resources:\n", aClassifier.GetC1minusC2());
    aClassifier.TraceResourceIdVector(
        "current but not requested resources:\n", aClassifier.GetC2minusC1());
    aClassifier.TraceResourceIdVector(
        "requested and current resources:\n", aClassifier.GetC1andC2());
#endif

    ConfigurationClassifier::ResourceIdVector::const_iterator iResource;

    // Request the deactivation of resources that are not requested in the
    // new configuration.
    const ConfigurationClassifier::ResourceIdVector& rResourcesToDeactivate (
        aClassifier.GetC2minusC1());
    for (iResource=rResourcesToDeactivate.begin();
         iResource!=rResourcesToDeactivate.end();
         ++iResource)
    {
        requestResourceDeactivation(*iResource);
    }

    // Request the activation of resources that are requested in the
    // new configuration but are not part of the current configuration.
    const ConfigurationClassifier::ResourceIdVector& rResourcesToActivate (
        aClassifier.GetC1minusC2());
    for (iResource=rResourcesToActivate.begin();
         iResource!=rResourcesToActivate.end();
         ++iResource)
    {
        requestResourceActivation(*iResource, ResourceActivationMode_ADD);
    }

    pLock.reset();
}




//----- XResourceFactoryManager -----------------------------------------------
    
void SAL_CALL ConfigurationController::addResourceFactory(
    const OUString& sResourceURL,
    const Reference<XResourceFactory>& rxResourceFactory)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();
    mpImplementation->mpResourceFactoryContainer->AddFactory(sResourceURL, rxResourceFactory);
}



    
void SAL_CALL ConfigurationController::removeResourceFactoryForURL(
    const OUString& sResourceURL)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();
    mpImplementation->mpResourceFactoryContainer->RemoveFactoryForURL(sResourceURL);
}




void SAL_CALL ConfigurationController::removeResourceFactoryForReference(
    const Reference<XResourceFactory>& rxResourceFactory)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();
    mpImplementation->mpResourceFactoryContainer->RemoveFactoryForReference(rxResourceFactory);
}




Reference<XResourceFactory> SAL_CALL ConfigurationController::getResourceFactory (
    const OUString& sResourceURL)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    return mpImplementation->mpResourceFactoryContainer->GetFactory(sResourceURL);
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL ConfigurationController::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    if (aArguments.getLength() == 1)
    {
        const ::vos::OGuard aSolarGuard (Application::GetSolarMutex());

        mpImplementation.reset(new Implementation(
            *this,
            Reference<frame::XController>(aArguments[0], UNO_QUERY_THROW)));
    }
}




//-----------------------------------------------------------------------------

void ConfigurationController::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
	if (mbIsDisposed)
	{
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "ConfigurationController object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }

    if (mpImplementation.get() == NULL)
    {
        OSL_ASSERT(mpImplementation.get() != NULL);
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "ConfigurationController not initialized")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//===== ConfigurationController::Implementation ===============================

ConfigurationController::Implementation::Implementation (
    ConfigurationController& rController,
    const Reference<frame::XController>& rxController)
    : mxControllerManager(rxController, UNO_QUERY_THROW),
      mpBroadcaster(new ConfigurationControllerBroadcaster(&rController)),
      mxRequestedConfiguration(new Configuration(&rController, true)),
      mpBase(NULL),
      mpResourceFactoryContainer(new ResourceFactoryManager(mxControllerManager)),
      mpResourceManager(
          new ConfigurationControllerResourceManager(mpResourceFactoryContainer,mpBroadcaster)),
      mpConfigurationUpdater(
          new ConfigurationUpdater(mpBroadcaster, mpResourceManager,mxControllerManager)),
      mpQueueProcessor(new ChangeRequestQueueProcessor(&rController,mpConfigurationUpdater)),
      mpConfigurationUpdaterLock(),
      mnLockCount(0)
{
    mpQueueProcessor->SetConfiguration(mxRequestedConfiguration);
}




ConfigurationController::Implementation::~Implementation (void)
{
}




} } // end of namespace sd::framework
