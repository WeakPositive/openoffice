package complex.imageManager;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.ui.XImageManager;
import com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import com.sun.star.ui.XUIConfiguration;
import com.sun.star.ui.XUIConfigurationManager;
import com.sun.star.ui.XUIConfigurationPersistence;


// ---------- junit imports -----------------
import lib.TestParameters;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

/**
 *
 */
public class CheckImageManager  {
    boolean checkUIConfigManager = false;
    XMultiServiceFactory xMSF = null;
    /**
     * The test parameters
     */
    private static TestParameters param = null;

    @Before public void before()
    {
        xMSF = getMSF();
        param = new TestParameters();
        param.put("ServiceFactory", xMSF); // some qadevOOo functions need the ServiceFactory
    } 
    
//    public String[] getTestMethodNames() {
//        return new String[]{"checkImageManagerFromModule"};//, "checkImageManager"};
//    }
    
    @Test public void checkImageManagerFromModule()
    {
        System.out.println(" **** ImageManager from ModuleUIConfigurationManager *** ");
        XUIConfigurationManager xManager = null;
        try {
            Object o = (XInterface)xMSF.createInstance(
                    "com.sun.star.ui.ModuleUIConfigurationManagerSupplier");
            XModuleUIConfigurationManagerSupplier xMUICMS = 
                    UnoRuntime.queryInterface(XModuleUIConfigurationManagerSupplier.class, o);
            xManager = xMUICMS.getUIConfigurationManager(
                    "com.sun.star.text.TextDocument");
        }
        catch(com.sun.star.uno.Exception e) {
            fail("Exception. " + e.getMessage());
        }
        XImageManager xImageManager = UnoRuntime.queryInterface(XImageManager.class, xManager.getImageManager());
        performChecks(xImageManager, "ModuleUIConfig", xManager);
    }

    public void checkImageManager() {
        System.out.println(" **** ImageManager from UIConfigurationManager *** ");
        XUIConfigurationManager xManager = null;
        try {
            xManager = UnoRuntime.queryInterface(XUIConfigurationManager.class, xMSF.createInstance("com.sun.star.comp.framework.UIConfigurationManager"));
        }
        catch(com.sun.star.uno.Exception e) {
            fail("Exception. " + e.getMessage());
        }

        XImageManager xImageManager = UnoRuntime.queryInterface(XImageManager.class, xManager.getImageManager());
        performChecks(xImageManager, "UIConfig", xManager);
    }
    
    private void performChecks(XImageManager xImageManager, String testObjectName, XUIConfigurationManager xManager) {
        util.dbg.printInterfaces(xImageManager);

        OXUIConfigurationListenerImpl configListener = new OXUIConfigurationListenerImpl(xManager, xMSF);
        param.put("XUIConfiguration.XUIConfigurationListenerImpl", configListener);
        
        XInitialization xInit = UnoRuntime.queryInterface(XInitialization.class, xImageManager);
        _XInitialization _xInit = new _XInitialization(param, xInit);
        assertTrue(testObjectName + "::XInitialization.initialize", _xInit._initialize());
        
        // xImageManager is already there, just write a test ;-)
        _XImageManager _xImage = new _XImageManager(param, xImageManager);
        assertTrue(testObjectName + "::XImageManager.getAllImageNames", _xImage._getAllImageNames());
        assertTrue(testObjectName + "::XImageManager.getImages", _xImage._getImages());
        assertTrue(testObjectName + "::XImageManager.hasImage", _xImage._hasImage());
        assertTrue(testObjectName + "::XImageManager.insertImages", _xImage._insertImages());
        assertTrue(testObjectName + "::XImageManager.removeImages", _xImage._removeImages());
        assertTrue(testObjectName + "::XImageManager.replaceImages", _xImage._replaceImages());
        assertTrue(testObjectName + "::XImageManager.reset", _xImage._reset());
        
        XTypeProvider xType = UnoRuntime.queryInterface(XTypeProvider.class, xImageManager);
        _XTypeProvider _xType = new _XTypeProvider(param, xType);
        assertTrue(testObjectName + "::XTypeProvider.getImplementationId", _xType._getImplementationId());
        assertTrue(testObjectName + "::XTypeProvider.getTypes", _xType._getTypes());
        
        XUIConfiguration xUIConfig = UnoRuntime.queryInterface(XUIConfiguration.class, xImageManager);
        _XUIConfiguration _xUIConfig = new _XUIConfiguration(param, xUIConfig);
        _xUIConfig.before();
        assertTrue(testObjectName + "::XUIConfig.addConfigurationListener", _xUIConfig._addConfigurationListener());
        assertTrue(testObjectName + "::XUIConfig.removeConfigurationListener", _xUIConfig._removeConfigurationListener());
        
        XUIConfigurationPersistence xUIConfigPersistence = (XUIConfigurationPersistence)UnoRuntime.queryInterface(XUIConfiguration.class, xImageManager);
        _XUIConfigurationPersistence _xUIConfigPersistence = new _XUIConfigurationPersistence(param, xUIConfigPersistence);
        _xUIConfigPersistence.before();
        assertTrue(testObjectName + "::XUIConfigPersistence.isModified", _xUIConfigPersistence._isModified());
        // System.out.println(testObjectName + "::XUIConfigPersistence.isReadOnly "+ _xUIConfigPersistence._isReadOnly());
        assertTrue(testObjectName + "::XUIConfigPersistence.isReadOnly", _xUIConfigPersistence._isReadOnly());
        assertTrue(testObjectName + "::XUIConfigPersistence.reload", _xUIConfigPersistence._reload());
        assertTrue(testObjectName + "::XUIConfigPersistence.store", _xUIConfigPersistence._store());
        assertTrue(testObjectName + "::XUIConfigPersistence.storeToStorage", _xUIConfigPersistence._storeToStorage());
        
        XComponent xComp = UnoRuntime.queryInterface(XComponent.class, xImageManager);
        _XComponent _xComp = new _XComponent(param, xComp);
        _xComp.before();
        assertTrue(testObjectName + "::XComponent.addEventListener", _xComp._addEventListener());
        assertTrue(testObjectName + "::XComponent.removeEventListener", _xComp._removeEventListener());
        assertTrue(testObjectName + "::XComponent.dispose", _xComp._dispose());
    }
    
    
    class OXUIConfigurationListenerImpl implements _XUIConfiguration.XUIConfigurationListenerImpl {
        private boolean triggered = false;
        private XUIConfigurationManager xUIManager = null;
        private XMultiServiceFactory xMSF = null;
        
        public OXUIConfigurationListenerImpl(XUIConfigurationManager xUIManager, XMultiServiceFactory xMSF) {

            this.xUIManager = xUIManager;
            this.xMSF = xMSF;
        }
        
        public boolean actionWasTriggered() {
            return triggered;
        }
        
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            triggered = true;
        }
        
        public void elementInserted(com.sun.star.ui.ConfigurationEvent configurationEvent) {
            triggered = true;
        }
        
        public void elementRemoved(com.sun.star.ui.ConfigurationEvent configurationEvent) {
            triggered = true;
        }
        
        public void elementReplaced(com.sun.star.ui.ConfigurationEvent configurationEvent) {
            triggered = true;
        }
        
        public void fireEvent() {
            // remove for real action: 
            triggered = !triggered;
/*            try {
                XIndexAccess xMenuBarSettings = xUIManager.getSettings(
                                        "private:resource/menubar/menubar", true);

            }
            catch(com.sun.star.container.NoSuchElementException e) {
                System.out.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.IllegalAccessException e) {
                System.out.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                System.out.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                System.out.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            } */
        }
        
        public void reset() {
            // remove comment for real function
            //triggered = false;
        }
        
    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();

}
