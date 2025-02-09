package edu.unicauca.iot.data.network

import edu.unicauca.iot.data.model.Actuador
import edu.unicauca.iot.data.model.ActuadorResponse
import edu.unicauca.iot.model.Sensor
import okhttp3.ResponseBody
import retrofit2.Response
import retrofit2.http.*

interface ApiService {

    /* Actualizar el estado de un actuador
    @FormUrlEncoded
    @POST("update_actuador.php")
    suspend fun updateActuador(
        @Field("id") id: Int,
        @Field("estado") estado: String
    ): Response<ResponseBody>
     */

    @GET("get_actuador.php")
    suspend fun getEstadoBomba(): Response<ActuadorResponse> // Obtener el estado del actuador "bomba"

    @FormUrlEncoded
    @POST("update_actuador.php")
    suspend fun updateActuador(
        @Field("estado") estado: String
    ): Response<ResponseBody>

    // Obtener el estado de un actuador por ID
    @GET("get_actuador.php")
    suspend fun getActuador(@Query("id") id: Int): Actuador

    // Actualizar el valor de un sensor
    @FormUrlEncoded
    @POST("update_sensor.php")
    suspend fun updateSensor(
        @Field("id") id: Int,
        @Field("valor") valor: Float
    ): Response<ResponseBody>

    @GET("get_sensor.php")
    suspend fun getSensorValue(@Query("nombre") nombre: String): Response<Sensor>
}