const mongoose = require('mongoose');

const temperaturaSchema = new mongoose.Schema({
  valor: Number,
  unidad: { type: String, default: "°C" },
  hora: String,
  ubicacion: String,
  fecha: { type: Date, default: Date.now }
});

module.exports = mongoose.model('Temperatura', temperaturaSchema);