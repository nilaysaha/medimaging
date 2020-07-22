var express = require('express');
var router = express.Router();

/* GET images listing. */
router.get('/', function(req, res, next) {
    const imagelist = [
	{"id":1, "description": "image 1", "date":"20/12/1978"},
	{"id":2, "description": "image 2", "date":"20/12/1978"},
	{"id":3, "description": "image 3", "date":"20/12/1978"}
    ]
    res.render('imageview', { image: imagelist });
});

module.exports = router;
