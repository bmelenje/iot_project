package edu.unicauca.iot.data.network


import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.concurrent.TimeUnit

object RetrofitInstance {
    // URL base de la API (ajusta esto según tu servidor)
    private const val BASE_URL = "http://192.168.18.170/api_iot/"

    // Interceptor para logging (opcional, pero útil para depuración)
    private val loggingInterceptor = HttpLoggingInterceptor().apply {
        level = HttpLoggingInterceptor.Level.BODY // Muestra logs detallados de las solicitudes y respuestas
    }

    // Configurar el cliente OkHttp
    private val client = OkHttpClient.Builder()
        .addInterceptor(loggingInterceptor) // Agregar el interceptor de logging
        .connectTimeout(30, TimeUnit.SECONDS) // Tiempo de espera para la conexión
        .readTimeout(30, TimeUnit.SECONDS) // Tiempo de espera para la lectura
        .writeTimeout(30, TimeUnit.SECONDS) // Tiempo de espera para la escritura
        .build()

    // Instancia de Retrofit
    val api: ApiService by lazy {
        Retrofit.Builder()
            .baseUrl(BASE_URL) // Establecer la URL base
            .addConverterFactory(GsonConverterFactory.create()) // Usar Gson para convertir JSON a objetos Kotlin
            .client(client) // Añadir el cliente OkHttp configurado
            .build()
            .create(ApiService::class.java) // Crear la instancia de ApiService
    }
}