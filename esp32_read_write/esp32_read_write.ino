#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


//define el número de bytes que recibimos por cada entero
#define NUM_BYTES_ENTERO 2
//define el número de bytes que recibimos por todo el array
//si fuese dinamico el manejo de memoria podría omitirse esto
#define NUM_BYTES_ARRAY 300
//delay en ms entre animaciones de anuncios
#define DELAY_ANIMACION 200
//número que debe de recibir para que se considere inicio de arreglo recibido
#define INICIA_ARRAY 3000
//número que debe de recibir para que se considere fin de arreglo recibido
#define FIN_ARRAY 3100
 
int LED = 13; //led del arduino
int numBytes = -1; //indica el número de bytes que se recibe por bt

//flag que indica si se están recibiendo datos
bool isReceiving = false;

//tengo problemas en la asignación de memoria dinámica, por ello lo puse estática
//pero no será eficiente si se recibirán muchos arrays, quiza sea necesario añadir memoria 
/*
int *msjFinal;//array dinámico de datos recibidos en valores enteros
byte *msjTmp;//array dinámico de datos recibidos
*/
int msjFinal[NUM_BYTES_ARRAY/NUM_BYTES_ENTERO];
//array que se le regresa a android con un valor que representa un código
byte msjRetorno[NUM_BYTES_ENTERO];

//checará el byte entrante para verificar si se trata de inicio de array de datos entrante
byte dummy[NUM_BYTES_ENTERO];
//controla el índice del array dummy, solo ayuda a saber si se recibió código de inicio o de fin
byte dummyIndex = 0;

//indice que controla el lugar de escritura del byte de entrada leido una vez que se esta llenando el array entrante
int indexFinal = 0;

//tamaño de array de entrada
int tamArray = 0;

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("DEVI"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
/*
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);
  */

 
  
  checarDatosRec();

  
} 

//llena el array de datos si es que hay que hacerlo
void checarDatosRec(){

  int intValue;//dato entero recibido
  int estado;//estado en que está el arduino al recibir dato 

  if (SerialBT.available()>0){ 

    /*el buffer de lectuura por default almacena 64 bytes, sin embargo, se envian más de 64 bytes en cada arreglo
    se debe de buscar la manera de que se lean todos estos bytes que se envíen, hay varias opciones:

    1.- modificar cosntante #define _SS_MAX_RX_BUFF del archivo SoftwareSerial.h por un valor más alto, archivo ubicado en la carpeta de arduino
    en la pc instalada en la ruta Arduino\hardware\arduino\avr\libraries\SoftwareSerial\src pero esto genera un mayor uso de memoria

    2.- leer serial hasta que se encuentre la constante de fin de array #define FIN_ARRAY, no he logrado que funcione, no lee todo el 
    array de bytes enviado

    AGREGA AQUÍ MAS OPCIONES...

    */
    //SerialBT.flush();
    
    ////numero de bytes recibidos
    numBytes = SerialBT.available();
    
    //Serial.print("numBytes recibidos = "); 
    //Serial.println(numBytes);
    //asignarMemoria();

    if(SerialBT.available()){
      dummy[dummyIndex++] = SerialBT.read();

      //cada que se recibe un entero completo se checa su valor y se realiza la accion peritnente según el estado
      if(dummyIndex == NUM_BYTES_ENTERO){
        intValue = bytes2Int(dummy[0],dummy[1]);//obtiene valor entero leido

       //reset a indice de arreglo dummy
        dummyIndex = 0;
        //checar estados
        
        if(isReceiving && intValue != INICIA_ARRAY && intValue != FIN_ARRAY){//llenar array
          //Serial.println("llenando array "); 
          //aqui se escribe en la sd el valor intValue
          msjFinal[indexFinal++] = intValue; 
          tamArray++;
          return;
        }
        else if(intValue == INICIA_ARRAY && !isReceiving){//inicia array
          Serial.println("inicia array ");
          isReceiving = true;
          return;
        }else if(intValue == FIN_ARRAY && isReceiving){//termina array
          Serial.println("termina array ");
          showIntArray(msjFinal, tamArray);
          showLed(msjFinal[0]);      
          
          //llamada a función que descompone el entero en bytes
          intToByteMsjRetorno(&tamArray);    
          SerialBT.write(msjRetorno,2);   
          Serial.print("tamArray = "); 
          Serial.println(tamArray); 
          isReceiving = false;
          indexFinal = 0;
          tamArray = 0;
          return;
        }
      }
    }

  }
}

//descompone el entero data en sus bytes y lo almacena en la variale global msjRetorno
void intToByteMsjRetorno(int *data) {
        msjRetorno[0] = (byte) ((*data & 0x0000FF00) >> 8);
        msjRetorno[1] = (byte) ((*data & 0x000000FF) >> 0);
}

//verifica si hay nuevos datos por recibir o si se han recibido todos
bool checarDato(int valor){
  if(valor == FIN_ARRAY){
    return false;
  }else if(valor == INICIA_ARRAY){
    return true;
  }
}

//realiza asignación de memoria de acuerdo a los bytes recibidos
void asignarMemoria(){
  
  //problemas para el manejo de memoria dinámica, se optó por memoria estática
  //pero no es la mejor opción, dependiendo de la cantidad de anuncios a mostrar
  //quizá sea necesario añadir memoria ram

    /*
    //antes de asignar memoria y valores, liberar memoria, puede ser que previamente haya sido reservada
    if (msjTmp != NULL || msjFinal != NULL){ 
      free(msjTmp);
      free(msjFinal);
      Serial.println("memoria liberada"); 
    }
    Serial.println("memoria ya liberada");
  //asigna tamaño de memoria de acuerdo a tamaño de array recibido, por cada valor  entero recibimos 2 bytes
    msjTmp = (byte*) malloc (numBytes + 1);
    msjFinal = (int*) malloc (numBytes/NUM_BYTES_ENTERO + 1);

    //si falla asignación de memoria mandar mensaje de error
    if (msjTmp == NULL || msjFinal == NULL){
        Serial.println("Asignación de memoria falló!!!!!!");
        return;
    } 
    */  
}

//convierte el array de bytes a array de int, sizeArray
void byteArray2IntArray(byte *src,int *dst,int sizeArraySrc){  
  Serial.println("byte2IntArray()\n");
  int j = 0;
  for(int i=0;i<sizeArraySrc;i=i+NUM_BYTES_ENTERO){
    dst[j++] = bytes2Int(src[i],src[i+1]);    
  } 
}

//muestra el arreglo de bytes recibidos
void showBytesArray(byte *array,int sizeArray){
  Serial.println("showBytesArray()\n");
  
  Serial.print("[");
  for(int i = 0; i<sizeArray;i++){
      Serial.print(array[i]);
      Serial.print(" ");
  }
  Serial.println("]");
}

//muestra el arreglo de int creado del array de bytes recibido
void showIntArray(int *array,int sizeArray){
  Serial.println("showIntArray()\n");
  Serial.print("[");
  for(int indexFinal = 0; indexFinal<sizeArray;indexFinal++){
      Serial.print(array[indexFinal]);
      Serial.print(" ");
  }
  Serial.println("]");  
}

//combina un par de bytes y retorna el entero formado por ambos int -> (b1 | b2)
int bytes2Int(byte b1, byte b2){
  /*
  Serial.print("bytes2Int(): \n");
  Serial.print(b1);
  Serial.print(" ");
  Serial.print(b2);
  */
  
  int entero = b1 << 8 | b2;
  /*
  Serial.print(" ");
  Serial.println(entero);
  */
    return entero;
}

//muestra una animación con las matrices matrix y matrix2
//no es posible determinar el tamaño de un array dentro de la funcion, por ello 
//la funcion debe de recibir el apuntador del array y su tamaño como dos parámetros separados
// https://forum.arduino.cc/index.php?topic=527619.msg3598453#msg3598453
//
void showAnimacion(int *matrix,int size1, int *matrix2,int size2){
  Serial.println("showAnimacion()");
/*
  for(int i=0; i<size1; i++) {
    pixels.setPixelColor(matrix[i], pixels.Color(255,150 ,0 ));
  } 
  pixels.show();
  delay(DELAY_ANIMACION);

  for(int i=0; i<jj; i++) {
    pixels.setPixelColor(matrix[i], pixels.Color(0,0 ,0 ));
  } 
  pixels.show();
  delay(DELAY_ANIMACION);
  
  for(int i=0; i<size2; i++) {
    pixels.setPixelColor(matrix2[i], pixels.Color(255,150 ,0 ));
  }
  pixels.show();
  delay(DELAY_ANIMACION);

  for(int i=0; i<=tamArray; i++) {
    pixels.setPixelColor(matrixPeaton[i], pixels.Color(0,0 ,0 ));
  } 
*/
}

//enciende/apaga led
void showLed(int control){
  Serial.println("showLed()");
  //solo para visualizar primer elemento del array recibido
  if (control > 0){ 
    //endiende led
    digitalWrite(LED, HIGH); 
  } 
  else{ 
    //apaga led
    digitalWrite(LED, LOW); 
  } 
}