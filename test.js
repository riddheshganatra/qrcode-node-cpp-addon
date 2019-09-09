var qr = require('qrcodeine');

// or:
var qrPngBuffer = qr.encodePng('Some text to put in a QR Code PNG');
console.log(qrPngBuffer.data.toString('base64'));