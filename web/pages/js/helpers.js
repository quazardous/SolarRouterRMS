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
