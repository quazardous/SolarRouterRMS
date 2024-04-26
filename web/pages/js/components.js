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
                event.preventDefault();
                rmsInfoBoxProxy.resetOffshoreMode();
                this.popup.close();
            }
        }, 'Are you sure you want to reset the form?'));
    }

    data() {
        return {
            localIpClass: rms.localConfig.getStationIp() ? 'valid' : 'invalid',
            apIpClass: '',
        };
    }

    template() {
        const store = this.store();
        const localIp = new IpFormInputHelper('form-init-stationIp', rms.localConfig.getStationIp());
        localIp.required = true;
        localIp.label = 'Local IP';
        localIp.help = 'Provide the IP on your local network';
        localIp.attr.class = store.value.localIpClass;
        const apIp = new IpFormInputHelper('form-init-apIp', rms.localConfig.getAPIp());
        apIp.label = 'AP IP';
        apIp.help = 'Provide the IP when in AP mode. Empty for default.';
        apIp.attr.class = store.value.apIpClass;
        apIp.attr.placeholder = DEFAULT_AP_IP;
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

function rmsInfoBox() {
    // proxy between RMS and DOM
    let data = {
        offshoreMode: false,
        stationIp: rms.localConfig.getStationIp(),
        apIp: rms.localConfig.getAPIp(),
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
    Local IP: <b>${rms.localConfig.getStationIp()}</b> -
    AP IP: <b>${rms.localConfig.getAPIp()}</b>
    `;
            }
            return '<b><a id="#rms-info-config" href="#" onClick="config()" title="Click to config">DIRECT MODE</a></b>';
        }, {signals: ['rms-info'], events:{config: () => { multiPopup.popup('init'); }}});
    });

    return rmsInfoBoxProxy;
}
