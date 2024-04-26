const DEFAULT_AP_IP = '192.168.1.4'

class SolarRouterRMS {
    constructor(configs = {}) {
        this.configs = configs;
        this.localConfig = new LocalConfig();
        const self = this;
        // Offshore Mode allows to use a local version of the admin panel with a remote RMS
        // This is useful for development and testing
        // Normally the admin panel is served by the RMS
        this.offshoreMode = false;
        this.api = new RMSAPI(() => {
            if (this.offshoreMode) {
                return `http://${self.getStationIp()}/`;
            }
            return '';
        });
    }

    setConfig(name, value) {
        this.configs[name] = value;
    }

    getConfig(name) {
        return this.configs[name];
    }

    hasLocalConfig() {
        return !this.localConfig.empty(this.localConfig.getData('stationIp'));
    }

    checkHello() {
        return this.api.get('api/hello');
    }

    setOffshoreMode(offshore = true) {
        this.offshoreMode = offshore;
    }

    setOffshoreModeIps(stationIp, apIp) {
        this.setStationIp(stationIp);
        this.setAPIp(apIp);
    }

    getConfigs() {
        return this.configs;
    }

    getStationIp() {
        let ip = this.localConfig.getData('stationIp');
        return this.localConfig.empty(ip) ? '' : ip;
    }
    setStationIp(ip) {
        this.localConfig.saveData('stationIp', ip);
    }
    getAPIp() {
        const ip = this.localConfig.getData('apIp');
        return this.localConfig.empty(ip) ? DEFAULT_AP_IP : ip;
    }
    setAPIp(ip) {
        if (DEFAULT_AP_IP === ip) {
            this.removeData('apIp');
            return;
        }
        this.localConfig.saveData('apIp', ip);
    }
}


