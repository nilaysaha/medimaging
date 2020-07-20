// Require the framework and instantiate it
const fastify = require('fastify')({ logger: true })
const PORT = 3300
const needle = require('needle')
const reader = require('./reader')

//This program relies on a running dicom server: In this case we use orthanc server
DICOM_SERVER="http://localhost:8042"
DICOM_FILES_DIR="./images"


// Declare route: test route
fastify.get('/', async (request, reply) => {
  return { hello: 'world' }
})


//Fetch the image with {id} and process it.
fastify.post('/images/:id/process', async (req, res) => {
    try {
	const img_url=`${DICOM_SERVER}/instances/${id}/file`
	var options = {
	    compressed         : true, // sets 'Accept-Encoding' to 'gzip, deflate, br'
	    follow_max         : 5,    // follow up to five redirects
	    rejectUnauthorized : true  // verify SSL certificate
	}
        
	var stream = needle.get(img_url, options);
	var wstream = require('fs').createWriteStream(`${DICOM_FILES_DIR}/${id}.dcm`)
	stream.pipe(wstream)
	stream.pipe(reader.decoder).pipe(reader.encoder).pipe(reader.sink) //have to add a pipeline for processing image
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
