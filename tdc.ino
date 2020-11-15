// PINES
const int mando = 4;
const int led = 13;

// Infrarrojos de posición
const int POS_DD = 0;
const int POS_DI = 1;
const int POS_TD = 2;
const int POS_TI = 3;

// Ultrasonidos para detectar contrincante
const int TRG_DD = 7;
const int TRG_DI = 6;
const int TRG_D = 5;
const int TRG_I = 4;

const int ECH_DD = 3;
const int ECH_DI = 2;
const int ECH_D = 1;
const int ECH_I = 0;

// Llaves para puente H
const int Q1_DERECHO = 13;
const int Q2_DERECHO = 12;
const int Q1_IZQUIERDO = 11;
const int Q2_IZQUIERDO = 10;

// Direcciones motor
const int ADELANTE = 1;
const int ATRAS = -1;

// Duraciones movimiento
const int TMP_GIRO_90 = 500;  // TODO: debería probarse hasta encontrar el valor
const int TMP_GIRO_45 = 500;  // TODO: debería probarse hasta encontrar el valor
const int TMP_GIRO_35 = 500;  // TODO: debería probarse hasta encontrar el valor
const int TMP_MOVER = 1000;
const int TMP_ATACAR = 5000;

// Variables
bool corriendo;
//bool mandoPresionado;

void setup() {

  pinMode(mando, INPUT);           // Input del mando
  pinMode(led, OUTPUT);            // Led de encendido
  pinMode(POS_DD, INPUT);          // CYN Delantero Derecho
  pinMode(POS_DI, INPUT);          // CYN Delantero Izquierdo
  pinMode(POS_TD, INPUT);          // CYN Trasero Derecho
  pinMode(POS_TI, INPUT);          // CYN Trasero Izquierdo

  corriendo = false;
}

void loop() {
  verificarMando(); // Controla que se haya pulsado el mando remoto y apagar - prender el arduino

  if(corriendo){
    ubicarse();       // Evita salirse de la pista
    atacar();         // Si ve un enemigo lo ataca
  }
}

// Funciones
void verificarMando(){
  // Si se presiono el mando remoto
  if(digitalRead(mando) == HIGH){ 
      if(corriendo){
        corriendo = false;
      }
      else{
        corriendo = true;
        delay(5000);
      }
  }
}

void ubicarse(){
  // Lee todos los sensores
  bool dd = digitalRead(POS_DD) == HIGH;
  bool di = digitalRead(POS_DI) == HIGH;
  bool td = digitalRead(POS_TD) == HIGH;
  bool ti = digitalRead(POS_TI) == HIGH;

  // Detiene el robot
  detener();
  
  if(dd && td){ // Borde a la derecha
    girarIzquierda(TMP_GIRO_90);
    avanzar(ADELANTE, TMP_MOVER);
  }

  else if(di && ti){ // Borde a la derecha
    girarDerecha(TMP_GIRO_90);
    avanzar(ADELANTE, TMP_MOVER);
  }

  else if(dd && di){ // Borde delante
    avanzar(ATRAS, TMP_MOVER);
  }
  
  else{ // Borde atrás o sin borde
    avanzar(ADELANTE, TMP_MOVER);
  }
}

// Va con las ruedas para adelante
void avanzar(int direccion, int duracion){
  avanzarDerecha(direccion, duracion);
  avanzarIzquierda(direccion, duracion);
}

// Movimiento de la rueda derecha
void avanzarDerecha(int direccion, int duracion){
  if(direccion == ADELANTE){
    digitalWrite(Q1_DERECHO, HIGH);
    digitalWrite(Q2_DERECHO, LOW);
  }

  else{
    digitalWrite(Q1_DERECHO, LOW);
    digitalWrite(Q2_DERECHO, HIGH);    
  }

  delay(duracion);
  detener();
}

// Movimiento de la rueda izquierda
void avanzarIzquierda(int direccion, int duracion){
  if(direccion == ADELANTE){
    digitalWrite(Q1_IZQUIERDO, HIGH);
    digitalWrite(Q2_IZQUIERDO, LOW);
  }

  else{
    digitalWrite(Q1_IZQUIERDO, LOW);
    digitalWrite(Q2_IZQUIERDO, HIGH);    
  }

  delay(duracion);
  detener();
}

// Detiene todas las ruedas
void detener(){
  for(int pin = 10; pin < 14; pin++){
    digitalWrite(pin, LOW);
  }
}

void girarIzquierda(int tiempo){
  detener();
  digitalWrite(Q1_DERECHO, HIGH);
  delay(tiempo);
  digitalWrite(Q1_DERECHO, LOW);
}

void girarDerecha(int tiempo){
  detener();
  digitalWrite(Q1_IZQUIERDO, HIGH);
  delay(tiempo);
  digitalWrite(Q1_IZQUIERDO, LOW);  
}

void atacar(){
  // Detecta objetos en todos los sensores
  bool oponenteEnDD = detectaOponente(TRG_DD, ECH_DD);
  bool oponenteEnDI = detectaOponente(TRG_DI, ECH_DI);
  bool oponenteEnD = detectaOponente(TRG_D, ECH_D);
  bool oponenteEnI = detectaOponente(TRG_I, ECH_I);

  // Estrategias según donde lo encuentra
  if(oponenteEnI){    // Si está a la izquierda
    girarIzquierda(TMP_GIRO_90);
  }
  
  else if(oponenteEnD){     // Si está a la derecha
    girarDerecha(TMP_GIRO_90);
  }
  
  else if(oponenteEnDI && oponenteEnDD){  // Si está en ambos delanteros
    avanzar(ADELANTE, TMP_ATACAR);
  }

  else if(oponenteEnDI){    // Si sólo está en el delantero izquierdo
    girarIzquierda(TMP_GIRO_35);
  }

  else if(oponenteEnDD){    // Si sólo está en el delantero derecho
    girarDerecha(TMP_GIRO_35);
  }

  else{
    girarDerecha(TMP_GIRO_45);
  }
}

bool detectaOponente(int trigger, int echo){
  digitalWrite(trigger, HIGH);
  delay(1);
  digitalWrite(trigger, LOW);
  int duracion = pulseIn(echo, HIGH);
  int distancia = duracion / 58.2;  // cm. Valor especificado por el fabricante del sensor
  return distancia < 80;
} 
