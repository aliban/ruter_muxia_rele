//
// Autor: Juan Manuel Sende Sanchez
// Programa: Reinicio router mediante prueba ICMP
// Version: 0.4
//

#include <SPI.h>
#include <Ethernet.h>
#include <ICMPPing.h>
#include <dht.h>
#include <string.h>

#include "plotly_streaming_ethernet.h"

#include "ruter_muxia_func.h"


// network configuration. dns server, gateway and subnet are optional.

 // the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  

  // Definicion de Parametros de red
IPAddress dnServer(8, 8, 8, 8);
//IPAddress gateway(192, 168, 1, 1);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
//IPAddress ip(192, 168, 1, 2);
IPAddress ip(192, 168, 2, 2);

  // Equipo a probar por ping

IPAddress pingAddr(74,125,26,147);
//IPAddress pingAddr(192,168,2,3);


SOCKET pingSocket = 0;

char ping_buffer [256];
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));


// Actualizacion NO-IP

EthernetClient cliente_noip;

String HOSTNAME="playalago.ddns.net";
String TOKEN="YWxpYmFuOmFuYWtpcmE=";

// Numero de paquetes que se deben de perder antes de reiniciar el router

int paquetes_fallidos=15;
int con_paquetes_fallidos=0;
int max_paquetes_fallidos;

// Numero de milisegundos de pausa que tarda en arrancar el router

long tiempo_arranque=50000;

// En arranque esperamos el doble de paquetes

bool estado_arranque=true;
int multiplicador_arranque=2;


// Definci´n de PINs reles/leds

#define SW_REINICIA 7
#define SW_RECONECTA 6
#define LED_DESCONECTADO 8
//
//#define LED_CONECTADO 9;
//
//Para itboard usamos el 2
#define LED_CONECTADO 2

// Definicion de PINs DHT22

#define DHT_PIN 3 
dht DHT;

// Envio de datos a Internet

#define nTraces 2
char *tokens[nTraces] = {"ckgvahmyj5", "a1uc2wsvrr"};
plotly graph = plotly("aliban", "53bxzx1ek1", tokens, "tmp_humedad_playalago", nTraces);
unsigned long proxima_medicion = 0;


void setup() {
  Serial.begin(9600);

  // initialize the ethernet device
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  //print out the IP address
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());
  
  // Inicializamos los reles
  inicializa_pin (LED_DESCONECTADO, OUTPUT, LOW);
  inicializa_pin (LED_CONECTADO, OUTPUT, LOW);
  inicializa_pin (SW_REINICIA, OUTPUT, LOW);
  inicializa_pin (SW_RECONECTA, OUTPUT, LOW);
  
}


void loop() {  
  if (estado_arranque)
   {
     Serial.println("Esperando a que arranque el router");
     delay (tiempo_arranque);
     ICMPEchoReply echoReply = ping(pingAddr, 4);
     if (echoReply.status != SUCCESS)
       {
       Serial.println("Pulsando el boton de conexion");
       pulsa_inicio_3g (SW_RECONECTA);
       max_paquetes_fallidos=paquetes_fallidos*multiplicador_arranque;
       }
     else
       {
         Serial.println("Ya estamos conectados no reiniciamos 3G");

       }
     actualiza_noip(cliente_noip, HOSTNAME, TOKEN);
    
     // Inicializamos los graficos de Plotly

    graph.init();
    graph.log_level = 4;
    graph.timezone = "Europe/Madrid";
    graph.fileopt="overwrite";
    graph.openStream();
    
    estado_arranque=false;
   }
   
  ICMPEchoReply echoReply = ping(pingAddr, 4);
  if (echoReply.status == SUCCESS)
  {
    digitalWrite(LED_DESCONECTADO, LOW);
    digitalWrite(LED_CONECTADO, HIGH);    
    delay (400);  
    sprintf(ping_buffer,
            "Reply[%d] from: %d.%d.%d.%d: bytes=%d time=%ldms TTL=%d",
            echoReply.data.seq,
            echoReply.addr[0],
            echoReply.addr[1],
            echoReply.addr[2],
            echoReply.addr[3],
            REQ_DATASIZE,
            millis() - echoReply.data.time,
            echoReply.ttl);
    con_paquetes_fallidos=0;
    max_paquetes_fallidos=paquetes_fallidos;
   if (millis() > proxima_medicion)
     {
     // Leemos la temperatura y humedad
     DHT.read22(DHT_PIN);
     // Lo transformamos en cadenas
     String hum = String((int) DHT.humidity);
     String temp = String((int) DHT.temperature);
     // Mostramos los resultados 
     Serial.print("Temperatura: ");
     Serial.print(temp);
     Serial.print("    Humedad: ");
     Serial.println(hum);  
    // Ploteamos los resultados en Plotly
    graph.plot(millis(), (int) DHT.temperature, tokens[0]);
    graph.plot(millis(), (int) DHT.humidity, tokens[1]);
    // Proxima medicion en 15 minutos
    proxima_medicion=millis()+900000;
   }
  }
  else
  {
    sprintf(ping_buffer, "Echo request failed; %d", echoReply.status);
    digitalWrite(LED_DESCONECTADO, HIGH);
    con_paquetes_fallidos++;
    if (con_paquetes_fallidos >= max_paquetes_fallidos)
    {
     digitalWrite(LED_CONECTADO, HIGH);
     Serial.println("Reiniciando router");
     estado_arranque = reinicia_router (SW_REINICIA);
     con_paquetes_fallidos=0;    
    }    
    digitalWrite(LED_CONECTADO, LOW);
  }
  Serial.println(ping_buffer);
  delay(1000);
}
