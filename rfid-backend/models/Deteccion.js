const mongoose = require('mongoose');

const deteccionSchema = new mongoose.Schema({
  evento: String,
  distancia: String,
  hora: String,
  fecha: {
    type: Date,
    default: Date.now
  }
});

module.exports = mongoose.model('Deteccion', deteccionSchema);
