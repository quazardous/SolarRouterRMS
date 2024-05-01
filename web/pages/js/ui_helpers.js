// UI render helpers ans stuff

const {signal, component, render, store} = reef;

class ManagedInput {
    constructor(name, onChange, id) {
        this.name = name;
        this.id = id ? id : name;
        this.onChange = onChange;
        this.listeners = [];
    }

    addListeners() {
        if (!this.onChange) {
            return;
        }
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
        this.onChange(event.target.value, event);
    }

    getValue(value) {
        return value;
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
    constructor(id, onSubmit, managedInputOnly = false) {
        this.id = id;
        this.onSubmit = onSubmit;
        this.listeners = [];
        this.managedInputOnly = managedInputOnly;
        /**
         * @var {Array.<ManagedInput>}
         */
        this.inputs = [];
        this.listening = false;
    }

    /**
     * @param {ManagedInput} input 
     */
    addInput(input) {
        this.inputs.push(input);
    }

    addListeners() {
        if (this.listening) {
            return;
        }
        this.inputs.forEach(input => {
            input.addListeners();
        });
        this.addEventListener('submit', this.handleSubmit.bind(this));
        this.listening = true;
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
        this.inputs.forEach(input => {
            input.removeListeners();
        });
        this.listening = false;
    }

    handleSubmit(event) {
        event.preventDefault();
        const formDoc = document.getElementById(this.id);
        console.log(formDoc);
        const formData = new FormData(formDoc);
        const data = {};
        if (this.managedInputOnly) {
            this.inputs.forEach(input => {
                data[input.name] = input.getValue(formData.get(input.name));
            });
        } else {
            for (const [key, value] of formData.entries()) {
                data[key] = value;
            }
        }
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

    store(refresh) {
        if (refresh || !this._store) {
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

    static addListeners() {
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
    }
}

// render helpers

class FormInputHelper {
    /**
     * @param {String} id 
     * @param {String} type 
     * @param {String} value 
     */
    constructor(id, type, value = '') {
        this.id = id;
        this.name = id;
        this.type = type;
        this.value = value ?? '';
        this.label = '';
        this.help = '';
        this.attr = {};
        this.required = false;
    }

    htmlAttr() {
        return `
        type="${this.type}" id="${this.id}" name="${this.name}" 
        ${Object.keys(this.attr).map(key => `${key}="${this.attr[key]}"`).join(' ')}
        ${this.required?'required="required"':''}`;
    }

    /**
     * Generate the HTML for the input field
     * @returns {String} The HTML string
     */
    html() {
        let inputHtml = `
            <input ${this.htmlAttr()} value="${this.value}">
        `;
        if (this.label) {
            inputHtml = `
            <label>${this.label}${inputHtml}</label>
            `;
        }
        if (this.help) {
            inputHtml += `
            <help>${this.help}</help>
            `;
        }
        return inputHtml;
    }
}

class CheckboxFormInputHelper extends FormInputHelper {
    /**
     * @param {String} id 
     * @param {String} value 
     * @param {String} label 
     */
    constructor(id, checked = false) {
        super(id, 'checkbox', 1);
        this.checked = checked;
    }
    htmlAttr() {
        let attr = super.htmlAttr();
        if (this.checked) {
            attr += " checked";
        }
        return `${attr}`;
    }
}

class SelectFormInputHelper extends FormInputHelper {
    /**
     * @param {String} id 
     * @param {String} value 
     * @param {String} label 
     */
    constructor(id, value = '', choices = []) {
        super(id, 'select', value);
        this.choices = choices;
    }

    html() {
        let inputHtml = `
            <select ${this.htmlAttr()}>
                ${Object.keys(this.choices).map(key => `<option value="${this.choices[key]}" ${this.value == this.choices[key] ? 'selected' : ''}>${this.choices[key]}</option>`).join('')}
            </select>
        `;
        if (this.label) {
            inputHtml = `
            <label>${this.label}${inputHtml}</label>
            `;
        }
        if (this.help) {
            inputHtml += `
            <help>${this.help}</help>
            `;
        }
        return inputHtml;
    }

}

class IpFormInputHelper extends FormInputHelper {
    /**
     * @param {String} id 
     * @param {String} value 
     * @param {String} label 
     */
    constructor(id, value = '') {
        super(id, 'text', value);
        this.attr.placeholder = '0.0.0.0';
    }
    html() {
        if (this.required) {
            this.attr.pattern = '^([0-9]{1,3}\.){3}[0-9]{1,3}$';
        } else {
            this.attr.pattern = '^([0-9]{1,3}\.){3}[0-9]{1,3}$|^$';
        }
        return super.html();
    }
}
