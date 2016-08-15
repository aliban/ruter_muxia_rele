//
// Autor: Juan Manuel Sende Sanchez
// Programa: Reinicio router mediante prueba ICMP
// Version: 0.4
//
// Archivo de funciones
//


//
// Reinicia la conexion 3G si no esta activa
//

bool pulsa_inicio_3g (int boton)
{
    digitalWrite(boton, HIGH);
    delay (1000);
    digitalWrite(boton, LOW);
    delay (15000);
    return (true); 
}

bool reinicia_router (int boton)
{
  digitalWrite(boton, HIGH);
  delay (1000);
  digitalWrite(boton, LOW);
  return (true);
}

bool inicializa_pin (int pin, int modo, int init_value)
{
  pinMode(pin, modo);
  digitalWrite(pin, init_value);
  return (true);
}

bool actualiza_noip (EthernetClient cliente_noip, String hostname, String token)
{
  // if you get a connection, report back via serial:
  if (cliente_noip.connect("dynupdate.no-ip.com", 80)) {
    Serial.println("Conectado a NO-IP");
    // Make a HTTP request:
    //replace yourhost.no-ip.org with your no-ip hostname
    String url = "GET /nic/update?hostname="+ hostname +" HTTP/1.0";
    cliente_noip.println(url);
    cliente_noip.println("Host: dynupdate.no-ip.com");
    // Usuario y clave codificada en base64
    String auth = "Authorization: Basic "+ token;
    cliente_noip.println(auth);
    cliente_noip.println("User-Agent: Arduino Sketch/1.0 aliban@acoruxa.net");
    cliente_noip.println();
    return (true);
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("Conexion Fallida a NO-IP");
    return (false);
  }
}
