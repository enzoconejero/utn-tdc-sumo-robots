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
const int Q1_IZQUIERDO = 9;
const int Q2_IZQUIERDO = 8;

// MOSFET Puente H
const int MOSFET_DERECHO = 11;
const int MOSFET_IZQUIERDO = 10;

// Distancia máxima de oponente en cm
const int DISTANCIA_MAXIMA = 100;

// Potencias MOSFET
const int PW_STOP = 0;
const int PW_WALK = 64;
const int PW_MAX = 255;
const int PW_GIRO = 255; // TODO: debería probarse hasta encontrar el valor

// TODO: Enum + switch cases ?
// Direcciones motor
const int ADELANTE = 1;
const int ATRAS = -1;
const int DERECHA = 2;
const int IZQUIERDA = 3;

// Duraciones movimiento
const int TMP_GIRO_90 = 500;  // TODO: debería probarse hasta encontrar el valor
const int TMP_GIRO_45 = 500;  // TODO: debería probarse hasta encontrar el valor
const int TMP_GIRO_35 = 500;  // TODO: debería probarse hasta encontrar el valor
const int TMP_MOVER = 1000;   // TODO: debería probarse hasta encontrar el valor
const int TMP_ATACAR = 5000;

// Linea blanca infrarrojo
const int LINEA_BLANCA = 300;

class Ultrasonido{
  private:
    int trigger;
    int echo;
    int distancia;
  
  public:
    Ultrasonido(int _trigger, int _echo){
      trigger = _trigger;
      echo = _echo;
    }

    bool buscarOponente(){
      digitalWrite(trigger, HIGH);
      delay(1);
      digitalWrite(trigger, LOW);
      int duracion = pulseIn(echo, HIGH);
      distancia = duracion / 58.2;  // cm. Valor especificado por el fabricante del sensor
    }

    int detectaOponente(){
      return distancia < DISTANCIA_MAXIMA;  
    }

    int velocidadAtaque(){ // Entre PW_WALK y PW_MAX proporcional a la distancia entre 0 y 80
      if(distancia >= 100){
        return 0;  
      }
      int vel = map(distancia, 0, 100, PW_WALK, PW_MAX);
      return vel;
    }
};

// Conjunto motor + mosfet + rueda + puente H
// Mueve, direcciona, frena
class Rueda{
  private:
    int q1;
    int q2;
    int mosfet;

  public:
    Rueda(int _q1, int _q2, int _mosfet){
      q1 = _q1;
      q2 = _q2;
      mosfet = _mosfet;
    }

    void mover(int direccion){
      if(direccion == ADELANTE){
        digitalWrite(q1, HIGH);
        digitalWrite(q2, LOW);
      }
      else{
        digitalWrite(q1, LOW);
        digitalWrite(q2, HIGH);
      }
    }
};

// Variables
bool corriendo;
Ultrasonido usDD = Ultrasonido(TRG_DD, ECH_DD);
Ultrasonido usDI = Ultrasonido(TRG_DI, ECH_DI);
Ultrasonido usD = Ultrasonido(TRG_D, ECH_D);
Ultrasonido usI = Ultrasonido(TRG_I, ECH_I);

Rueda rd = Rueda(Q1_DERECHO, Q2_DERECHO, MOSFET_DERECHO);
Rueda ri = Rueda(Q1_IZQUIERDO, Q2_IZQUIERDO, MOSFET_IZQUIERDO);

void setup() {

  pinMode(TRG_DD, OUTPUT);            // Trigger del ultrasonido Delantero Derecho
  pinMode(TRG_DI, OUTPUT);            // Trigger del ultrasonido Delantero Izquierdo
  pinMode(TRG_D, OUTPUT);             // Trigger del ultrasonido Derecho
  pinMode(TRG_I, OUTPUT);             // Trigger del ultrasonido Izquierdo

  pinMode(ECH_DD, INPUT);             // Echo del ultrasonido Delantero Derecho
  pinMode(ECH_DI, INPUT);             // Echo del ultrasonido Delantero Izquierdo
  pinMode(ECH_D, INPUT);              // Echo del ultrasonido Derecho
  pinMode(ECH_I, INPUT);              // Echo del ultrasonido Izquierdo

  pinMode(Q1_DERECHO, OUTPUT);        // Q1 Puente H - Motor Derecho
  pinMode(Q2_DERECHO, OUTPUT);        // Q2 Puente H - Motor Derecho
  pinMode(Q1_IZQUIERDO, OUTPUT);      // Q1 Puente H - Motor Izquierdo
  pinMode(Q2_IZQUIERDO, OUTPUT);      // Q2 Puente H - Motor Izquierdo

  pinMode(MOSFET_DERECHO, OUTPUT);    // Mosfet Puente H - Motor Derecho
  pinMode(MOSFET_IZQUIERDO, OUTPUT);  // Mosfet Puente H - Motor Izquierdo
  
  corriendo = false;
}

void loop() {
  verificarMando(); // Controla que se haya pulsado el mando remoto y apagar - prender el arduino

  if(corriendo){
    ubicarse();       // Evita salirse de la pista
    atacar();         // Si ve un enemigo lo ataca
  }
}

// Encendido y apagado
void verificarMando(){
  // Si se presiono el mando remoto
  if(digitalRead(mando) == HIGH){ 
      if(corriendo){
        corriendo = false;
      }
      else{
        corriendo = true;
        delay(5000);  // TODO: hacer constante
      }
  }
}

// Retorna si un sensor detecta línea blanca, 
// pasa cuando la lectura analógica da un valor > a cierto minimo (300 para probar)
bool detectaLineaBlanca(int sensor){
  return analogRead(POS_DD) >= LINEA_BLANCA;
}

// Si detecta linea blanca en algún sensor se aleja de la misma
void ubicarse(){
  // Lee todos los sensores
  bool dd = detectaLineaBlanca(POS_DD);
  bool di = detectaLineaBlanca(POS_DI);
  bool td = detectaLineaBlanca(POS_TD);
  bool ti = detectaLineaBlanca(POS_TI);

  if(dd || di || td || ti){
    setVelocidad(PW_WALK);
  
    if(dd && td){ // Borde a la derecha
      girarIzquierda(TMP_GIRO_90);
      avanzar(ADELANTE);
    }
  
    else if(di && ti){ // Borde a la derecha
      girarDerecha(TMP_GIRO_90);;
      avanzar(ADELANTE);
    }
  
    else if(dd && di){ // Borde delante
      avanzar(ATRAS);
    }
  
    else if(td && ti){ // Borde atrás o sin borde
      avanzar(ADELANTE);
    }
  }

}

// Va con las ruedas para adelante
void avanzar(int direccion){
  rd.mover(direccion);
  ri.mover(direccion);
}

void setVelocidad(int velocidad){
  analogWrite(MOSFET_DERECHO, velocidad);
  analogWrite(MOSFET_IZQUIERDO, velocidad);
}

// TODO: unir giro izquierdo y derecho
// Gira el robot por un cierto tiempo, más tiempo => más angulo
// Durante este tiempo no se hará otra cosa que girar
void girarIzquierda(int tiempo){
  setVelocidad(PW_GIRO);
  rd.mover(ATRAS);
  ri.mover(ADELANTE);
  delay(tiempo);
  avanzar(ADELANTE);
}

void girarDerecha(int tiempo){
  setVelocidad(PW_GIRO);
  rd.mover(ATRAS);
  ri.mover(ADELANTE);
  delay(tiempo);
  avanzar(ADELANTE);
}

void atacar(){
  // Detecta objetos en todos los sensores
  usDD.buscarOponente();
  usDI.buscarOponente();
  usD.buscarOponente();
  usI.buscarOponente();

  // Estrategias según donde lo encuentra
  // Si está a la izquierda
  if(usI.detectaOponente()){    
    girarIzquierda(TMP_GIRO_90);
    setVelocidad(PW_WALK);
  }

  // Si está a la derecha
  else if(usD.detectaOponente()){     
    girarDerecha(TMP_GIRO_90);
    setVelocidad(PW_WALK);
  }

  // Si está en ambos delanteros
  else if(usDD.detectaOponente() && usDI.detectaOponente()){  
    setVelocidad(usDD.velocidadAtaque());
    avanzar(ADELANTE);
  }

  // Si sólo está en el delantero izquierdo
  else if(usDI.detectaOponente()){    
    girarIzquierda(TMP_GIRO_35);
    setVelocidad(PW_WALK);
  }

  else if(usDD.detectaOponente()){    // Si sólo está en el delantero derecho
    girarDerecha(TMP_GIRO_35);
    setVelocidad(PW_WALK);
  }

  else{
    girarDerecha(TMP_GIRO_45);
  }
}
