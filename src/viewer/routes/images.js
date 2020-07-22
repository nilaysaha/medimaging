const express = require('express');
const router = express.Router();
const needle = require('needle');
const reader = require('../../reader');

const DICOM_SERVER="http://localhost:8042"
const DICOM_FILES_DIR=__dirname+"/images"


const formThumbnails = function(id) {
    const imgList = reader.listImages(id)
    const imgList_str = imgList.map(img_png => {
	return `<img src="/images/${id}/${img_png}" class = "img-thumbnail" alt="Medical image of ${id}"/>`
    })
    return imgList
}

const imageList = function(id) {
    const imgList = reader.listImages(id)
    return  imgList.map(x => {
	return {"id":id, "name":x}
    })
}


/* GET images listing. */
router.get('/', function(req, res, next) {    
    try {
	const req_url = `${DICOM_SERVER}/instances`
	needle.get(req_url, function(error, response, body) {
	    if (error) throw error;
	    const img_array = body.map(id => {
		console.log(`now forming output for image: ${id}`)
		return {id: id, "description": "medical image", "date": new Date()}
	    })
	    res.render('imageview', { image: img_array });
	});
    }
    catch(err) {
	console.error(err)
	throw new Error(err)
    }   
});

router.get('/details/:id', function(req, res, next) {    
    try {
	res.render('imageDetails', { "id":req.params.id, "images": imageList(req.params.id) });
    }
    catch(err) {
	console.error(err)
	throw new Error(err)
    }   
});

router.get('/process/:id', function(req, res, next) {    
    try {
	//now we call the processing of the image. It will convert the image to png and store them public folder of webapp
	reader.processImage(req.params.id)
    }
    catch(err) {
	console.error(err)
	throw new Error(err)
    }   
});

module.exports = router;
