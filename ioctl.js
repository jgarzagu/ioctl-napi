const os = require('os');

// Export ioctl
module.exports = require('bindings')('ioctl').ioctl;

// Export memAddress functions
const bufferMemAddress = require('bindings')('ioctl').memAddress;
Buffer.prototype.memAddress = function memAddress () {
  return bufferMemAddress(this);
};
Buffer.prototype.memAddressLE = function memAddressLE () {
	if (os.endianness() == 'LE') {
		return swapMemAddress(bufferMemAddress(this));
	} else {
		return bufferMemAddress(this);
	}
};
Buffer.prototype.memAddressBE = function memAddressLE () {
	if (os.endianness() == 'BE') {
		return bufferMemAddress(this);	
	} else {
		return swapMemAddress(bufferMemAddress(this));
	}
};
function swapMemAddress (a) {
	if(a.length == 16) { // 64-bit architecture 
		return (a[14]+a[15]+a[12]+a[13]+a[10]+a[11]+a[8]+a[9]+
			a[6]+a[7]+a[4]+a[5]+a[2]+a[3]+a[0]+a[1]);
	} else if (a.length == 8) { // 32-bit architecture 
		return (a[6]+a[7]+a[4]+a[5]+a[2]+a[3]+a[0]+a[1]);
	} else {
		throw "Unexpected memory Address size";
	}
}
