// Initiate the app

const rmsInfoBoxProxy = rmsInfoBox();

multiPopup.registerTemplate('init', new PopupInitTpl());

document.addEventListener('DOMContentLoaded', () => {
    rms.checkHello().then(data => {
        console.log('Hello:', data);
    }).catch(error => {
        // seams we are running in offshore mode aka from local file system or another server
        // rms.setOffshoreMode();
        rmsInfoBoxProxy.setOffshoreMode();
        console.error('Error checking hello:', error);
        if (!rms.hasLocalConfig()) {
            console.log('Local storage has not been initialized.');
            multiPopup.popup('init');
        }
    });
});