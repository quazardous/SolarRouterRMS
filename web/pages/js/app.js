// Initiate the app
// all global variables are defined here

// entrypoint to access RMS
const rms = new SolarRouterRMS();

// proxies between RMS and DOM
// Info Box
const rmsInfoBoxProxy = rmsInfoBox();

// popups
const multiPopup = MultiPopup.factory('multi-popup');
multiPopup.registerTemplate('init', new PopupInitTpl());

// listeners
businessListeners();
