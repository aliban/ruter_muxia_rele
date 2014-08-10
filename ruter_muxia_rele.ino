//
// Autor: Juan Manuel Sende Sanchez
// Programa: Reinicio router mediante prueba ICMP
// Version: 0.4
//

#include <SPI.h>
#include <Ethernet.h>
#include <ICMPPing.h>
#include <dht.h>
#include "ruter_muxia_func.h"


// network configuration. dns server, gateway and subnet are optional.

 // the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  

  // Definicion de Parametros de red
IPAddress dnServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 1, 0);
//IPAddress ip(192, 168, 1, 2);
IPAddress ip(192, 168, 2, 2);

  // Equipo a probar por ping

//IPAddress pingAddr(74,125,26,147);
IPAddress pingAddr(192,168,2,3);


SOCKET pingSocket = 0;

char ping_buffer [256];
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));


// Actualizacion NO-IP

EthernetClient cliente_noip;

// Numero de paquetes que se deben de perder antes de 7r el router

int paquetes_fallidos=15;
int con_paquetes_fallidos=0;
int max_paquetes_fallidos;

// Numero de milisegundos de pausa que tarda en arrancar el router

long tiempo_arranque=50000;

// En arranque esperamos el doble de paquetes

bool estado_arranque=true;
int multiplicador_arranque=2;


// Variables de ubicacion de reles

int reinicia=7;
int reconecta=6;
int led_desconectado=8;
//
//int led_conectado=9;
//
//Para itboard usamos el 2
int led_conectado=2;

bool actualiza_noip ()
{
  // if you get a connection, report back via serial:
  if (cliente_noip.connect("dynupdate.no-ip.com", 80)) {
    Serial.println("Conectado a NO-IP");
    // Make a HTTP request:
    //replace yourhost.no-ip.org with your no-ip hostname
    cliente_noip.println("GET /nic/update?hostname=XXXXXX.no-ip.org HTTP/1.0");
    cliente_noip.println("Host: dynupdate.no-ip.com");
    // Usuario y clave codificada en base64
    cliente_noip.println("Authorization: Basic XXXXX"); 
    cliente_noip.println("User-Agent: Arduino Sketch/1.0 aliban@acoruxa.net");
    cliente_noip.println();
    return (true);
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("Conexion Fallid a NO-IP");
    return (false);
  }
}

void setup() {
  Serial.begin(9600);

  // initialize the ethernet device
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  //print out the IP address
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());
  // Salida digitales de control 8 para paquetes perdidos (LED ROJO)
  pinMode(led_desconectado, OUTPUT);
  digitalWrite(led_desconectado, LOW);
  // Salida digital de control 9 el sistema esta funcionando OK (LED VERDE)
  pinMode(led_conectado,OUTPUT);
  digitalWrite(led_conectado, LOW);
  // Salida digital para el control de rele de reinicio de router
  pinMode(reinicia, OUTPUT);
  digitalWrite(reinicia, LOW);
  // Pulsador de conexion 3G
  pinMode(reconecta, OUTPUT);
  digitalWrite(reconecta, LOW);
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
       pulsa_inicio_3g (reconecta);
       max_paquetes_fallidos=paquetes_fallidos*multiplicador_arranque;
       }
     else
       {
         Serial.println("Ya estamos conectados no reiniciamos 3G");

       }
     actualiza_noip();
     estado_arranque=false;
   }
   
  ICMPEchoReply echoReply = ping(pingAddr, 4);
  if (echoReply.status == SUCCESS)
  {
    digitalWrite(led_desconectado, LOW);
    digitalWrite(led_conectado, HIGH);    
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
  }
  else
  {
    sprintf(ping_buffer, "Echo request failed; %d", echoReply.status);
    digitalWrite(led_desconectado, HIGH);
    con_paquetes_fallidos++;
    if (con_paquetes_fallidos >= max_paquetes_fallidos)
    {
     digitalWrite(led_conectado, HIGH);
     Serial.println("Reiniciando router");
     estado_arranque = reinicia_router (reinicia);
     con_paquetes_fallidos=0;    
    }    
    digitalWrite(led_conectado, LOW);
  }
  Serial.println(ping_buffer);
  delay(1000);
}
