#include <Adafruit_Fingerprint.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define SD_CS 15
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCLK 18
#define FINGER_DETECT_PIN 1  // Substitua pelo pino correto
StaticJsonDocument<200> BioData;

const char* ssid = "Uni-CV Digital";
const char* password = "Pn2c7Cc9";
const char* mqtt_server = "62.171.180.52";
const char* topic_sub = "fingerprint_CMD";
const char* topic_pub = "fingerprint_RESP";
const char* topic_pub_registed = "fingerprint_REGISTED";

const int relayPin = 27;  // Defina o pino ao qual o relé está conectado

const char* name_r;
const char* cmd;
const char* status_r;
const char* time1 = "00000";
const char* logg_info;
bool msg_status = false;
String name, status, Device_id = "BSES300T";
const char* Espclient_id = "BSES300T";
bool found = false;
int id;
WiFiClient espClient;
PubSubClient client(espClient);

HardwareSerial mySerial2(2);
HardwareSerial mySerial3(1);

Adafruit_Fingerprint finger1 = Adafruit_Fingerprint(&mySerial2);
Adafruit_Fingerprint finger2 = Adafruit_Fingerprint(&mySerial3);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");

    if (client.connect(Espclient_id)) {
      Serial.println("conectado");
      client.subscribe(topic_sub);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);


  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("Erro ao analisar JSON: ");
    Serial.println(error.c_str());
    return;
  } else {
    msg_status = true;
  }


  name_r = doc["name"];
  cmd = doc["Cmd"];
  status_r = doc["status"];
  int id2mod = doc["id"];

  Serial.print("Nome: ");
  Serial.println(name_r);
  Serial.print("Comando: ");
  Serial.println(cmd);
  Serial.print("Status: ");
  Serial.println(status_r);

  if (strcmp(cmd, "Register_finger_BSE11S300T") == 0) {
    registerFingerprint();
    msg_status = false;
  } else if (strcmp(cmd, "Verify_finger_BSE11S300T") == 0) {
    verifyFingerprint();
    msg_status = false;
  } else if (strcmp(cmd, "Delete_finger_BSE11S300T") == 0) {
    deleteFingerprint();
    msg_status = false;
  } else if (strcmp(cmd, "Delete_all_fingers_BSE11S300T") == 0) {
    deleteAllFingerprints();
    msg_status = false;
  } else if (strcmp(cmd, "Edit_finger_BSE11S300T") == 0) {
    editNameInSD(id2mod, name_r);
    msg_status = false;


  } else if (strcmp(cmd, "Open_Door_BSE11S300T") == 0) {
    open_door();
    msg_status = false;
  } else {
    msg_status = false;
  }
}

void setup() {
  Serial.begin(115200);




  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card MOUNT FAIL");
    while (1)
      ;
  } else {
    Serial.println("SD Card MOUNT SUCCESS");
    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.println("SDCard Size: " + String(cardSize) + "MB");
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
    }
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }
  }

  pinMode(FINGER_DETECT_PIN, INPUT);  // Configura o pino de detecção como entrada
  pinMode(relayPin, OUTPUT);
  mySerial2.begin(57600, SERIAL_8N1, 16, 4);
  mySerial3.begin(57600, SERIAL_8N1, 17, 5);

  finger1.begin(57600);
  finger2.begin(57600);

  Serial.println("Inicializando sensor de impressão digital 1...");
  if (!finger1.verifyPassword()) {
    Serial.println("Sensor de impressão digital 1 não encontrado");
    while (1)
      ;
  }
  Serial.println("Sensor de impressão digital 1 inicializado com sucesso");

  Serial.println("Inicializando sensor de impressão digital 2...");
  if (!finger2.verifyPassword()) {
    Serial.println("Sensor de impressão digital 2 não encontrado");
    while (1)
      ;
  }
  Serial.println("Sensor de impressão digital 2 inicializado com sucesso");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!msg_status) {
    if (digitalRead(FINGER_DETECT_PIN) == HIGH) {
      verifyFingerprint();
    }
  }
  Id_found();
  //msg_status = true;
}

void Id_found() {
  if (found) {
    if (status.toInt() == 1) {

      open_door();
      BioData["id"] = id;
      BioData["name"] = name;
      BioData["status_"] = status;
      BioData["logg_info"] = logg_info;
      BioData["device_id"] = Device_id;
      char payload[256];
      serializeJson(BioData, payload);
      if (client.connected()) {
        client.publish(topic_pub, payload);
        Serial.println("Data published to MQTT.");
      } else {
        Serial.println("MQTT not connected. Data not published.");
      }
      found = false;
    }
  }
}

//Função para Editar o Nome
bool editNameInSD(uint16_t id, String newName) {
  File file = SD.open("/fingerprints.txt");
  if (!file) {
    Serial.println("Falha ao abrir o arquivo fingerprints.txt para leitura");
    return false;
  }
  File tempFile = SD.open("/temp.txt", FILE_WRITE);
  if (!tempFile) {
    Serial.println("Falha ao abrir o arquivo temp.txt para escrita");
    file.close();
    return false;
  }
  bool updated = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    int commaIndex1 = line.indexOf(',');
    int commaIndex2 = line.indexOf(',', commaIndex1 + 1);
    uint16_t currentID = line.substring(0, commaIndex1).toInt();
    if (currentID == id) {
      tempFile.print(id);
      tempFile.print(",");
      tempFile.print(newName);
      tempFile.print(",");
      tempFile.println(line.substring(commaIndex2 + 1));
      updated = true;
    } else {
      tempFile.println(line);
    }
  }
  file.close();
  tempFile.close();
  if (updated) {
    SD.remove("/fingerprints.txt");
    SD.rename("/temp.txt", "/fingerprints.txt");
  } else {
    SD.remove("/temp.txt");
  }
  return updated;
}

//Função para Editar o Status
bool editStatusInSD(uint16_t id, String newStatus) {
  File file = SD.open("/fingerprints.txt");
  if (!file) {
    Serial.println("Falha ao abrir o arquivo fingerprints.txt para leitura");
    return false;
  }
  File tempFile = SD.open("/temp.txt", FILE_WRITE);
  if (!tempFile) {
    Serial.println("Falha ao abrir o arquivo temp.txt para escrita");
    file.close();
    return false;
  }
  bool updated = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    int commaIndex1 = line.indexOf(',');
    int commaIndex2 = line.indexOf(',', commaIndex1 + 1);
    uint16_t currentID = line.substring(0, commaIndex1).toInt();
    if (currentID == id) {
      tempFile.print(id);
      tempFile.print(",");
      tempFile.print(line.substring(commaIndex1 + 1, commaIndex2));
      tempFile.print(",");
      tempFile.println(newStatus);
      updated = true;
    } else {
      tempFile.println(line);
    }
  }
  file.close();
  tempFile.close();
  if (updated) {
    SD.remove("/fingerprints.txt");
    SD.rename("/temp.txt", "/fingerprints.txt");
  } else {
    SD.remove("/temp.txt");
  }
  return updated;
}

void open_door() {
  Serial.println("Ativando relé por 4 segundos...");
  digitalWrite(relayPin, HIGH);  // Ativa o relé
  delay(3000);                   // Aguarda 4 segundos
  digitalWrite(relayPin, LOW);   // Desativa o relé
  Serial.println("Relé desativado.");
}

void registerFingerprint() {
  Serial.println("Coloque o dedo no sensor 1");
  if (client.connected()) {
    client.publish(topic_pub, "Coloque o dedo no sensor 1");
    Serial.println("Data published to MQTT.");
  } else {
    Serial.println("MQTT not connected. Data not published.");
  }

  // Capturar impressão digital do sensor 1
  while (finger1.getImage() != FINGERPRINT_OK) {
    Serial.print(".");
    finger1.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
    delay(1000);  // Adiciona um pequeno atraso para evitar loop rápido
  }
  Serial.println("Imagem capturada com sucesso");

  if (finger1.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Falha ao converter imagem no sensor 1");
    return;
  }
  Serial.println("Imagem convertida com sucesso");

  // Verificar se a impressão já está registrada no sensor 1
  if (finger1.fingerSearch() == FINGERPRINT_OK) {
    id = finger1.fingerID;
    Serial.print("Impressão digital já registrada com ID: ");
    client.publish(topic_pub, "Impressão digital já registrada");
    Serial.println(id);


    if (getNameAndStatus(id, name, status)) {
      Serial.print("Nome: ");
      Serial.println(name);
      Serial.print("Status: ");
      Serial.println(status);
    } else {
      Serial.println("Falha ao recuperar nome e status");
    }
    return;  // Se já estiver registrado, sai da função
  } else {
    Serial.println("Impressão digital não registrada, criando modelo...");

    // Criar modelo se a impressão digital não estiver registrada
    if (finger1.createModel() != FINGERPRINT_OK) {
      Serial.println("Falha ao criar modelo no sensor 1");
      return;
    }
    Serial.println("Modelo criado com sucesso");

    // Gerar ID único
    int id = generateUniqueID();
    Serial.print("ID gerado: ");
    Serial.println(id);

    if (finger1.storeModel(id) != FINGERPRINT_OK) {
      Serial.println("Falha ao armazenar modelo no sensor 1");
      return;
    }
    Serial.println("Modelo armazenado com sucesso no sensor 1");

    // Coletar nome e status do usuário
    String name = name_r;
    name.trim();

    // Verificar se o nome já existe
    if (getIDByName(name) != -1) {
      Serial.println("Nome já registrado. Escolha um nome diferente.");

      finger1.deleteModel(id);  // Remover o modelo do sensor 1
      finger2.deleteModel(id);
      client.publish(topic_pub, "Nome já registrado. Escolha um nome diferente.");
      return;
    }

    String status = status_r;
    status.trim();

    // Salvar informações no cartão SD
    if (!saveIDToSD(id, name, status)) {
      Serial.println("Falha ao salvar informações no cartão SD. Removendo ID do sensor.");
      finger1.deleteModel(id);  // Remover o modelo do sensor 1
      finger2.deleteModel(id);
      return;
    }
    Serial.println("Impressão digital armazenada com sucesso no sensor 1");
    client.publish(topic_pub, "Impressão digital armazenada com sucesso no sensor 1");
    // Registrar a impressão digital no sensor 2 com o mesmo ID
    Serial.println("Coloque o dedo no sensor 2 para registro");
    client.publish(topic_pub, "Coloque o dedo no sensor 2 para registro");

    // Capturar impressão digital do sensor 2
    while (finger2.getImage() != FINGERPRINT_OK) {
      Serial.print(".");
      finger2.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
      delay(1000);  // Adiciona um pequeno atraso para evitar loop rápido
    }
    Serial.println("Imagem capturada com sucesso no sensor 2");

    if (finger2.image2Tz(1) != FINGERPRINT_OK) {
      Serial.println("Falha ao converter imagem no sensor 2");
      finger1.deleteModel(id);  // Remover o modelo do sensor 1
      finger2.deleteModel(id);
      return;
    }
    Serial.println("Imagem convertida com sucesso no sensor 2");

    // Criar modelo no sensor 2
    if (finger2.createModel() != FINGERPRINT_OK) {
      Serial.println("Falha ao criar modelo no sensor 2");
      finger1.deleteModel(id);  // Remover o modelo do sensor 1
      finger2.deleteModel(id);
      return;
    }
    Serial.println("Modelo criado com sucesso no sensor 2");

    // Armazenar o modelo no sensor 2 com o mesmo ID
    if (finger2.storeModel(id) != FINGERPRINT_OK) {
      Serial.println("Falha ao armazenar modelo no sensor 2");
      finger1.deleteModel(id);  // Remover o modelo do sensor 1
      finger2.deleteModel(id);
      return;
    }
    Serial.println("Impressão digital armazenada com sucesso no sensor 2");
    client.publish(topic_pub, "Impressão digital armazenada com sucesso no sensor 2");
    BioData["id"] = id;
    BioData["name"] = name;
    BioData["status_"] = status;
    BioData["device_id"] = Device_id;

    char payload[256];
    serializeJson(BioData, payload);
    if (client.connected()) {
      client.publish(topic_pub_registed, payload);
      Serial.println("Data published to MQTT.");
    } else {
      Serial.println("MQTT not connected. Data not published.");
    }
  }
}

// Registrar a impressão digital no sensor
bool saveIDToSD(uint16_t id, String name, String status) {
  File file = SD.open("/fingerprints.txt", FILE_APPEND);
  if (file) {
    file.print(id);
    file.print(",");
    file.print(name);
    file.print(",");
    file.println(status);
    file.close();
    return true;
  } else {
    Serial.println("Falha ao abrir o arquivo no SD");
    return false;
  }
}


void verifyFingerprint() {


  // Verificar no sensor 1
  Serial.println("Coloque o dedo no sensor 1 ou sensor 2");
  if (finger1.getImage() != FINGERPRINT_OK && finger2.getImage() != FINGERPRINT_OK)
    ;

  // Verifica se o dedo foi colocado no sensor 1
  if (finger1.image2Tz() == FINGERPRINT_OK && finger1.fingerSearch() == FINGERPRINT_OK) {
    id = finger1.fingerID;
    Serial.print("Impressão digital encontrada no sensor 1 com ID: ");
    Serial.println(id);

    if (getNameAndStatus(id, name, status)) {
      Serial.print("Nome: ");
      Serial.println(name);
      Serial.print("Status: ");
      Serial.println(status);
      finger1.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_PURPLE, 10);
    } else {
      Serial.println("Falha ao recuperar nome e status");
    }
    found = true;  // Impressão digital encontrada no sensor 1
    logg_info = "in";
  }

  // Se não for encontrada no sensor 1, verificar no sensor 2
  if (!found) {
    // Verifica se o dedo foi colocado no sensor 2
    if (finger2.image2Tz() == FINGERPRINT_OK && finger2.fingerSearch() == FINGERPRINT_OK) {
      int id = finger2.fingerID;
      Serial.print("Impressão digital encontrada no sensor 2 com ID: ");
      Serial.println(id);
      String name, status;
      if (getNameAndStatus(id, name, status)) {
        Serial.print("Nome: ");
        Serial.println(name);
        Serial.print("Status: ");
        Serial.println(status);
        finger2.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_PURPLE, 10);
      } else {
        Serial.println("Falha ao recuperar nome e status");
      }
      found = true;  // Impressão digital encontrada no sensor 2
      logg_info = "out";
    }
  }

  // Se não foi encontrada em nenhum dos sensores
  if (!found) {
    Serial.println("Impressão digital não encontrada em nenhum dos sensores");
    finger1.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
    finger2.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
  }
}


void deleteFingerprint() {


  String nameToDelete = name_r;
  nameToDelete.trim();  // Remove espaços em branco extras

  int id = getIDByName(nameToDelete);
  if (id != -1) {
    if (deleteIDFromSD(nameToDelete) && deleteIDFromSensor(id)) {
      Serial.println("Impressão digital deletada com sucesso");
    } else {
      Serial.println("Falha ao deletar impressão digital");
    }
  } else {
    Serial.println("Nome não encontrado");
  }
}

void deleteAllFingerprints() {
  Serial.println("Deletando todas as impressões digitais do SD...");

  if (deleteAllFromSD()) {
    Serial.println("Todas as impressões digitais foram deletadas do SD com sucesso");
  } else {
    Serial.println("Falha ao deletar todas as impressões digitais do SD");
  }

  Serial.println("Deletando todas as impressões digitais do sensor 1...");
  finger1.LEDcontrol(FINGERPRINT_LED_FLASHING, 50, FINGERPRINT_LED_RED, 10);
  if (deleteAllFromSensor(finger1)) {
    Serial.println("Todas as impressões digitais foram deletadas do sensor 1 com sucesso");
    finger1.LEDcontrol(FINGERPRINT_LED_FLASHING, 50, FINGERPRINT_LED_BLUE, 10);
  } else {
    Serial.println("Falha ao deletar todas as impressões digitais do sensor 1");
  }

  Serial.println("Deletando todas as impressões digitais do sensor 2...");
  finger2.LEDcontrol(FINGERPRINT_LED_FLASHING, 50, FINGERPRINT_LED_RED, 10);
  if (deleteAllFromSensor(finger2)) {
    Serial.println("Todas as impressões digitais foram deletadas do sensor 2 com sucesso");
    finger2.LEDcontrol(FINGERPRINT_LED_FLASHING, 50, FINGERPRINT_LED_BLUE, 2);
  } else {
    Serial.println("Falha ao deletar todas as impressões digitais do sensor 2");
  }
}

uint16_t generateUniqueID() {
  // Implemente a lógica para gerar um ID único
  static uint16_t currentID = 1;
  while (idExists(currentID)) {
    currentID++;
  }
  return currentID;
}

bool idExists(uint16_t id) {
  File file = SD.open("/fingerprints.txt");
  if (!file) {
    return false;
  }
  while (file.available()) {
    if (file.parseInt() == id) {
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}


bool getNameAndStatus(uint16_t id, String& name, String& status) {
  File file = SD.open("/fingerprints.txt");
  if (!file) {
    return false;
  }
  while (file.available()) {
    String line = file.readStringUntil('\n');
    int commaIndex1 = line.indexOf(',');
    int commaIndex2 = line.indexOf(',', commaIndex1 + 1);
    if (line.substring(0, commaIndex1).toInt() == id) {
      name = line.substring(commaIndex1 + 1, commaIndex2);
      status = line.substring(commaIndex2 + 1);
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

int getIDByName(String nameToFind) {
  File file = SD.open("/fingerprints.txt");
  if (!file) {
    return -1;
  }
  while (file.available()) {
    String line = file.readStringUntil('\n');
    int commaIndex1 = line.indexOf(',');
    int commaIndex2 = line.indexOf(',', commaIndex1 + 1);
    String name = line.substring(commaIndex1 + 1, commaIndex2);
    if (name == nameToFind) {
      int id = line.substring(0, commaIndex1).toInt();
      file.close();
      return id;
    }
  }
  file.close();
  return -1;
}

bool deleteIDFromSD(String nameToDelete) {
  File file = SD.open("/fingerprints.txt");
  if (!file) {
    Serial.println("Falha ao abrir o arquivo fingerprints.txt para leitura");
    return false;
  }
  File tempFile = SD.open("/temp.txt", FILE_WRITE);
  if (!tempFile) {
    Serial.println("Falha ao abrir o arquivo temp.txt para escrita");
    file.close();
    return false;
  }
  bool deleted = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    int commaIndex1 = line.indexOf(',');
    int commaIndex2 = line.indexOf(',', commaIndex1 + 1);
    String name = line.substring(commaIndex1 + 1, commaIndex2);
    if (name != nameToDelete) {
      tempFile.println(line);
    } else {
      deleted = true;
    }
  }
  file.close();
  tempFile.close();
  if (deleted) {
    SD.remove("/fingerprints.txt");
    SD.rename("/temp.txt", "/fingerprints.txt");
  } else {
    SD.remove("/temp.txt");
  }
  return deleted;
}

bool deleteIDFromSensor(int id) {
  if (finger1.deleteModel(id) == FINGERPRINT_OK && finger2.deleteModel(id) == FINGERPRINT_OK) {
    Serial.print("Impressão digital com ID ");
    Serial.print(id);
    Serial.println(" deletada dos sensores com sucesso");
    return true;
  } else {
    Serial.print("Falha ao deletar impressão digital com ID ");
    Serial.println(id);
    return false;
  }
}

bool deleteAllFromSD() {
  if (SD.remove("/fingerprints.txt")) {
    Serial.println("Arquivo fingerprints.txt deletado do SD com sucesso");
    return true;
  } else {
    Serial.println("Falha ao deletar o arquivo fingerprints.txt do SD");
    return false;
  }
}

bool deleteAllFromSensor(Adafruit_Fingerprint& finger) {
  bool success = true;
  for (int id = 1; id <= 200; id++) {  // Assumindo que o sensor pode armazenar até 200 impressões digitais
    if (finger.deleteModel(id) != FINGERPRINT_OK) {
      success = false;
      Serial.print("Falha ao deletar impressão digital com ID ");
      Serial.println(id);
    }
  }
  return success;
}

void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // Files have sizes, directories do not.
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
