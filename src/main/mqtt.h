#ifndef MQTT_H
#define MQTT_H

void mqtt_start();

void mqtt_envia_mensagem(char * topico, char * mensagem);

extern float thermostat_temp;
extern int thermostat_state;

#endif