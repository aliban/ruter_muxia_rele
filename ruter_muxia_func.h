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
