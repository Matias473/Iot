const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');
const Acceso = require('./models/Acceso');
const Deteccion = require('./models/Deteccion');
const Temperatura = require('./models/Temperatura');
const Humedad = require('./models/Humedad');

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.json());

// Conexión a MongoDB
mongoose.connect('mongodb://127.0.0.1:27017/rfid')
  .then(() => console.log('Conectado a MongoDB'))
  .catch(err => console.error('Error conectando a MongoDB:', err));


// Ruta POST para guardar accesos (tarjeta RFID)
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

// Ruta GET para consultar accesos
app.get('/api/accesos', async (req, res) => {
  try {
    const accesos = await Acceso.find().sort({ fecha: -1 });
    res.json(accesos);
  } catch (error) {
    res.status(500).json({ error: 'Error al obtener accesos' });
  }
});


// Ruta POST para guardar detecciones (sensor de proximidad)
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

// Ruta GET para consultar todas las detecciones
app.get('/api/detecciones', async (req, res) => {
  try {
    const detecciones = await Deteccion.find().sort({ fecha: -1 });
    res.json(detecciones);
  } catch (error) {
    res.status(500).json({ error: 'Error al obtener detecciones' });
  }
});

// Ruta POST para guardar la Temperatura
app.post('/api/temperatura', async (req, res) => {
  try {
    const nueva = new Temperatura(req.body);
    await nueva.save();
    console.log("Temperatura registrada:", nueva);
    res.status(201).json({ message: 'Temperatura guardada' });
  } catch (error) {
    res.status(500).json({ error: 'Error al guardar temperatura' });
  }
});

// Ruta POST para guardar la Humedad
app.post('/api/humedad', async (req, res) => {
  try {
    const nueva = new Humedad(req.body);
    await nueva.save();
    console.log("Humedad registrada:", nueva);
    res.status(201).json({ message: 'Humedad guardada' });
  } catch (error) {
    res.status(500).json({ error: 'Error al guardar humedad' });
  }
});


//Ruta GET para consultar todos los datos de Temperatura
app.get('/api/temperaturas', async (req, res) => {
  try {
    const temperaturas = await Temperatura.find().sort({ fecha: -1 });
    res.json(temperaturas);
  } catch (error) {
    console.error('Error al obtener temperaturas:', error);
    res.status(500).json({ error: 'Error al obtener temperaturas' });
  }
});

//Ruta GET para consultar todos los datos de humedad
app.get('/api/humedades', async (req, res) => {
  try {
    const humedades = await Humedad.find().sort({ fecha: -1 });
    res.json(humedades);
  } catch (error) {
    console.error('Error al obtener humedades:', error);
    res.status(500).json({ error: 'Error al obtener humedades' });
  }
});

// Iniciar servidor
app.listen(PORT, () => {
  console.log(`Servidor escuchando en http://localhost:${PORT}`);
});
