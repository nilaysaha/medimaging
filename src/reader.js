"use strict";

var dicom = require("dicom");
var fs    = require("fs");

class Reader{
    constructor(){
	this.encoder = new dicom.json.JsonEncoder();
	this.decoder = 	dicom.decoder({
	    guess_header: true
	});
	this.sink =  new dicom.json.JsonSink((err, json) => {
	    var self = this
	    if (err) {
		console.log("Error:", err);
		process.exit(10);
	    }
	    this.print_element(json, dicom.tags.PatientID);
	    this.print_element(json, dicom.tags.IssuerOfPatientID);
	    this.print_element(json, dicom.tags.StudyInstanceUID);
	    this.print_element(json, dicom.tags.AccessionNumber);
	});
    }

    print_element(json, elem) {
	const val = dicom.json.get_value(json, elem)
	console.log(`key: ${elem.name}, value: ${val}`);
    };

    exec(fname) {
	try {
	    if (fs.existsSync(fname)){
		return fs.createReadStream(fname).pipe(this.decoder).pipe(this.encoder).pipe(this.sink);
	    }else {
		throw new Error(`File:${fname} does not exist`); 
	    }
	}
	catch (err){
	    console.error(err.toString())
	}
	
    }

    watermark(fname) {
	return;
    }
    
}

module.exports = new Reader()
 
if (require.main == module) {
    const reader = require("fs").createReadStream(process.argv[2]).pipe(decoder).pipe(encoder).pipe(sink);
    
}


