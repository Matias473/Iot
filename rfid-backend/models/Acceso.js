const mongoose = require('mongoose');

const accesoSchema = new mongoose.Schema({
  uid: String,
  usuario: String,
  hora: String,
  fecha: {
    type: Date,
    default: Date.now
  }
});

module.exports = mongoose.model('Acceso', accesoSchema);
