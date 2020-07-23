"use strict";

const needle = require('needle')
const cron = require("node-cron");

const DICOM_SERVER="http://localhost:8042"
const CRON_PERIOD=10

const reader = require('./reader')

class Monitor{
    constructor(){
	this.changes = []
    }

    process_changes(instance_arr) {
	
	console.log(`Now processing image ID array:${instance_arr}`)
	instance_arr.map(x => {
	    console.log(`Now processing image with ID:${x}`)
	    reader.processImage(x)
	})

    }


    filter_changes(){
	const new_instance = this.changes.filter(item => {
	    return (item.ChangeType == "NewInstance")
	})	
	
	const id_array =  new_instance.map(x => {
	    return x.ID
	})
	this.process_changes(id_array)
    }

    get_changes(){
	try{
	    const query_url=`${DICOM_SERVER}/changes`
	    console.log(`query url:${query_url}`)
	    needle.get(query_url, (err, resp, body) => {
		if (err) throw err;
		console.log(body)
		this.changes = body.Changes;
		this.filter_changes()
	    })
	}
	catch(err){
	    console.error(err)
	}
    }

    start(period=CRON_PERIOD){
	// Creating a cron job which runs on every 10 second 
	cron.schedule(`*/${period} * * * * *`, () => { 
	    console.log(`running a task every ${CRON_PERIOD} second`); 
	    this.get_changes()
	}); 
    }

}


if (require.main == module) {
    
    let m = new Monitor()
    m.start()

}

