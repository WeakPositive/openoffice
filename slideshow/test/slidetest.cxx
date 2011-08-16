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

#include <testshl/simpleheader.hxx>
#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <cppcanvas/spritecanvas.hxx>

#include "view.hxx"
#include "unoview.hxx"
#include "unoviewcontainer.hxx"
#include "shape.hxx"
#include "tests.hxx"
#include "../engine/slide/layermanager.hxx"
#include "../engine/slide/layer.hxx"
#include "com/sun/star/presentation/XSlideShowView.hpp"

namespace target = slideshow::internal;
using namespace ::com::sun::star;

namespace
{

class LayerManagerTest : public CppUnit::TestFixture
{
    target::UnoViewContainer      maViews;
    target::LayerManagerSharedPtr mpLayerManager;
    TestViewSharedPtr             mpTestView;
    TestShapeSharedPtr            mpTestShape;

public:
    void setUp()
    {
        mpTestShape = createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            1.0);
        mpTestView = createTestView();
        maViews.addView( mpTestView );

        mpLayerManager.reset(
            new target::LayerManager(
                maViews,
                basegfx::B2DRange(0.0,0.0,100.0,100.0),
                false ));
    }

    void tearDown()
    {
        mpLayerManager.reset();
        maViews.dispose();
    }

    void testLayer()
    {
        target::LayerSharedPtr pBgLayer(
            target::Layer::createBackgroundLayer( basegfx::B2DRange(0,0,100,100) ) );
        pBgLayer->addView( mpTestView );

        target::LayerSharedPtr pFgLayer(
            target::Layer::createLayer( basegfx::B2DRange(0,0,100,100) ) );
        pFgLayer->addView( mpTestView );

        CPPUNIT_ASSERT_MESSAGE( "BG layer must confess that!",
                                pBgLayer->isBackgroundLayer() );
        CPPUNIT_ASSERT_MESSAGE( "FG layer lies!",
                                !pFgLayer->isBackgroundLayer() );

        CPPUNIT_ASSERT_MESSAGE( "BG layer must not have pending updates!",
                                !pBgLayer->isUpdatePending() );
        pBgLayer->addUpdateRange( basegfx::B2DRange(0,0,10,10) );
        CPPUNIT_ASSERT_MESSAGE( "BG layer must have pending updates!",
                                pBgLayer->isUpdatePending() );

        TestShapeSharedPtr pTestShape = createTestShape(
            basegfx::B2DRange(0.0,0.0,1000.0,1000.0),
            1.0);
        pBgLayer->updateBounds( pTestShape );
        CPPUNIT_ASSERT_MESSAGE( "BG layer must not resize!",
                                !pBgLayer->commitBounds() );

        TestShapeSharedPtr pTestShape2 = createTestShape(
            basegfx::B2DRange(0.0,0.0,1.0,1.0),
            1.0);
        pFgLayer->updateBounds( pTestShape2 );
        CPPUNIT_ASSERT_MESSAGE( "FG layer must resize!",
                                pFgLayer->commitBounds() );
    }

    void testBasics()
    {
        mpLayerManager->activate( false );

        CPPUNIT_ASSERT_MESSAGE( "Un-added shape must have zero view layers",
                                mpTestShape->getViewLayers().empty() );
        mpLayerManager->addShape(mpTestShape);
        CPPUNIT_ASSERT_MESSAGE( "Adding a shape requires a LayerManager update",
                                mpLayerManager->isUpdatePending() );

        // update does the delayed viewAdded call to the shape
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "Added shape must have one view layer",
                                mpTestShape->getViewLayers().size() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape must been rendered",
                                mpTestShape->getNumRenders() );
        CPPUNIT_ASSERT_MESSAGE( "Shape must not been updated",
                                !mpTestShape->getNumUpdates() );

        // test second view, check whether shape gets additional view
        TestViewSharedPtr pTestView( createTestView() );
        CPPUNIT_ASSERT_MESSAGE( "Adding second View failed",
                                maViews.addView( pTestView ) );
        CPPUNIT_ASSERT_MESSAGE( "View container must have two views",
                                maViews.end() - maViews.begin() == 2 );
        mpLayerManager->viewAdded(pTestView);
        CPPUNIT_ASSERT_MESSAGE( "Added shape must have two view layers",
                                mpTestShape->getViewLayers().size() == 2 );

        CPPUNIT_ASSERT_MESSAGE( "Removing second View failed",
                                maViews.removeView( pTestView ) );
        mpLayerManager->viewRemoved(pTestView);
        CPPUNIT_ASSERT_MESSAGE( "Added shape must have one view layer",
                                mpTestShape->getViewLayers().size() == 1 );

        mpLayerManager->deactivate();
    }

    void testShapeOrdering()
    {
        TestShapeSharedPtr pShape2( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            2.0));
        TestShapeSharedPtr pShape3( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            3.0));
        TestShapeSharedPtr pShape4( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            4.0));

        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);

        mpLayerManager->activate( false );

        // update does the delayed viewAdded call to the shape
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "View must have background layer only",
                                mpTestView->getViewLayers().empty() );

        // LayerManager must now generate one extra view layer
        mpLayerManager->enterAnimationMode(pShape2);
        CPPUNIT_ASSERT_MESSAGE( "No update pending on LayerManager",
                                mpLayerManager->isUpdatePending() );
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "View must have one extra layer only",
                                mpTestView->getViewLayers().size() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "View layer must have 10x10 size",
                                mpTestView->getViewLayers().at(0)->getBounds() ==
                                basegfx::B2DRange(0.0,0.0,10.0,10.0) );

        // LayerManager must now remove the extra view layer
        mpLayerManager->leaveAnimationMode(pShape2);
        CPPUNIT_ASSERT_MESSAGE( "No update pending on LayerManager",
                                mpLayerManager->isUpdatePending() );
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager #2",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "Shape 1 must be on background layer", 
                                mpTestShape->getViewLayers().at(0).first == mpTestView );
        CPPUNIT_ASSERT_MESSAGE( "Shape 2 must be on background layer", 
                                pShape2->getViewLayers().at(0).first == mpTestView );
        CPPUNIT_ASSERT_MESSAGE( "Shape 3 must have one layer", 
                                pShape3->getViewLayers().size() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape 3 must be on background layer", 
                                pShape3->getViewLayers().at(0).first == mpTestView );
        CPPUNIT_ASSERT_MESSAGE( "Shape 4 must be on background layer", 
                                pShape4->getViewLayers().at(0).first == mpTestView );

        // checking deactivation (all layers except background layer
        // must vanish)
        mpLayerManager->enterAnimationMode(pShape3);
        CPPUNIT_ASSERT_MESSAGE( "No update pending on LayerManager", 
                                mpLayerManager->isUpdatePending() );
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager", 
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "Shape 4 must not be on background layer", 
                                pShape4->getViewLayers().at(0).first != mpTestView );
        mpLayerManager->leaveAnimationMode(pShape3);
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager", 
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "Shape 4 must be on background layer", 
                                pShape4->getViewLayers().at(0).first == mpTestView );

        mpLayerManager->deactivate();
        CPPUNIT_ASSERT_MESSAGE( "Update pending on deactivated LayerManager", 
                                !mpLayerManager->isUpdatePending() );
    }

    void testShapeRepaint()
    {
        TestShapeSharedPtr pShape2( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            2.0));
        TestShapeSharedPtr pShape3( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            3.0));
        TestShapeSharedPtr pShape4( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            4.0));
        TestShapeSharedPtr pShape5( createTestShape(
            basegfx::B2DRange(20.0,20.0,30.0,30.0),
            4.0));

        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->enterAnimationMode(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);
        mpLayerManager->addShape(pShape5);

        mpLayerManager->activate( false );
        mpLayerManager->update();

        CPPUNIT_ASSERT_MESSAGE( "First shape not rendered", 
                                mpTestShape->getNumRenders() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Second shape not rendered", 
                                pShape2->getNumRenders() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Third shape not rendered", 
                                pShape3->getNumRenders() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Fourth shape not rendered", 
                                pShape4->getNumRenders() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Fifth shape not rendered", 
                                pShape5->getNumRenders() == 1 );
        
        mpLayerManager->enterAnimationMode(pShape4);
        mpLayerManager->update();

        CPPUNIT_ASSERT_MESSAGE( "First shape not rendered", 
                                mpTestShape->getNumRenders() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Second shape not rendered", 
                                pShape2->getNumRenders() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Third shape not rendered", 
                                pShape3->getNumRenders() == 2 );
        CPPUNIT_ASSERT_MESSAGE( "Fourth shape not rendered", 
                                pShape4->getNumRenders() == 2 );
        CPPUNIT_ASSERT_MESSAGE( "Fifth shape not rendered", 
                                pShape5->getNumRenders() == 2 );

        mpLayerManager->leaveAnimationMode(pShape2);
        mpLayerManager->leaveAnimationMode(pShape4);
        mpLayerManager->update();

        CPPUNIT_ASSERT_MESSAGE( "First shape not rendered #2", 
                                mpTestShape->getNumRenders() == 2 );
        CPPUNIT_ASSERT_MESSAGE( "Second shape not rendered #2", 
                                pShape2->getNumRenders() == 2 );
        CPPUNIT_ASSERT_MESSAGE( "Third shape not rendered #2", 
                                pShape3->getNumRenders() == 3 );
        CPPUNIT_ASSERT_MESSAGE( "Fourth shape not rendered #2", 
                                pShape4->getNumRenders() == 3 );        
        CPPUNIT_ASSERT_MESSAGE( "Fifth shape not rendered #2", 
                                pShape5->getNumRenders() == 3 );        
    }

    void testRefCounting()
    {
        TestShapeSharedPtr pShape2( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            2.0));
        TestShapeSharedPtr pShape3( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            3.0));
        TestShapeSharedPtr pShape4( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            4.0));

        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);

        mpLayerManager->removeShape(mpTestShape);
        mpLayerManager->removeShape(pShape2);
        mpLayerManager->removeShape(pShape3);
        mpLayerManager->removeShape(pShape4);

        CPPUNIT_ASSERT_MESSAGE( "Shape 1 must have refcount of 1", 
                                mpTestShape.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape 2 must have refcount of ", 
                                pShape2.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape 3 must have refcount of 1", 
                                pShape3.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape 4 must have refcount of", 
                                pShape4.use_count() == 1 );


        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);

        mpLayerManager->activate( false );
        mpLayerManager->update();

        mpLayerManager->removeShape(mpTestShape);
        mpLayerManager->removeShape(pShape2);
        mpLayerManager->removeShape(pShape3);
        mpLayerManager->removeShape(pShape4);

        CPPUNIT_ASSERT_MESSAGE( "Shape 1 must have refcount of 1", 
                                mpTestShape.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape 2 must have refcount of ", 
                                pShape2.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape 3 must have refcount of 1", 
                                pShape3.use_count() == 1 );
        CPPUNIT_ASSERT_MESSAGE( "Shape 4 must have refcount of 1", 
                                pShape4.use_count() == 1 );

        maViews.removeView(mpTestView);
        mpLayerManager->viewRemoved(mpTestView);
        CPPUNIT_ASSERT_MESSAGE( "View must have refcount of 1", 
                                mpTestView.use_count() == 1 );
    }

    // hook up the test
    CPPUNIT_TEST_SUITE(LayerManagerTest);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST(testLayer);
    CPPUNIT_TEST(testShapeOrdering);
    CPPUNIT_TEST(testShapeRepaint);
    CPPUNIT_TEST(testRefCounting);
    CPPUNIT_TEST_SUITE_END();

}; // class LayerManagerTest

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(LayerManagerTest, "LayerManagerTest");
} // namespace


