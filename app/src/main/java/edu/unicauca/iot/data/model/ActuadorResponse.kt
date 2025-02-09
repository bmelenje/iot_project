package edu.unicauca.iot.data.model

data class ActuadorResponse(
    val estado: String?, // Estado del actuador ("ON" u "OFF")
    val error: String?   // Mensaje de error (opcional)
)