"use strict";

const dicom = require("dicom")
const fs    = require("fs")
const im = require('imagemagick')
const path = require('path')

const OUTPUT_DIR_IMAGE=__dirname+"/viewer/public/images/"

//This program relies on a running dicom server: In this case we use orthanc server
const DICOM_SERVER="http://localhost:8042"
const DICOM_FILES_DIR=__dirname+"/images"

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


    //list all the png files created after processing
    listImages(id) {
	const dirName = path.join(OUTPUT_DIR_IMAGE,id);
	return  fs.readdirSync(dirName)
    }

    //TODO: not yet idempotent. Ideally should not process if this has already been done once.
    processImage(id) {
	try {
	    const img_url=`${DICOM_SERVER}/instances/${id}/file`
	    var options = {
		compressed         : true, // sets 'Accept-Encoding' to 'gzip, deflate, br'
		follow_max         : 5,    // follow up to five redirects
		rejectUnauthorized : false  // verify SSL certificate
	    }
	    
	    if (!fs.existsSync(DICOM_FILES_DIR)) {
		fs.mkdirSync(DICOM_FILES_DIR)
	    }

	    var image_fname = `${DICOM_FILES_DIR}/${id}.dcm`
	    console.log(`creating image file:${image_fname} from the ${img_url}`)

	    var wstream = fs.createWriteStream(image_fname)
            
	    needle
		.get(img_url, options)
		.pipe(wstream)
		.on('done', () => {
		    console.log('file reading from server has finished')
		    wstream.close()
		})
	    
	    //now process the file downloaded	
	    wstream.on('close', ()=> {
		fs.createReadStream(image_fname)
		    .pipe(this.decoder)
		    .pipe(this.encoder)
		    .pipe(this.sink) 
		
		//have to add a pipeline for processing image			    
		this.convert2png(image_fname, null)
	    })
	    
	    return { "filename": image_fname }
	}
	
	catch(err) {
	    console.error(err)
	}
	
    }

    
}

module.exports = new Reader()
 
if (require.main == module) {
    const reader = require("fs").createReadStream(process.argv[2]).pipe(decoder).pipe(encoder).pipe(sink);
    
}


