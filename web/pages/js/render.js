let {signal, component, render, store} = reef;

class ManagedInput {
    constructor(id, onChange) {
        this.id = id;
        this.onChange = onChange;
        this.listeners = [];
    }

    addListeners() {
        const element = document.getElementById(this.id);
        // const inputType = element.getAttribute('type').toLowerCase();
        const tagName = element.tagName.toLowerCase();
        const onChange = this.handleChange.bind(this);
        if (tagName === 'input') {
            this.addEventListener('input', onChange);
        } else {
            this.addEventListener('change', onChange);
        }
    }

    addEventListener(event, callback) {
        const element = document.getElementById(this.id);
        this.listeners.push({element, event, callback});
        element.addEventListener(event, callback);
    }

    removeListeners() {
        while (this.listeners.length > 0) {
            const listener = this.listeners.shift();
            listener.element.removeEventListener(listener.event, listener.callback);
        }
    }

    handleChange(event) {
        console.log('Change:', event.target.value);
        this.onChange(event.target.value, event);
    }

    getValue() {
        return document.getElementById(this.id).value;
    }
}

class ManagedButton extends ManagedInput {
    constructor(id, onClick, confirm = '') {
        super(id, null);
        this.onClick = onClick;
        this.confirm = confirm;
        this.bypassValidation = false;
    }

    addListeners() {
        this.addEventListener('click', this.handleClick.bind(this));
    }

    handleClick(event) {
        let confirmed = true;
        if (this.confirm && this.confirm !== '') {
            confirmed = confirm(this.confirm);
        }
        this.onClick(confirmed, event);
    }
}

class ManagedForm {
    constructor(id, onSubmit) {
        this.id = id;
        this.onSubmit = onSubmit;
        this.listeners = [];
        this.eventController = new AbortController();
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

    addListeners() {
        this.inputs.forEach(input => {
            input.addListeners();
        });
        this.addEventListener('submit', this.handleSubmit.bind(this));
    }

    addEventListener(event, callback) {
        const element = document.getElementById(this.id);
        this.listeners.push({element, event, callback});
        element.addEventListener(event, callback, {signal: this.eventController.signal});
    }

    removeListeners() {
        while (this.listeners.length > 0) {
            const listener = this.listeners.shift();
            listener.element.removeEventListener(listener.event, listener.callback);
        }
        this.inputs.forEach(input => {
            input.removeListeners();
        });
    }

    handleSubmit(event) {
        event.preventDefault();
        const data = {};
        this.inputs.forEach(input => {
            data[input.id] = input.getValue();
        });
        this.onSubmit(data, event);
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

    onShow() {
        
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

        let preventCloseCallback = null;
        if (!tpl.canClose()) {
            preventCloseCallback = e => {
                if (e.key === "Escape") {
                    e.preventDefault();
                }
            };
            // prevents modal to close on ESC if not allowed
            addEventListener('keydown', preventCloseCallback);
        }
        
        component('#' + this.id, renderTpl, {signals: [tpl.id]});
        const popup = document.querySelector('#' + this.id);
        this.pendingPostRender.push(() => {
            tpl.onShow();
            popup.showModal();
            popup.addEventListener('close', () => {
                console.log('Closing:', tpl.id);
                if (preventCloseCallback) {
                    removeEventListener('keydown', preventCloseCallback);
                }
                self.currentTpl = null;
                tpl.onClose();
            }, {once: true});
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
}

const multiPopup = MultiPopup.factory('multi-popup');

// execute deferred functions after rendering
addEventListener('reef:render', function (event) {
    // console.log('Rendered:', event.target);
    for (const popup of Object.values(MultiPopup.popups)) {
        if (event.target.id === popup.id) {
            popup.postRender();
        }
    }
});

addEventListener('click', (event) => {
    if (['button', 'a'].includes(event.target.tagName.toLowerCase())) {
        const message = event.target.getAttribute('confirm');
        if(message !== null) {
            if (!confirm(message ? message : 'Are you sure?')) {
                event.preventDefault();
            }
        }
    }
});