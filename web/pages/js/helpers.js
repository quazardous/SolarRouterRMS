class IPAddress {
    /**
     * @param {String} ip 
     */
    constructor(ip) {
        this.ip = ip.trim();
    }

    isValid() {
        IPAddress.validateIP(this.ip);
    }

    toUL() {
        if (!this.isValid()) {
            return 0;
        }

        const ul = this.ip.split('.').reduce((acc, curr, index) => {
            return acc + parseInt(curr) << (24 - (8 * index));
        }, 0);

        return ul;
    }

    static validateIP(ipString) {
        const ipRegex = /^([0-9]{1,3}\.){3}[0-9]{1,3}$/;
        return ipRegex.test(ipString);
    }
}

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

    /**
     * Generate the HTML for the input field
     * @returns {String} The HTML string
     */
    html() {
        let inputHtml = `
            <input type="${this.type}" id="${this.id}" name="${this.name}" value="${this.value}" 
                ${Object.keys(this.attr).map(key => `${key}="${this.attr[key]}"`).join(' ')}
                ${this.required?'required="required"':''}>
        `;
        if (this.label) {
            inputHtml = `
            <label for="${this.name}">${this.label}</label>
            ${inputHtml}
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