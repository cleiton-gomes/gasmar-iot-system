const express = require('express');
const router = express.Router();
const sensorController = require('../controllers/sensorController');

router.post('/data', sensorController.receberDados);
router.get('/status', sensorController.getUltimoStatus);
router.get('/historico', sensorController.getHistorico);
module.exports = router;