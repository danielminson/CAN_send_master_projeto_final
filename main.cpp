#include "mbed.h"

//-------Declara uso da serial---------------------
Serial pc(USBTX, USBRX); // tx, rx

//----------Pinos de entrada analogica (ADC)------
AnalogIn AXIS_X(A3);
AnalogIn AXIS_Y(A2);
//----------Pinos de entrada digital------
DigitalIn botao_SW1(D6); // botão usuário shield
DigitalIn botao_SW2(D7); // botão usuário shield

//--------Declara uso da CAN----------------------
CAN can1(PB_8, PB_9);

//--------Declara uso do LED nativo da placa------
DigitalOut LED(PA_5);

//-----Declara uso do botao nativo da placa--------
DigitalIn SW(PC_13);

//---------Dados em hexadecimal de cada mensagem-------
static char info = 0x55;
static char left = 0x01;
static char right = 0x02;
static char up = 0x03;
static char down = 0x04;
static char stop = 0x05;
static char vidro_up = 0x06;
static char vidro_down = 0x07;
static char vidro_stop = 0x08;
static char marcha_lenta_1 = 0x09;
static char marcha_lenta_2 = 0x10;
static char marcha_lenta_3 = 0x11;

Timer debounce_real;
InterruptIn botao(PC_13); // definição do botão de usuário
  int estado = -3;

int main() {
  botao_SW1.mode(PullUp);
  botao_SW2.mode(PullUp);

  //-----atribuição dos dados as mensagens----------
  CANMessage msg(5, &info, 8, CANData, CANStandard);
  CANMessage msg_up(5, &up, 8, CANData, CANStandard);
  CANMessage msg_down(5, &down, 8, CANData, CANStandard);
  CANMessage msg_left(5, &left, 8, CANData, CANStandard);
  CANMessage msg_right(5, &right, 8, CANData, CANStandard);
  CANMessage msg_stop(5, &stop, 8, CANData, CANStandard);
  CANMessage msg_up_vidro(5, &vidro_up, 8, CANData, CANStandard);
  CANMessage msg_down_vidro(5, &vidro_down, 8, CANData, CANStandard);
  CANMessage msg_stop_vidro(5, &vidro_stop, 8, CANData, CANStandard);
  CANMessage msg_marcha_lenta1(6, &marcha_lenta_1, 8, CANData, CANStandard);
  CANMessage msg_marcha_lenta2(6, &marcha_lenta_2, 8, CANData, CANStandard);
  CANMessage msg_marcha_lenta3(6, &marcha_lenta_3, 8, CANData, CANStandard);
  //---boas vindas------------
  pc.printf("Ola!!!\n");

  float VAL_X = 0.0f, VAL_Y = 0.0f;
  int SW1 = 1;
  int SW2 = 1;

  debounce_real.start();
  int estado_marcha_lenta = 1;

  debounce_real.start();

  void bot_pressed(void);
  botao.fall(&bot_pressed);

  while (1) {
    //------Leitura do joystick nos eixos X e Y --------------
    VAL_X = AXIS_X.read();
    VAL_Y = AXIS_Y.read();
    SW1 = botao_SW1;
    SW2 = botao_SW2;

    if (estado == -3) {
      pc.printf("\n\rescolha o modo de operacao do carro!");
      estado = -2;
    }
    if (estado == -2) {
      // pc.printf("%d",estado);
      char s = pc.getc();
      // pc.printf("digite o número desejado");
      // Se o comando for 1 = temos o estado ligado (enable)
      if (s == '1' && estado == -2) {

        pc.printf("estado 1\r\n");
        if (can1.write(msg_marcha_lenta1)) {
          pc.printf("marcha lenta1 enviada\n");
          estado = 0;

        }

        else {
          pc.printf("marcha lenta1 NAO enviada\n");
        }
        wait_us(1000);
      }
      // Se o comando for 6 = temos o estado desligado (disable)
      if (s == '2' && estado == -2) {

        pc.printf("estado 2\r\n");
        if (can1.write(msg_marcha_lenta2)) {
          pc.printf("marcha lenta2 enviada\n");
          estado = 0;
        }

        else {
          pc.printf("marcha lenta2 NAO enviada\n");
        }

        wait_us(1000);
      }
      // Se o comando for 2 = temos a alteração entre o perfil trapezoidal e o
      // perfil "manual"
      if (s == '3' && estado == -2) {
        s = '\0';
        if (can1.write(msg_marcha_lenta1)) {
          pc.printf("marcha lenta1 enviada\n");
          estado = 0;
        }

        else {
          pc.printf("marcha lenta1 NAO enviada\n");
        }
        pc.printf("estado 3\r\n");
        wait_us(1000);
      }
      // wait_us(1000);
    }

    //----Envia mensagem para +Y -----
    if (VAL_X > 0.7f) {
      if (can1.write(msg_right)) {
        pc.printf("Direita enviada\n");
      }

      else {
        pc.printf("Direita NAO enviada\n");
      }
    }

    //----Envia mensagem para -Y -----
    if (VAL_X < 0.3f) {
      // printf("\n\rLEFT");
      if (can1.write(msg_left)) {
        pc.printf("Esquerda enviada\n");
      }

      else {
        pc.printf("Esquerda NAO enviada\n");
      }
    }

    //----Envia mensagem para +X -----
    if (VAL_Y > 0.7f) {
      // printf("\n\rUP");
      if (can1.write(msg_up)) {
        pc.printf("Cima enviada\n");
      }

      else {
        pc.printf("Cima NAO enviada\n");
      }
    }

    //----Envia mensagem para -X -----
    if (VAL_Y < 0.3f) {
      // printf("\n\rDown");
      if (can1.write(msg_down)) {
        pc.printf("Baixo enviada\n");
      }

      else {
        pc.printf("Baixo NAO enviada\n");
      }
    }

    //----Se o joystick estiver em repouso, envia mensagem para parar os motores
    // dos eixos--------
    if (((VAL_Y < 0.65f) && (VAL_Y > 0.45f)) &&
        ((VAL_X < 0.65f) && (VAL_X > 0.45f))) {
      // printf("\n\rStop");
      if (can1.write(msg_stop)) {
        pc.printf("Parada enviada\n");
      }

      else {
        pc.printf("Parada NAO enviada\n");
      }
    }

    if (SW1 == 0) { // Botão usuário pressionado
      if (can1.write(msg_up_vidro)) {
        pc.printf("Vidro cima enviado\n");
      }

      else {
        pc.printf("Vidro cima NAO enviado\n");
      }
    }

    if (SW2 == 0) { // Botão externo pressionado
      if (can1.write(msg_down_vidro)) {
        pc.printf("Vidro baixo enviado\n");
      }

      else {
        pc.printf("Vidro baixo enviado NAO enviado\n");
      }
    }
    if (SW1 == 1 && SW2 == 1) { // Botões usuário NÃO pressionado
      if (can1.write(msg_stop_vidro)) {
        pc.printf("Vidro parado enviado\n");
      }

      else {
        pc.printf("Vidro parado NAO enviado\n");
      }
    }
    msg.data[0] = 0x00; // limpa a variavel

    wait_us(200000); // controla o envio/recebimento de mensagens
  }
}

void bot_pressed() {
  if (debounce_real.read_ms() > 250) {
    estado = -3;
    debounce_real.reset();
  }
}