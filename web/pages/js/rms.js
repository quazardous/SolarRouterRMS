
class ConfigParam {
    constructor(group, type, name, value, readonly, dirty, label, help) {
        this.group = group;
        this.type = type;
        this.name = name;
        this.value = value;
        this.readonly = readonly;
        this.dirty = dirty;
        this.label = label;
        this.help = help;
        if (!this.label) {
            this.label = this.getLabel();
        }
    }

    getLabel() {
        return this.name.replace(/([a-z])([A-Z])/g, '$1 $2').replace(/^(.)(.*)$/, (_, first, rest) => first.toUpperCase() + rest);
    }
}

class ConfigParamGroup {
    constructor(name) {
        this.name = name;
        /**
         * @var {Object.<String, ConfigParam>}
         */
        this.params = {};
    }

    addParam(name, param) {
        this.params[name] = new ConfigParam(
            this.name,
            param.type,
            name,
            param.value,
            param.readonly,
            param.dirty,
            param.label,
            param.help);
    }
}

class SolarRouterRMS {
    static DEFAULT_AP_IP = '192.168.4.1';
    static LOOP_INTERVAL = 3000;

    static MODE_AUTO = 'auto';
    static MODE_UNKNOWN = 'unknown';
    static MODE_DIRECT = 'direct';
    static MODE_AP = 'ap';
    static MODE_STATION = 'station';

    constructor(configs = {}) {
        this.configs = configs;
        this.localConfig = new LocalConfig();
        this.configParams = {};
        /**
         * @var {Object.<String, ConfigParamGroup>}
         */
        this.configParamsGroups = {};
        const self = this;
        // Offshore Mode allows to use a local version of the admin panel with a remote RMS
        // This is useful for development and testing
        // Normally the admin panel is served by the RMS
        this.offshore = false;
        this.mode = SolarRouterRMS.MODE_UNKNOWN;
        /**
         * @var {RMSAPI}
         */
        this.api = new RMSAPI(() => {
            switch (self.mode) {
                case SolarRouterRMS.MODE_DIRECT:
                    return '';
                case SolarRouterRMS.MODE_AP:
                    return `http://${self.getAPIp()}/`;
                case SolarRouterRMS.MODE_STATION:
                    return `http://${self.getStationIp()}/`;
                default:
                    throw new Error(`Invalid mode ${self.mode}`);
            }
        });

        this.hello = {};
        this.hellok = false;

        this.configParamsDone = false;
    }

    addConfigParam(name, param) {
        if (!this.configParamsGroups[param.group]) {
            this.configParamsGroups[param.group] = new ConfigParamGroup(param.group);
        }
        this.configParamsGroups[param.group].addParam(name, param);
        this.configParams[name] = param;
    }

    setConfig(name, value) {
        this.configs[name] = value;
    }

    getConfig(name) {
        return this.configs[name];
    }

    hasLocalConfig() {
        return !LocalConfig.empty(this.localConfig.getData('stationIp'));
    }

    queryHello(mode = SolarRouterRMS.MODE_AUTO) {
        let p = null;
        switch (mode) {
            case SolarRouterRMS.MODE_AUTO:
                p = this.api.get('api/hello');
                break;
            case SolarRouterRMS.MODE_DIRECT:
                p = (new RMSAPI()).get('api/hello');
                break;
            case SolarRouterRMS.MODE_AP:
                if (LocalConfig.empty(this.getAPIp())) {
                    return Promise.reject(new Error('No AP IP'));
                }
                p = (new RMSAPI(`http://${this.getAPIp()}/`)).get('api/hello');
                break;
            case SolarRouterRMS.MODE_STATION:
                p = (new RMSAPI(`http://${this.getStationIp()}/`)).get('api/hello');
                break;
            default:
                throw new Error(`Invalid mode ${mode}`);
        }
        return p.then(data => {
            if ([SolarRouterRMS.MODE_DIRECT, SolarRouterRMS.MODE_AP, SolarRouterRMS.MODE_STATION].includes(mode)) {
                this.setMode(mode);
            }
            this.hello = data;
            this.hellok = true;
            return data;
        }).catch(error => {
            this.hellok = false;
            throw error;
        });
    }

    guessHelloOffshore() {
        const promises = [];
        promises.push(this.queryHello(SolarRouterRMS.MODE_STATION)
            .catch(error => {
                console.info('Error checking offshore station', error);
                throw error;
            })
            .then(data => {
                console.log('Hello Offshore Station:', data);
                return {mode: SolarRouterRMS.MODE_STATION, data};
            }));
        promises.push(this.queryHello(SolarRouterRMS.MODE_AP)
            .catch(error => {
                console.info('Error checking offshore AP', error);
                throw error;
            })
            .then(data => {
                console.log('Hello Offshore AP:', data);
                return {mode: SolarRouterRMS.MODE_AP, data};
            }));
        try {
            return Promise.any(promises).then(data => {
                console.log('Guessing Offshore Mode:', SolarRouterRMS.MODE_AP);
                return data;
            });
        } catch (error) {
            return Promise.reject(error);
        }

    }

    setOffshore(offshore = true) {
        this.offshore = offshore;
    }

    setMode(mode) {
        this.mode = mode;
        if (mode === SolarRouterRMS.MODE_DIRECT) {
            this.setOffshore(false);
        } else if ([SolarRouterRMS.MODE_AP, SolarRouterRMS.MODE_STATION].includes(mode)) {
            this.setOffshore(true);
        }
    }

    setOffshoreIps(stationIp, apIp) {
        this.setStationIp(stationIp);
        this.setAPIp(apIp);
    }

    getConfigs() {
        return this.configs;
    }

    getStationIp() {
        let ip = this.localConfig.getData('stationIp');
        return LocalConfig.empty(ip) ? '' : ip;
    }
    setStationIp(ip) {
        this.localConfig.saveData('stationIp', ip);
    }

    getAPIp() {
        const ip = this.localConfig.getData('apIp');
        return LocalConfig.empty(ip) ? SolarRouterRMS.DEFAULT_AP_IP : ip;
    }

    setAPIp(ip) {
        if (SolarRouterRMS.DEFAULT_AP_IP === ip) {
            this.localConfig.removeData('apIp');
            return;
        }
        this.localConfig.saveData('apIp', ip);
    }

    /**
     * Check RMS hello/connectivity (Offshore or direct).
     * Emit signals for the UI.
     */
    checkHello() {
        const self = this;
        if (this.offshore && this.mode === SolarRouterRMS.MODE_UNKNOWN) {
            console.log('Guessing Offshore Mode');
            return this.guessHelloOffshore()
                .then(data => {
                    self.emit('rms.hello', {rms:self, hello: data.data, mode: data.mode});
                    return true;
                })
                .catch(error => {
                    self.emit('rms.hello', {rms:self, hello: false, mode: this.mode});
                }); 
        } else {
            if ([SolarRouterRMS.MODE_DIRECT, SolarRouterRMS.MODE_AP, SolarRouterRMS.MODE_STATION].includes(this.mode)) {
                return this.queryHello()
                    .then(data => {
                        self.emit('rms.hello', {rms:self, mode: this.mode, hello: data});
                        return true;
                    })
                    .catch(error => {
                        self.emit('rms.hello', {rms:self, mode: this.mode, hello: false});
                    });
            }
        }
        return Promise.resolve(false);
    }

    startLoops() {
        const self = this;
        setInterval(() => {
            this.loop();
        }, SolarRouterRMS.LOOP_INTERVAL);
    }

    /**
     * @param {String} event 
     * @param {*} data 
     */
    emit(event, data) {
        console.log('Emit:', event, data);
        const e = new CustomEvent(event, { detail: data });
        window.dispatchEvent(e);
    }

    start() {
        this.startLoops();
    }

    loop() {
        const self = this;
        this.checkHello().then(hello => {
            if (hello && ! self.configParamsDone) {
                self.configParamsDone = true;
                self.queryConfigParams();
            }
        });
    }

    queryConfigParams() {
        this.api.get('api/config').then(data => {
            for (const key in data.configs) {
                this.addConfigParam(key, data.configs[key]);
            }
            this.emit('rms.config', {rms: self, config: data.configs});
        });
    }
}


