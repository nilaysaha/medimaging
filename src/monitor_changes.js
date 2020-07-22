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
	
	instance_arr.map(x => {
	    reader.processImage(x)
	})

    }


    filter_changes(){
	const new_instances = this.changes.map(item => {
	    if (item.ChangeType == "NewInstance"){
		return item
	    }
	})	
	
	this.process_changes(new_instances)
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

