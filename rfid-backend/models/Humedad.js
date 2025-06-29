const mongoose = require('mongoose');

const humedadSchema = new mongoose.Schema({
  valor: Number,
  unidad: { type: String, default: "%" },
  hora: String,
  ubicacion: String,
  fecha: { type: Date, default: Date.now }
});

module.exports = mongoose.model('Humedad', humedadSchema);