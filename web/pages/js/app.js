// Initiate the app

document.addEventListener('DOMContentLoaded', () => {
    rms.checkHello().then(data => {
        console.log('Hello:', data);
    }).catch(error => {
        // seams we are running in offline mode aka from local file system or another server
        // rms.setOfflineMode();
        rmsProxy.setOfflineMode();
        console.error('Error checking hello:', error);
        if (!rms.hasLocalConfig()) {
            console.log('Local storage has not been initialized.');
            multiPopup.popup('init');
        }
    });
});