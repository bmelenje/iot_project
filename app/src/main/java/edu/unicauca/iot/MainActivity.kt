package edu.unicauca.iot

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import edu.unicauca.iot.data.network.RetrofitInstance
import edu.unicauca.iot.model.Sensor
import edu.unicauca.iot.ui.theme.IOTTheme
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            IOTTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    SensorUI(modifier = Modifier.padding(innerPadding))
                }
            }
        }
    }
}

@Composable
fun SensorUI(modifier: Modifier = Modifier) {
    var temperatura by remember { mutableStateOf("Cargando...") }
    var humedad by remember { mutableStateOf("Cargando...") }

    // Cargar datos desde la API
    LaunchedEffect(Unit) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                // Solicitar la temperatura
                val tempResponse = RetrofitInstance.api.getSensorValue("temperatura")
                if (tempResponse.isSuccessful) {
                    val sensorTemp = tempResponse.body()
                    if (sensorTemp != null) {
                        temperatura = "${sensorTemp.valor.toInt()}°C"
                    }
                }

                // Solicitar la humedad
                val humResponse = RetrofitInstance.api.getSensorValue("humedad")
                if (humResponse.isSuccessful) {
                    val sensorHum = humResponse.body()
                    if (sensorHum != null) {
                        humedad = "${sensorHum.valor.toInt()}%"
                    }
                }
            } catch (e: Exception) {
                temperatura = "Error al cargar"
                humedad = "Error al cargar"
            }
        }
    }

    Column(
        modifier = modifier.fillMaxSize().padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(text = "Temperatura", fontSize = 20.sp, color = androidx.compose.ui.graphics.Color.Red)
        Text(text = temperatura, fontSize = 32.sp, color = androidx.compose.ui.graphics.Color.Red)
        Spacer(modifier = Modifier.height(16.dp))
        Text(text = "Humedad Ambiental", fontSize = 20.sp, color = androidx.compose.ui.graphics.Color.Blue)
        Text(text = humedad, fontSize = 32.sp, color = androidx.compose.ui.graphics.Color.Blue)
        Spacer(modifier = Modifier.height(32.dp))
        ButtonONOFF(title = "Modo")
        Spacer(modifier = Modifier.height(16.dp))
        ButtonONOFF(title = "Motorbomba")
    }
}


@Composable
fun ButtonONOFF(title: String, modifier: Modifier = Modifier) {
    val buttonState = remember { mutableStateOf("OFF") }

    LaunchedEffect(Unit) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                val response = RetrofitInstance.api.getEstadoBomba()
                if (response.isSuccessful) {
                    val estado = response.body()?.estado
                    if (estado != null) {
                        buttonState.value = estado
                    }
                }
            } catch (e: Exception) {
                println("Error de red: ${e.message}")
            }
        }
    }

    fun updateActuadorState(newState: String) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                val response = RetrofitInstance.api.updateActuador(newState)
                if (response.isSuccessful) {
                    println("Estado actualizado correctamente: $newState")
                }
            } catch (e: Exception) {
                println("Error de red: ${e.message}")
            }
        }
    }

    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(text = title, fontSize = 18.sp)
        Button(
            onClick = {
                val newState = if (buttonState.value == "OFF") "ON" else "OFF"
                buttonState.value = newState
                updateActuadorState(newState)
            },
            modifier = modifier
        ) {
            Text(text = buttonState.value)
        }
    }
}
