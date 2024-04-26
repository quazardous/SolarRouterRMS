const DEFAULT_AP_IP = '192.168.1.4'

class LocalConfig {
    saveData(key, data) {
        if (this.empty(data)) {
            this.removeData(key);
            return;
        }
        localStorage.setItem(key, JSON.stringify(data));
    }
    getData(key) {
        const data = localStorage.getItem(key);
        return JSON.parse(data);
    }
    removeData(key) {
        localStorage.removeItem(key);
    }
    clearAllData() {
        localStorage.clear();
    }
    getStationIp() {
        let ip = this.getData('stationIp');
        return this.empty(ip) ? '' : ip;
    }
    setStationIp(ip) {
        this.saveData('stationIp', ip);
    }
    getAPIp() {
        const ip = this.getData('apIp');
        return this.empty(ip) ? DEFAULT_AP_IP : ip;
    }
    setAPIp(ip) {
        if (DEFAULT_AP_IP === ip) {
            this.removeData('apIp');
            return;
        }
        this.saveData('apIp', ip);
    }
    empty(val) {

        // test results
        //---------------
        // []        true, empty array
        // {}        true, empty object
        // null      true
        // undefined true
        // ""        true, empty string
        // ''        true, empty string
        // 0         false, number
        // true      false, boolean
        // false     false, boolean
        // Date      false
        // function  false
    
        if (val === undefined)
            return true;
    
        if (typeof (val) == 'function' || typeof (val) == 'number' || typeof (val) == 'boolean' || Object.prototype.toString.call(val) === '[object Date]')
            return false;
    
        if (val == null || val.length === 0)        // null or 0 length array
            return true;
    
        if (typeof (val) == "object") {
            // empty object
            var r = true;
            for (var f in val)
                r = false;
            return r;
        }
    
        return false;
    }
}

class RMSAPI {
    constructor(baseUrl = '/') {
        this.baseUrl = baseUrl;
    }

    url(endpoint) {
        let url;
        if (typeof this.baseUrl === 'function') {
            url = this.baseUrl(endpoint);
        } else {
            url = this.baseUrl;
        }
        return `${url}${endpoint}`;
    }

    get(endpoint) {
        return new Promise((resolve, reject) => {
            fetch(this.url(endpoint))
                .then(response => response.json())
                .then(data => resolve(data))
                .catch(error => {
                    console.error('Error fetching data:', error);
                    reject(error);
                });
        });
    }

    post(endpoint, payload) {
        return new Promise((resolve, reject) => {
            fetch(this.url(endpoint), {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(payload)
            })
                .then(response => response.json())
                .then(data => resolve(data))
                .catch(error => {
                    console.error('Error posting data:', error);
                    reject(error);
                });
        });
    }
}

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
                return `http://${self.localConfig.getStationIp()}:3000/`;
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
        this.localConfig.setStationIp(stationIp);
        this.localConfig.setAPIp(apIp);
    }

    getConfigs() {
        return this.configs;
    }
}

const rms = new SolarRouterRMS();