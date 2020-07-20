// Require the framework and instantiate it
const fastify = require('fastify')({ logger: true })
const PORT = 3300
const needle = require('needle')

//This program relies on a running dicom server: In this case we use orthanc server
DICOM_SERVER="http://localhost:8042"
DICOM_FILES=''

// Declare route: test route
fastify.get('/', async (request, reply) => {
  return { hello: 'world' }
})


// Declare route: submit image
fastify.get('/images', async (req, res) => {
    const img_list_url=`${DICOM_SERVER}/instances`
    return needle.get(img_list_url)
	.then( resp => {
	    return resp
	})
	.catch(err => {
	    console.error(err)
	})
})

fastify.get('/images/:id', async (req, res) => {
    const img_detail_url=`${DICOM_SERVER}/instances/${id}`
    return needle.get(img_detail_url)
	.then( resp => {
	    return resp
	})
	.catch(err => {
	    console.error(err)
	})
})

//Fetch the image with {id} and process it.
fastify.post('/images/:id/process', async (req, res) => {
    const img_url=`${DICOM_SERVER}/instances/${id}/file`
    

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
