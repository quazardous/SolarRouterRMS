/**
 * @var {Reef} reef
 * @source ../lib/reef.js
 */

/**
 * @var {SolarRouterRMS} rms
 */

let {signal, component, render, store} = reef;

class ManagedInput {
    constructor(id, onChange) {
        this.id = id;
        this.onChange = onChange;
    }

    addListener() {
        const element = document.getElementById(this.id);
        console.log('Id:', this.id, 'Element:', element);
        // const inputType = element.getAttribute('type').toLowerCase();
        const tagName = element.tagName.toLowerCase();
        if (tagName === 'input') {
            element.addEventListener('input', this.handleChange.bind(this));
        } else {
            element.addEventListener('change', this.handleChange.bind(this));
        }
    }

    removeListener() {
        const element = document.getElementById(this.id);
        element.removeEventListener('change', this.handleChange.bind(this));
    }

    handleChange(event) {
        this.onChange(event.target.value, event.target);
    }

    getValue() {
        return document.getElementById(this.id).value;
    }
}

class ManagedForm {
    constructor(id, onSubmit) {
        this.id = id;
        this.onSubmit = onSubmit;
        /**
         * @var {Array.<MonitoredInput>}
         */
        this.inputs = [];
    }

    /**
     * @param {ManagedInput} input 
     */
    addInput(input) {
        this.inputs.push(input);
    }
    addListener() {
        this.inputs.forEach(input => {
            input.addListener();
        });
        const element = document.getElementById(this.id);
        element.addEventListener('submit', this.handleSubmit.bind(this));
    }

    removeListener() {
        this.inputs.forEach(input => {
            input.removeListener();
        });
        const element = document.getElementById(this.id);
        element.removeEventListener('submit', this.handleSubmit.bind(this));
    }

    handleSubmit(event) {
        event.preventDefault();
        const data = {};
        this.inputs.forEach(input => {
            data[input.id] = input.getValue();
        });
        this.onSubmit(data);
    }
}

class PopupTpl {
    constructor() {
        this.popup;
        this.id; // name of the template
        this._store;
    }
    data() {
        return {};
    }

    template() {
        throw new Error('Not implemented');
    }

    canClose() {
        // display the close button
        return true;
    }

    /**
     * @returns {Array.<ManagedInput>}
     */
    monitor() {
        return [];
    }

    onClose() {

    }

    store() {
        if (!this._store) {
            this._store = store(this.data(), {
                set(data, k, v) {
                    data[k] = v;
                }
            }, this.id);
        }
        return this._store;
    }
}

class PopupInitTpl extends PopupTpl {
    constructor() {
        super();
        this.form = new ManagedForm('form-init', (data) => {
            rmsProxy.setOfflineModeIps(data['form-init-stationIp'], data['form-init-apIp']);
            this.popup.close();
        });
        this.form.addInput(new ManagedInput('form-init-stationIp', (value, input) => {
            const data = this.store();
            data.set('localIpClass', IPAddress.validateIP(value) ? 'valid' : 'invalid');
        }));
        this.form.addInput(new ManagedInput('form-init-apIp', (value, input) => {
            const data = this.store();
            data.set('apIpClass', value.trim() === '' || IPAddress.validateIP(value) ? 'valid' : 'invalid');
        }));
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
<p><button>Save</button></p>
</fieldset>
</form>
`;
    }

    canClose() {
        return false;
    }

    monitor() {
        this.form.addListener();
    }

    onClose() {
        this.form.removeListener();
    }
}

class MultiPopup {
    /**
     * @var {Object.<MultiPopup>} popups
     */
    static popups = {};

    /**
     * @returns {MultiPopup}
     */
    static factory(id) {
        const popup = new MultiPopup(id);
        MultiPopup.popups[id] = popup;
        return popup;
    }
    /**
     * Represents a Render object.
     * @constructor
     * @param {string} id - The ID of the Render object.
     */
    constructor(id) {
        this.id = id;
        /**
         * @var {Object.<string, PopupTpl>} templates - The templates for the Render object.
         */
        this.templates = {};

        /**
         * @var {PopupTpl|null} currentTpl - The current popup template.
         */
        this.currentTpl = null;

        this.pendingPostRender = [];
    }

    /**
     * @param {PopupTpl} tpl 
     */
    registerTemplate(tplId, tpl) {
        tpl.id = tplId;
        tpl.popup = this;
        this.templates[tplId] = tpl;
    }

    popup(tplId) {
        console.log('Popup:', tplId);
        const self = this;
        this.closable = true;
        const renderTpl = () => {
            let content = self.templates[tplId].template();
            if (self.templates[tplId].canClose()) {
                content += `
                <form method="dialog">
                <button>Close</button>
                </form>`;
            }
            return content;
        };
        /**
         * @var {PopupTpl} tpl
         */
        const tpl = this.templates[tplId];
        
        component('#' + this.id, renderTpl, {signals: [tpl.id]});
        const popup = document.querySelector('#' + this.id);

        this.pendingPostRender.push(() => {
            tpl.monitor();
            popup.showModal();
            popup.addEventListener('close', () => {
                self.currentTpl = null;
                tpl.onClose();
            });
        });
        this.currentTpl = tpl;
    }

    close() {
        const popup = document.querySelector('#' + this.id);
        popup.close();
    }

    postRender() {
        while (this.pendingPostRender.length > 0) {
            const fn = this.pendingPostRender.shift();
            fn();
        }
    }

    preventClose() {
        return this.currentTpl && !this.currentTpl.canClose();
    }
}

const multiPopup = MultiPopup.factory('multi-popup');
multiPopup.registerTemplate('init', new PopupInitTpl());

// prevents modal to close on ESC if not allowed
addEventListener('keydown', e => {
    if (e.key === "Escape") {
        for (const popup of Object.values(MultiPopup.popups)) {
            if (popup.preventClose()) {
                e.preventDefault();
                break;
            }
        }
    }
});

// execute deferred functions after rendering
addEventListener('reef:render', function (event) {
    // console.log('Rendered:', event.target);
    for (const popup of Object.values(MultiPopup.popups)) {
        if (event.target.id === popup.id) {
            popup.postRender();
        }
    }
});

// proxy between RMS and DOM
let data = {
    offlineMode: false,
    stationIp: rms.localConfig.getStationIp(),
    apIp: rms.localConfig.getAPIp(),
};
console.log('Data:', data);
const rmsProxy = store({}, {
    set(data, key, value) {
        data[key] = value;
        rms.setConfig(key, value);
    },
    setOfflineMode() {
        data['offlineMode'] = true;
        rms.setOfflineMode();
    },
    setOfflineModeIps(data, stationIp, apIp) {
        data['offlineMode'] = true;
        data['stationIp'] = stationIp;
        data['apIp'] = apIp;
        rms.setOfflineModeIps(stationIp, apIp);
    }
}, 'rms-info');

addEventListener('DOMContentLoaded', () => {
    component('#rms-info', () => {
        if (rms.offlineMode) {
            return `
<b><a id="#rms-info-config" href="#" onClick="config" title="Click to config">OFFLINE MODE</a></b>
Local IP: <b>${rms.localConfig.getStationIp()}</b> -
AP IP: <b>${rms.localConfig.getAPIp()}</b>
`;
        }
        return '<b>DIRECT MODE</b>';
    }, {signals: ['rms-info'], events:{config: () => { multiPopup.popup('init'); }}});
});
