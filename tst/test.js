var expect  = require('chai').expect;
var request = require('request');
var needle  = require('needle');
var BASE_URL='http://localhost:8042'

var data = {
  image: { file: '../images/0003.DCM', content_type: 'application/dicom' }
}

it('Test server is running', function(done) {
    request(`${BASE_URL}/system` , function(error, response, body) {
	const resp_system = {
	    "ApiVersion" : 7,
	    "DatabaseBackendPlugin" : null,
	    "DatabaseVersion" : 6,
	    "DicomAet" : "ORTHANC",
	    "DicomPort" : 4242,
	    "HttpPort" : 8042,
	    "IsHttpServerSecure" : true,
	    "Name" : "",
	    "PluginsEnabled" : true,
	    "StorageAreaPlugin" : null,
	    "Version" : "1.7.2"
	}
	expect(response.statusCode).to.equal(200);
	console.log(body)
        expect(JSON.parse(body).ApiVersion).to.deep.equal(resp_system.ApiVersion);
        done();
    });


    needle.post(`${BASE_URL}/instances`, data, { multipart: true }, function(err, resp, body) {
	// needle will read the file and include it in the form-data as binary
	console.log(body)
    });

});
