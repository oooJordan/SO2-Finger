#ifndef FINGER_HEADER_H
#define FINGER_HEADER_H

#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <utmp.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>


#define SIX_MONTHS_SECONDS 15552000

//variabili globali
bool opt_l = false;
bool opt_s = false;
bool opt_m = false;
bool opt_p = false;
bool insert_space = false;

bool heading = false; //intestazione (opt_s)
char **printed_names; //serve per memorizzate i parametri (nomi) che sono stati già stampati
char **input_names; //serve per memorizzare tutti e soli i parametri (nomi) inseriti da input
char **not_found_names = NULL; //per lista degli utenti (parametri) non esistenti
int num_not_found = 0;//count dei parametri (nomi) in not_found_names

// prototipi di funzione
void call_finger_noparams();
void call_finger_with_params(const char *username_inserted);
bool check_printed_names(const char *name);
void finger_noparams_opt_l();

void printname_s(struct passwd *pw, struct utmp *finger_user);
void printname_l(struct passwd *pw);
void stampashort(struct passwd *pw, struct utmp *finger_user);
void print_not_existing_user(char **not_found_names, int num_not_found);

int graphical_user_interface(struct utmp *finger_user);//grafical user interface
void teletype_writer(struct utmp *finger_user);
char* IDLE(struct utmp *finger_user, int context);
void datetime_first_access(struct utmp *finger_user, int context);
char* formatted_datetime(time_t time);
char* check_wtmp(struct passwd *pw);

char* formatted_phone_number(char *numero);
char* print_gecos_fields(struct passwd *pw, struct utmp *finger_user);
bool write_user_permission(struct utmp *finger_user);

void print_user_plan(const char *username);



#endif