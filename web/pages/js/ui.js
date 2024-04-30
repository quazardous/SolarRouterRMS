// UI business class and UI component factories

// popup for RMS initialization (Mode Offshore / Mode Direct)
class PopupInitTpl extends PopupTpl {
    constructor() {
        super();
        this.form = new ManagedForm('form-init', (data) => {
            infoBoxProxy.setOffshoreIps(data['form-init-stationIp'], data['form-init-apIp']);
            this.popup.close();
            return;
        });
        this.form.addInput(new ManagedInput('form-init-stationIp', (value, event) => {
            const data = this.store();
            data.set('localIpClass', IPAddress.validateIP(value) ? 'valid' : 'invalid');
        }));
        this.form.addInput(new ManagedInput('form-init-apIp', (value, event) => {
            const data = this.store();
            data.set('apIpClass', value.trim() === '' || IPAddress.validateIP(value) ? 'valid' : 'invalid');
        }));
        this.form.addInput(new ManagedButton('form-init-reset', (confirmed, event) => {
            if (confirmed) {
                infoBoxProxy.resetOffshore();
                this.popup.close();
            }
            event.preventDefault();
        }, 'Are you sure you want to reset the IPs?'));
    }

    data() {
        return {
            localIpClass: rms.getStationIp() ? 'valid' : 'invalid',
            apIpClass: '',
        };
    }

    template() {
        const store = this.store();
        const localIp = new IpFormInputHelper('form-init-stationIp', rms.getStationIp());
        localIp.required = true;
        localIp.label = 'Local IP';
        localIp.help = 'Provide the IP on your local network';
        localIp.attr.class = store.value.localIpClass;
        // localIp.attr.autocomplete = 'off';
        const apIp = new IpFormInputHelper('form-init-apIp', rms.getAPIp());
        apIp.label = 'AP IP';
        apIp.help = 'Provide the IP when in AP mode. Empty for default.';
        apIp.attr.class = store.value.apIpClass;
        apIp.attr.placeholder = SolarRouterRMS.DEFAULT_AP_IP;
        // apIp.attr.autocomplete = 'off';
        console.log('Render form init', rms.getStationIp(), rms.getAPIp());
        return `
<h2>Initialize RMS</h2>
<p>It seams this admin is not running on the RMS.<br> We need to kown the IP of the RMS.</p>
<form id="form-init">
<fieldset>
<p>
${localIp.html()}
</p>
<p>
${apIp.html()}
</p>
<p><button>Save</button><button class="confirm" id="form-init-reset">Reset</button></p>
</fieldset>
</form>
`;
    }

    onShow() {
        this.form.addListeners();
    }

    onClose() {
        this.form.removeListeners();
        rms.loop();
    }
}

// factories

// Info Box Proxy
// Proxies are essentially kitchen to handle Reef JS stuff
/**
 * @typedef {Object} InfoBoxProxy
 * @property {function(string, any): void} set - Sets the value of a key in the data object and updates the corresponding RMS configuration.
 * @property {function(): void} setMode - Sets the direct mode and updates the corresponding RMS configuration.
 * @property {function(): void} setOffshore - Sets the offshore mode and updates the corresponding RMS configuration.
 * @property {function(): void} resetOffshore - Resets the offshore mode and related IPs.
 * @property {function(string, string): void} setOffshoreIps - Sets the offshore mode and the station IP and AP IP.
 */

/**
 * @returns {InfoBoxProxy}
 */
function proxyInfoBox() {
    const proxyData = () => {
        return {
            mode: rms.mode,
            offshore: rms.offshore,
            stationIp: rms.getStationIp(),
            apIp: rms.getAPIp(),
            hello: rms.hellok,
        };
    };

    const rmsProxy = store(proxyData(), {
        set(data, key, value) {
            rms.setConfig(key, value);
            Object.assign(data, proxyData());
        },
        setMode(data, mode) {
            rms.setMode(mode);
            Object.assign(data, proxyData());
        },
        setOffshore(data) {
            rms.setOffshore(true);
            Object.assign(data, proxyData());
        },
        resetOffshore(data) {
            rms.setOffshore(true);
            rms.setOffshoreIps(null, null);
            Object.assign(data, proxyData());
        },
        setOffshoreIps(data, stationIp, apIp) {
            rms.setOffshoreIps(stationIp, apIp);
            Object.assign(data, proxyData());
        },
        hello(data) {
            Object.assign(data, proxyData());
        }
    }, 'rms-info');

    addEventListener('DOMContentLoaded', () => {
        component('#rms-info', () => {
            console.log('Render rms info', rmsProxy.value);
            if (rmsProxy.value.offshore) {
                let apClass = '';
                let stationClass = '';
                switch (rmsProxy.value.mode) {
                    case SolarRouterRMS.MODE_AP:
                        apClass = rmsProxy.value.hello ? 'valid' : 'invalid';
                        break;
                    case SolarRouterRMS.MODE_STATION:
                        stationClass = rmsProxy.value.hello ? 'valid' : 'invalid';
                        break;
                }
                return `
    <b><a id="#rms-info-config" href="#" onClick="config()" title="Click to config">OFFSHORE MODE</a></b>
    Local IP: <b class="${stationClass}">${rmsProxy.value.stationIp}</b> -
    AP IP: <b class="${apClass}">${rmsProxy.value.apIp}</b>
    `;
            }
            if (rmsProxy.value.mode === SolarRouterRMS.MODE_DIRECT) {
                return '<b><a id="#rms-info-config" href="#" onClick="config()" title="Click to config">DIRECT MODE</a></b>';
            }
            return '<b><a id="#rms-info-config" href="#" onClick="config()" title="Click to config">UNKNOWN MODE</a></b>';
        }, {signals: ['rms-info'], events:{config: () => { multiPopup.popup('init'); }}});
    });

    return rmsProxy;
}

// Config Forms Proxy
// Proxies are essentially kitchen to handle Reef JS stuff
/**
 * @typedef {Object} ConfigFormsProxy
 */

/**
 * @returns {ConfigFormsProxy}
 */
function proxyConfigForms() {
    const proxyData = () => {
        return {
            mode: rms.mode,
            offshore: rms.offshore,
            stationIp: rms.getStationIp(),
            apIp: rms.getAPIp(),
            hello: rms.hellok,
        };
    };

    const formProxy = store(proxyData(), {
    }, 'config-forms');

    /**
     * @param {ConfigParamGroup} group 
     * @returns 
     */
    const renderTab = (name, group) => {
        const fieldset = [];
        
        for (const [name, param] of Object.entries(group.params)) {
            let input;
            switch (param.type) {
                case 'ip':
                    input = new IpFormInputHelper(name, IPAddress.toString(param.value));
                    break;
                default:
                    input = new FormInputHelper(name, param.type, param.value);
            }
            input.id = `config-param-${name}`;
            input.label = param.label;
            input.help = param.help;
            fieldset.push(input);
        }

        const html = `${fieldset.map(input => `<p>${input.html()}</p>`).join('')}`

        return `
<input name="tabs" type="radio" id="tab-${name}" class="input" />
<label for="tab-${name}" class="label">${name}</label>
<div class="panel" style="width: 100%;">
<h1>${name}</h1>
<p style="width: 100%;">
    <form id="config-form-${name}" style="width: 100%;">
    <fieldset>
    ${fieldset.map(input => `<p>${input.html()}</p>`).join('')}
    <p><button class="confirm">Save</button></p>
    </fieldset>
    </form>
</p>
</div>
`;
    };

/*

<form id="form-${name}">
<fieldset>
<p><button class="confirm">Save</button></p>
</fieldset>
</form>

*/

    const renderTabs = () => {
        let tabsHtml = '';
        for (const [name, group] of Object.entries(rms.configParamsGroups)) {
            tabsHtml += renderTab(name, group);
        }
        return tabsHtml;
    };

    addEventListener('rms.config', event => {
        render('#config-forms', renderTabs());
    });

    return formProxy;
}

function businessListeners() {
    // checking if we are in Offshore mode or not
    document.addEventListener('DOMContentLoaded', () => {
        rms.queryHello(SolarRouterRMS.MODE_DIRECT).then(data => {
            infoBoxProxy.setMode(SolarRouterRMS.MODE_DIRECT);
            console.log('Hello:', data);
            rms.start();
        }).catch(error => {
            // seams we are running in offshore mode aka from local file system or another server
            console.info('Offshore mode');
            infoBoxProxy.setOffshore();
            if (!rms.hasLocalConfig()) {
                console.log('Local storage has not been initialized.');
                multiPopup.popup('init');
            }
            rms.start();
        });
    });

    addEventListener('rms.hello', event => {
        infoBoxProxy.hello();
    });
}