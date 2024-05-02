// miscellaneous helper classes

class IPAddress {
    /**
     * @param {String} ip 
     */
    constructor(ip) {
        this.ip = ip.trim();
    }

    isValid() {
        return IPAddress.validateIP(this.ip);
    }

    toUL() {
        if (!this.isValid()) {
            return 0;
        }

        return (this.ip.split(".").reduce((ipInt, octet) => (ipInt << 8) + parseInt(octet, 10), 0) >>> 0);
    }

    static validateIP(ipString) {
        const ipRegex = /^([0-9]{1,3}\.){3}[0-9]{1,3}$/;
        return ipRegex.test(ipString);
    }

    static toString(ul) {
        return (
            (ul >>> 24) +
            "." +
            ((ul >> 16) & 255) +
            "." +
            ((ul >> 8) & 255) +
            "." +
            (ul & 255)
        );
    }
}

class LocalConfig {
    saveData(key, data) {
        if (LocalConfig.empty(data)) {
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
    static empty(val) {

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
