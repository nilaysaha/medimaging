var expect  = require('chai').expect;
var request = require('request');
var needle  = require('needle');
var BASE_URL='http://localhost:8042'

var data = {
  image: { file: __dirname+'/../images/0003.DCM', content_type: 'application/dicom' }
}

var uploaded_image_ID="87ae9004-f692ccad-c4599afc-64b4d6ba-1dff969c"

it('Test server', function(done) {
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
        expect(JSON.parse(body).ApiVersion).to.deep.equal(resp_system.ApiVersion);
        done();
    });
});


it('add image ', function(done) {

    needle.post(`${BASE_URL}/instances`, data, { multipart: true }, function(err, response, body) {
	// needle will read the file and include it in the form-data as binary
	if (!err){
	    console.log('uploaded image:')
	    console.log(body)
	    expect(response.statusCode).to.deep.equal(200)
	    done()
	}
    });   

});


it('delete image', function(done) {

    console.log(`${BASE_URL}/instances/${uploaded_image_ID}`)

    needle.delete(`${BASE_URL}/instances/${uploaded_image_ID}`, null,{}, function(err, response) {
    	console.log(response.statusCode)
    	expect(response.statusCode).to.equal(200)
	done()
    });

});
