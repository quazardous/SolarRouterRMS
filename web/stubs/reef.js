// stubs for the Reef library

/**
 * Create a new signal
 * @param  {Object} data The data object
 * @param  {String} name The custom event namespace
 * @return {Proxy}       The signal Proxy
 */
function signal (data = {}, name = '') {
}

/**
 * Render a template into the UI
 * @param  {Node|String} elem     The element or selector to render the template into
 * @param  {String}      template The template to render
 * @param  {Object}      events   The allowed event functions
 */
function render (elem, template, events) {
}

/**
 * Create a new listener
 * @param  {Node|String} elem     The element or selector to render the template into
 * @param  {Function}    template The template function to run when the data updates
 * @param  {Object}      options  Additional options
 */
function component (elem, template, options = {}) {
}

/**
 * Create a new store
 * @param  {Object} data    The data object
 * @param  {Object} setters The store functions
 * @param  {String} name    The custom event namespace for the signal
 * @return {Proxy}          The Store instance
 */
function store (data = {}, setters = {}, name = '') {
}
