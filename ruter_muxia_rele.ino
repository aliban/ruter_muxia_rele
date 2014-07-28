#include <SPI.h>
#include <Ethernet.h>
#include <ICMPPing.h>
#include <dht.h>


// network configuration. dns server, gateway and subnet are optional.

 // the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  

// Definicion de Parametros de red
IPAddress dnServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 252, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip(192, 168, 252, 2);

// Equipo a probar por ping

IPAddress pingAddr(74,125,26,147);

SOCKET pingSocket = 0;

char ping_buffer [256];
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

//Numero de paquetes que se deben de perder antes de reiniciar el router
int max_paquetes_fallidos=10;
int paquetes_fallidos=0;

//Numero de milisegundos de pausa antes de volver a reiniciar el rotuer desde el inicio

int tiempo_arranque=45000;

// Variables para el sensor de temperatura


void setup() {
  Serial.begin(9600);

  // initialize the ethernet device
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  //print out the IP address
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());
  // Salida digitales de control 8 para paquetes perdidos (LED ROJO)
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  // Salida digital de control 9 el sistema esta funcionando OK (LED VERDE)
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  // Salida digital para el control de rele de reinicio de router
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
}


void loop() {  
  
  ICMPEchoReply echoReply = ping(pingAddr, 4);
  if (echoReply.status == SUCCESS)
  {
    digitalWrite(8, LOW);
    digitalWrite(9, HIGH);
        
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
            
             
     paquetes_fallidos=0;
      
  }
  else
  {
    
    sprintf(ping_buffer, "Echo request failed; %d", echoReply.status);
    digitalWrite(8, HIGH);
    paquetes_fallidos++;
    if (paquetes_fallidos >= max_paquetes_fallidos)
    {
     digitalWrite(9, HIGH);
     digitalWrite(7, HIGH);
     delay (1000);
     digitalWrite(7, LOW);
     paquetes_fallidos=0;
     delay (tiempo_arranque);
    }    
    digitalWrite(9, LOW);
  }
  Serial.println(ping_buffer);
  delay(1000);
}
