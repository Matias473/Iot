const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');
const Acceso = require('./models/Acceso');
const Deteccion = require('./models/Deteccion');

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.json());

// Conexión a MongoDB
mongoose.connect('mongodb://127.0.0.1:27017/rfid')
  .then(() => console.log('Conectado a MongoDB'))
  .catch(err => console.error('Error conectando a MongoDB:', err));


// 🟢 Ruta POST para guardar accesos (tarjeta RFID)
app.post('/api/acceso', async (req, res) => {
  try {
    const nuevoAcceso = new Acceso(req.body);
    await nuevoAcceso.save();
    console.log('Acceso recibido:', nuevoAcceso);
    res.status(201).json({ message: 'Acceso guardado en MongoDB' });
  } catch (error) {
    console.error('Error al guardar acceso:', error);
    res.status(500).json({ error: 'Error del servidor' });
  }
});

// 🔵 Ruta GET para consultar accesos
app.get('/api/accesos', async (req, res) => {
  try {
    const accesos = await Acceso.find().sort({ fecha: -1 });
    res.json(accesos);
  } catch (error) {
    res.status(500).json({ error: 'Error al obtener accesos' });
  }
});


// 🟠 Ruta POST para guardar detecciones (sensor de proximidad)
app.post('/api/movimiento', async (req, res) => {
  try {
    const { evento, distancia, hora } = req.body;
    const nuevaDeteccion = new Deteccion({ evento, distancia, hora });
    await nuevaDeteccion.save();
    console.log("etección registrada:", nuevaDeteccion);
    res.status(201).json({ message: 'Detección guardada en MongoDB' });
  } catch (error) {
    console.error("Error al guardar detección:", error);
    res.status(500).json({ error: 'Error del servidor' });
  }
});

// 🔴 Ruta GET para consultar todas las detecciones
app.get('/api/detecciones', async (req, res) => {
  try {
    const detecciones = await Deteccion.find().sort({ fecha: -1 });
    res.json(detecciones);
  } catch (error) {
    res.status(500).json({ error: 'Error al obtener detecciones' });
  }
});

// 🚀 Iniciar servidor
app.listen(PORT, () => {
  console.log(`Servidor escuchando en http://localhost:${PORT}`);
});
