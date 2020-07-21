"use strict";

const dicom = require("dicom")
const fs    = require("fs")
const im = require('imagemagick')
const path = require('path')

const OUTPUT_DIR_IMAGE=__dirname+"/../processed_image"

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

    ensureDir(dname) {
	try {
	    console.log(`trying to create dir:${dname}`)
	    if (!fs.existsSync(dname)){
		fs.mkdirSync(dname,  {recursive: true})
	    }	    
	}

	catch(err) {
	    console.error(err)
	}
    }

    //convert the dcm image fname to png.
    //if we want to add watermark to image, add a text. Or send null.
    convert2png(fname,watermark_text) {
	try {
	    const t = path.basename(fname)
	    const new_dir = path.join(OUTPUT_DIR_IMAGE, t.split(".")[0])
	    const t_saved = path.join(new_dir, `sconvert.png`)

	    this.ensureDir(new_dir)

	    if (watermark_text == null ) {
		im.convert([fname, `label:'${watermark_text}'`,t_saved], 
			   (err, stdout) => {
			       if (err) throw err;
			       console.log('stdout:', stdout);
			   });
	    } else {
		im.convert([fname ,t_saved], 
			   (err, stdout) => {
			       if (err) throw err;
			       console.log('stdout:', stdout);
			   });
	    }

	}

	catch (err) {
	    console.error(err)
	}

	
    }
    
}

module.exports = new Reader()
 
if (require.main == module) {
    const reader = require("fs").createReadStream(process.argv[2]).pipe(decoder).pipe(encoder).pipe(sink);
    
}


