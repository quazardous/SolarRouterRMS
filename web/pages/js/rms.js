const DEFAULT_AP_IP = '192.168.1.4'

class LocalConfig {
    saveData(key, data) {
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
        return this.getData('stationIp');
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

    get(endpoint) {
        return new Promise((resolve, reject) => {
            fetch(`${this.baseUrl}${endpoint}`)
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
            fetch(`${this.baseUrl}${endpoint}`, {
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
        this.api = new RMSAPI();
        this.offlineMode = false;
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

    setOfflineMode(offline = true) {
        this.offlineMode = offline;
    }

    setOfflineModeIps(stationIp, apIp) {
        this.localConfig.setStationIp(stationIp);
        this.localConfig.setAPIp(apIp);
    }

    getConfigs() {
        return this.configs;
    }
}

const rms = new SolarRouterRMS();