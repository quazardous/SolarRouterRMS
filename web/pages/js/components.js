// business class and component factories

// popup for RMS initialization (Mode Offshore / Mode Direct)
class PopupInitTpl extends PopupTpl {
    constructor() {
        super();
        this.form = new ManagedForm('form-init', (data) => {
            rmsInfoBoxProxy.setOffshoreModeIps(data['form-init-stationIp'], data['form-init-apIp']);
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
                rmsInfoBoxProxy.resetOffshoreMode();
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
        apIp.attr.placeholder = DEFAULT_AP_IP;
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
    }
}

// factories

// Info Box Proxy
// Proxies are essentially kitchen to handle Reef JS stuff
/**
 * @typedef {Object} InfoBoxProxy
 * @property {function(string, any): void} set - Sets a value in the proxy and updates the corresponding RMS configuration.
 * @property {function(): void} setOffshoreMode - Sets the offshore mode and updates the corresponding RMS configuration.
 * @property {function(): void} resetOffshoreMode - Resets the offshore mode and related IPs.
 * @property {function(string, string): void} setOffshoreModeIps - Sets the offshore mode and the station IP and AP IP.
 */

/**
 * @returns {InfoBoxProxy}
 */
function rmsInfoBox() {
    let data = {
        offshoreMode: false,
        stationIp: rms.getStationIp(),
        apIp: rms.getAPIp(),
    };
    console.log('Data:', data);
    const rmsInfoBoxProxy = store({}, {
        set(data, key, value) {
            data[key] = value;
            rms.setConfig(key, value);
        },
        setOffshoreMode() {
            data['offshoreMode'] = true;
            rms.setOffshoreMode();
        },
        resetOffshoreMode() {
            data['offshoreMode'] = true;
            data['stationIp'] = null;
            data['apIp'] = null;
            rms.setOffshoreMode(true);
            rms.setOffshoreModeIps(null, null);
        },
        setOffshoreModeIps(data, stationIp, apIp) {
            data['offshoreMode'] = true;
            data['stationIp'] = stationIp;
            data['apIp'] = apIp;
            rms.setOffshoreModeIps(stationIp, apIp);
        }
    }, 'rms-info');

    addEventListener('DOMContentLoaded', () => {
        component('#rms-info', () => {
            if (rms.offshoreMode) {
                return `
    <b><a id="#rms-info-config" href="#" onClick="config()" title="Click to config">OFFSHORE MODE</a></b>
    Local IP: <b>${rms.getStationIp()}</b> -
    AP IP: <b>${rms.getAPIp()}</b>
    `;
            }
            return '<b><a id="#rms-info-config" href="#" onClick="config()" title="Click to config">DIRECT MODE</a></b>';
        }, {signals: ['rms-info'], events:{config: () => { multiPopup.popup('init'); }}});
    });

    return rmsInfoBoxProxy;
}

function businessListeners() {
    // checking if we are in Offshore mode or not
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
}