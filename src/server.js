// Require the framework and instantiate it
const fastify = require('fastify')({ logger: true })
const PORT = 3300
const needle = require('needle')
const reader = require('./reader')
const fs = require('fs')

//This program relies on a running dicom server: In this case we use orthanc server
DICOM_SERVER="http://localhost:8042"
DICOM_FILES_DIR=__dirname+"/images"


// Declare route: test route
fastify.get('/', async (request, reply) => {
  return { hello: 'world' }
})


//Fetch the image with {id} and process it.
fastify.get('/images/:id/process', async (req, res) => {
    try {
	const img_url=`${DICOM_SERVER}/instances/${req.params.id}/file`
	var options = {
	    compressed         : true, // sets 'Accept-Encoding' to 'gzip, deflate, br'
	    follow_max         : 5,    // follow up to five redirects
	    rejectUnauthorized : false  // verify SSL certificate
	}
	
	if (!fs.existsSync(DICOM_FILES_DIR)) {
	    fs.mkdirSync(DICOM_FILES_DIR)
	}

	var image_fname = `${DICOM_FILES_DIR}/${req.params.id}.dcm`
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
		.pipe(reader.decoder)
		.pipe(reader.encoder)
		.pipe(reader.sink) 
	    
	    //have to add a pipeline for processing image			    
	    reader.convert2png(image_fname, null)
	})

	
	return { "filename": image_fname }
    }
    
    catch(err) {
	console.error(err)
    }

})


if (require.main == module) {

    // Run the server!
    const start = async () => {
	try {
	    await fastify.listen(PORT)
	    fastify.log.info(`server listening on ${fastify.server.address().port}`)
	} catch (err) {
	    fastify.log.error(err)
	    process.exit(1)
	}
    }
    start()
}
