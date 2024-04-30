// Initiate the app
// all global variables are defined here

// entrypoint to access RMS
const rms = new SolarRouterRMS();

// proxies between RMS and DOM
// Info Box
const infoBoxProxy = proxyInfoBox();

// Config Forms
const configFormsProxy = proxyConfigForms();

// popups
MultiPopup.addListeners();
const multiPopup = MultiPopup.factory('multi-popup');
multiPopup.registerTemplate('init', new PopupInitTpl());

// listeners
businessListeners();
