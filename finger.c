#include "header.h"


int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "lsmp")) != -1) {
        switch (opt) {
            case 'l':
                opt_l = true;
                break;
            case 's':
                opt_s = true;
                break;
            case 'm':
                opt_m = true;
                break;
            case 'p':
                opt_p = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-s] [-m] [-p] [user ...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    int count_names=0; //inizializzo count_names
    input_names=(char**)malloc(sizeof(char*)*argc);
    for (int i = 1; i < argc; i++) { // scorro parametri inseriti in input
        if (strncmp(argv[i], "-", 1) == 0) { // se c'è un trattino si tratta di un'opzione quindi la salto
            continue;
        }
        //serve per memorizzare tutti e soli i parametri inseriti da input
        input_names[count_names]=argv[i]; //inserisco in array la parola posizionata all'indice count_names
        count_names++; //scorro parametri
    }

    printed_names = (char**)malloc(sizeof(char*)*argc);
    if (count_names == 0) { //non sono stati inseriti dei parametri (nomi) da input
        if(!opt_l){
            call_finger_noparams();
        }else{
            finger_noparams_opt_l();
        }
    } else {
        for(int c_finger_params=0; c_finger_params<count_names; c_finger_params++)//scorro parametri inseriti da input
        {
            call_finger_with_params(input_names[c_finger_params]);//richiamo la funzione per ogni parametro
        }   
    }

    //richiamo funzione che stampa tutti gli utenti (parametri) inseriti in input ma non esistenti
    print_not_existing_user(not_found_names, num_not_found);
    free(input_names);//libero memoria
    return 0;
}



// funzione che verifica se un nome è già stato stampato, se non lo è lo aggiunge a printed_names
bool check_printed_names(const char *username_inserted) {
    int count_names = 0; //lunghezza di printed_names inizializzata a 0
    //calcolo l'effettiva lunghezza attuale di printed_names
    while(printed_names[count_names] != NULL)
    {
        count_names++;
    }
    //scorro printed_names fin quando
    // - trovo una corrispondenza
    // oppure
    // - sono arrivato alla fine dell'array (count_names)
    for (int i = 0; i < count_names; i++) {
        if (strcmp(username_inserted, printed_names[i]) == 0) {
            return true; // Nome già stampato
        }
    }
    // Se il nome non è stato trovato in printed_names, lo memorizza
    printed_names[count_names] = strdup(username_inserted);
    return false; // Nome non era già stampato, restituisce false
}

/*
funzione cerca informazioni sugli utenti nel file PASSWD e successivamente nel file UTMP
se l'utente non viene trovao nel file UTMP verrà eseguita una ricerca nel file WTMP
*/
void call_finger_with_params(const char *username_inserted) {
    bool found_user_name = false; //indica se il nome è stato trovato
    bool user_already_printed = false; //indica se il nome è già stato stampato o meno
    struct passwd *pw;
    setpwent();
    // ciclo per attraversare tutte le voci nel file /etc/passwd
    while ((pw = getpwent()) != NULL) {
        bool user_passwd_found = false; // Indica se è stato trovato l'utente nel file /etc/passwd
        // Se l'opzione -m è attiva, cerco solo per il nome di login (pw_name)
        if (opt_m) {
            if (strcmp(pw->pw_name, username_inserted) == 0) {
                user_passwd_found = true;
            }
        } else {
            // Se l'opzione -m non è attiva, cerco sia per il nome di login che per il nome utente (gecos)
            char *gecos_copy = strdup(pw->pw_gecos); // copio la stringa gecos perché verrà manipolata
            char *gecos_user_name = strsep(&gecos_copy, ","); // ottengo nome dell'utente

            // Verifica se il nome di login dell'utente (pw_name) corrisponde a username_inserted
            // oppure se uno dei nomi nel campo gecos (user_name) corrisponde a username_inserted
            if (strcasecmp(pw->pw_name, username_inserted) == 0 || (gecos_user_name != NULL && strcasecmp(gecos_user_name, username_inserted) == 0)) {
                user_passwd_found = true;
            }
            // Controlla se ci sono ulteriori nomi nel campo gecos dell'utente separati da spazi.
            // Se uno di questi nomi corrisponde a username_inserted, imposta user_passwd_found a true.
            char *space_between_names;
            while (gecos_user_name != NULL && (space_between_names = strsep(&gecos_user_name, " ")) != NULL) {
                if (strcasecmp(space_between_names, username_inserted) == 0) {
                    user_passwd_found = true;
                }
            }
        }

        // se il nome utente è stato trovato
        if (user_passwd_found) {
            //verifico se il nome (login) è stato già stampato
            if (check_printed_names(pw->pw_name)) {
                user_already_printed=true;
                continue;
            }
            found_user_name = true; // ho trovato il nome
            bool printed_user_with_format = false;
            bool trovato_utmp_user = false; //utente nel file utmp

            struct utmp *finger_user;
            setutent(); // apro file utmp, setto puntatore al primo nome nel file
            while ((finger_user = getutent()) != NULL) { // ciclo per attraversare tutte le voci del file utmp
                // se ha una sessione attiva e il nome utente coincide
                if (finger_user->ut_type == USER_PROCESS && strncmp(finger_user->ut_user, pw->pw_name, UT_NAMESIZE) == 0) {
                    trovato_utmp_user = true; // ho trovato utente in utmp
                    // modalità di stampa in base a opzioni
                    if (!opt_s || (opt_s && opt_l)) {
                        if (!printed_user_with_format) { //stampa formato long
                            printname_l(pw);

                            char *risultato = print_gecos_fields(pw, finger_user);
                            if(risultato[0]!='\0'){
                                printf("%s", risultato);
                            }
                            free(risultato);
                            printed_user_with_format = true;
                        }
                        datetime_first_access(finger_user, 1); //stampo la data e l'ora del primo accesso utente
                    } else{ //stampa formato short
                        stampashort(pw, finger_user);
                        printed_user_with_format = true;

                    }
                }
            }
            // se le informazioni sull'utente non sono state ancora stampate
            if ((!printed_user_with_format && !opt_s) || (!printed_user_with_format && (opt_s && opt_l))) {
                printname_l(pw);
                char *gecos_result = print_gecos_fields(pw, finger_user); //ottengo campi gecos
                if(gecos_result[0]!='\0'){
                    printf("%s", gecos_result);//se non è una stringa vuota allora la stampo
                }
                free(gecos_result);
            }
            // se trovato_utmp_user == false allora non si è mai loggato
            if (!trovato_utmp_user) {
                if (finger_user==NULL) { //se non si è mai loggato allora utente sarà NULL
                    if(!opt_s || (opt_s && opt_l)){ //stampa long
                        char *wtmp_result = check_wtmp(pw); //cerco utente nel file wtmp
                        printf("%s", wtmp_result);
                        free(wtmp_result);
                    }else{ //stampa short
                        char *wtmp_result = check_wtmp(pw);
                        printf("%s", wtmp_result);
                        free(wtmp_result);
                    }
                }
            }

            // controllo stampe finali (PLAN)
            if (!opt_p) { 
                if (opt_s && !opt_l) {
                    continue;
                } else { // se no opt -s e no opt -p, allora stampo .plan per ogni utente
                    //scorro nomi stampati
                    int name_occurrences = check_printed_names(pw->pw_name);
                    for(int i = 0; i < name_occurrences; i++)
                    {
                        print_user_plan(pw->pw_name);

                    }

                }
            }
            endutent();
        }
    }
    endpwent();

    //se non ho trovato nessuna corrispondenza allora non esiste un utente con quel nome
    //lo inserisco in un array di stringhe per stamparli alla fine
    if (!found_user_name && !user_already_printed) {
        num_not_found++;
        not_found_names = (char **)realloc(not_found_names, num_not_found * sizeof(char *));
        not_found_names[num_not_found - 1] = strdup(username_inserted); // memorizzo il nome non trovato
    }
}

/*
funzione per gestire la stampa delle informazioni sugli utenti attivi (utmp) in formato long
utilizzando come funzione di supporto per la stampa la funzione call_finger_with_params
*/
void finger_noparams_opt_l(){
    char **names_array = NULL;
    int count_names = 0;
    struct utmp *finger_user;

    while ((finger_user = getutent()) != NULL) {
        if (finger_user->ut_type == USER_PROCESS) {
            char *usr = finger_user->ut_user;
            names_array = realloc(names_array, (count_names + 1) * sizeof(char *));
            names_array[count_names] = strndup(usr, UT_NAMESIZE); //copia della stringa
            count_names++;
        }
    }

    for (int i = 0; i < count_names; i++) {
        call_finger_with_params(names_array[i]);
        free(names_array[i]); // libero la memoria per ogni stringa
    }
    free(names_array); //libero memoria array
}


/*
funzione per gestire la stampa delle informazioni sugli utenti attivi (file UTMP) in formato short
utilizzando il file PASSWD per cercare le informazioni associate all'utente attivo
*/
void call_finger_noparams() 
{
    opt_s=true;
    struct utmp *finger_user;
    struct passwd *pw;
    setutent(); //apro file utmp, setto puntatore al primo nome nel file
    while((finger_user = getutent())!= NULL)// Ciclo per attraversare tutte le voci nel file utmp
    {
        if(finger_user->ut_type == USER_PROCESS)//se utente ha una sessione attiva
        {
            setpwent(); // apro /etc/passwd e carico informazioni associate all'utente
            if((pw=getpwnam(finger_user->ut_user)) != NULL) //carico dentro pw la riga associata all'utente
            {
                stampashort(pw, finger_user); //stampo informazioni utente in forma abbreviata  
            }
            endpwent(); //chiudo file /etc/passwd
        }
    }
    endutent();
}

// funzione per stampare i parametri (nomi) non esistenti
void print_not_existing_user(char **not_found_names, int num_not_found) {
    for (int i = 0; i < num_not_found; i++) {
        printf("finger: %s: no such user.\n", not_found_names[i]);
        free(not_found_names[i]); // libero la memoria per ogni stringa
    }
    free(not_found_names); // libero la memoria per l'array
}

/*
se [utente == NULL] la ricerca avviene nel file WTMP per cercare il suo ultimo accesso
la funzione gestisce anche le relative stampe
*/
char* check_wtmp(struct passwd *pw) {
    struct utmp finger_user;
    char *username_inserted = pw->pw_name; //nome di login
    int last_access_file; // descrittore di file per il file di log degli accessi degli utenti
    char* last_user_access = NULL; // puntatore alla stringa che conterrà l'ultimo accesso dell'utente cercato
    char last_tty[20]; // array per contenere il nome del terminale dell'ultimo accesso
    int graphic_interface=0; // indica se l'utente ha un'interfaccia grafica

    char* wtmp_user_result = malloc(200 * sizeof(char)); // stringa risultante da restituire

    char *gecos_copy = strdup(pw->pw_gecos); //copio la stringa gecos perché verrà manipolata
    char *gecos_user_name = strsep(&gecos_copy, ","); //ottengo nome dell'utente

    if(opt_s && !heading)
    {
        printf("Login     Name       Tty     Idle    Login Time    Office     Office Phone\n");
        heading=true;    
    }
    // apertura del file di log degli accessi degli utenti in sola lettura
    if ((last_access_file = open("/var/log/wtmp", O_RDONLY)) == -1) {
        exit(EXIT_FAILURE); // se l'apertura del file fallisce, termina il programma con un codice di errore
    }

    // scandisco  il file di log degli accessi degli utenti fino alla fine del file
    while (read(last_access_file, &finger_user, sizeof(struct utmp)) == sizeof(struct utmp)) {
        // controllo se il nome dell'utente corrente corrisponde al nome cercato e se il tipo di accesso è USER_PROCESS
        if (strncmp(finger_user.ut_name, username_inserted, UT_NAMESIZE) == 0 && finger_user.ut_type == USER_PROCESS) {
            // se l'utente è stato trovato, ottiengo il tempo dell'ultimo accesso e formatto la stringa per rappresentarlo
            time_t access_time = finger_user.ut_tv.tv_sec; //ultimo accesso in formato UNIX
            last_user_access = formatted_datetime(access_time); // stringa formattata per rappresentare la data e l'ora dell'ultimo accesso
            strncpy(last_tty, finger_user.ut_line, 20); // copio il nome del terminale dell'ultimo accesso

            graphic_interface=graphical_user_interface(&finger_user); //intefaccia grafica (1=si, 0=no)

            break; // termino il ciclo dopo aver trovato il primo accesso dell'utente
        }
    }
    close(last_access_file); // chiudo il file di log degli accessi degli utenti dopo la scansione
    // costruisco la stringa risultato in base a opzioni fornite
    // caso 1: se l'ultimo accesso e last_tty è disponibile (con stampa formato long)
    if (last_user_access != NULL && last_tty!=NULL && ((!opt_s) || (opt_s && opt_l))) {
        snprintf(wtmp_user_result, 1000, "Last login %s (CEST) on %s \n", last_user_access, last_tty);
        // caso 2: utente mai loggato (con stampa formato long)
    } else if (last_user_access == NULL && !opt_s){
        snprintf(wtmp_user_result, 1000, "Never Logged in\n");
        // caso 3: utente mai loggato, non trovato ne in utmp ne in wtmp (con stampa formato short)
    }else if(last_user_access == NULL && opt_s){
        snprintf(wtmp_user_result, 1000, "%s     %2.12s   *        *     No logins\n", username_inserted, gecos_user_name);
    }else{
        if(graphic_interface == 0) //utente non ha un'interfaccia grafica, stampo campi gecos
        {
            snprintf(wtmp_user_result, 1000, "%s      %2.10s  %s  \t*   %s ", username_inserted, gecos_user_name, last_tty, last_user_access);
            char* fields = print_gecos_fields(pw, &finger_user);
            strcat(wtmp_user_result,  fields);
            free(fields);
        }
        else //utente ha un'interfaccia grafica, non stampo campi gecos
        {
            snprintf(wtmp_user_result, 1000, "%s   %s  %s  \t*   %s ", username_inserted, gecos_user_name, last_tty, last_user_access);
            printf("%d", graphical_user_interface(&finger_user));
        }

    }

    return wtmp_user_result; // restituisco la stringa che rappresenta l'ultimo accesso dell'utente cercato (NULL se l'utente non è stato trovato)
}


//funzione per stampare in formato short le informazioni su utenti
void stampashort(struct passwd *pw, struct utmp *finger_user){
    if(!heading)
    {
        printf("Login     Name          Tty     Idle     Login Time    Office     Office Phone\n");
        heading = true;
    }
    printname_s(pw,finger_user);
    teletype_writer(finger_user); // Stampa il nome della tty
    char* final_idle = IDLE(finger_user, 0);
    printf("%s  ", final_idle);
    free(final_idle);
    datetime_first_access(finger_user, 0); // Stampa la data e l'ora del primo accesso
    graphical_user_interface(finger_user); //funzione per visualizzare l'interfaccia grafica dell'utente
    char *buffer_gecos = print_gecos_fields(pw, finger_user);
    printf("%s", buffer_gecos);
    free(buffer_gecos);
}

//funzione per stampare il nome utente e di login in formato short
void printname_s(struct passwd *pw, struct utmp *finger_user){
    char *gecos_copy = strdup(pw->pw_gecos); //copio la stringa gecos perché verrà manipolata
    char *gecos_user_name = strsep(&gecos_copy, ","); //ottengo nome dell'utente
    printf("%-9s %-12.9s ", finger_user->ut_user, gecos_user_name); //stampo nome di login e nomeutente
}

//funzione per stampare il nome utente in formato long
void printname_l(struct passwd *pw)
{
    if (pw != NULL) {
        char *gecos_copy = strdup(pw->pw_gecos); //copio la stringa gecos perché verrà manipolata
        char *gecos_user_name = strsep(&gecos_copy, ",");//ottengo nome dell'utente
        if (gecos_user_name == NULL){ //senon c'è il nome non lo stampo
            printf("Login: %-24s Name:  ", pw->pw_name);
            printf("Directory: %-20s Shell: %-20s\n", pw->pw_dir, pw->pw_shell);
        }
        else{
            printf("Login: %-24s Name: %2.12s\n", pw->pw_name, gecos_user_name);
            printf("Directory: %-20s Shell: %-20s\n", pw->pw_dir, pw->pw_shell);
        }
    }
}

//funzione per stampare il TTY dell'utente
void teletype_writer(struct utmp *finger_user){
    bool write_permission = write_user_permission(finger_user);
    //se non ho i permessi di scrittura
    if(write_permission){
        printf("*%-8s", finger_user->ut_line); //asterisco prima di TTY
    }else{
        printf(" %-8s", finger_user->ut_line);//altrimenti se ho i permessi di scrittura non metto asterisco prima di TTY
    }

}

//funzione per visualizzare l'interfaccia grafica dell'utente
int graphical_user_interface(struct utmp *finger_user) {
    //se l'utente ha un hostname associato
    if (strlen(finger_user->ut_host) != 0) {
        printf(" (%s)\n", finger_user->ut_host); // Stampa la graphical_user_interface nelle parentesi
        return 1; //uno per indicare che l'utente ha un'interfaccia grafica
        insert_space=false;
    } else {
        return 0; //zero per indicare che l'utente non ha un'interfaccia grafica
        insert_space=true;
    }
}

//funzione per stampare il tempo trascorso dall'ultima attività di un utente
char* IDLE(struct utmp *finger_user, int context) {
    time_t current_time = time(NULL);// ottengo l'ora corrente in secondi dal 1 gennaio 1970
    time_t last_activity = finger_user->ut_tv.tv_sec;// ottengo l'ultimo tempo di attività dell'utente
    time_t inactivity_minutes = (current_time - last_activity) / 60; //minuti di inattività
    time_t inactivity_seconds = (current_time - last_activity); //secondi di inattività
    bool write_permission = write_user_permission(finger_user); //raccolta informazioni su permessi di scrittura
    char* idle_result = malloc(200 * sizeof(char)); //alloco spazio per il risultato

    if (context == 1) { // call_finger_with_params
        if (inactivity_seconds >= SIX_MONTHS_SECONDS) {
            // se l'utente è inattivo da più di sei mesi, scrivo l'anno dell'ultima attività
            snprintf(idle_result, 1000, " %d", localtime(&last_activity)->tm_year + 1900);
        } else if (inactivity_minutes >= 60) {
            // se l'utente è inattivo da almeno un'ora, calcolo le ore e i minuti di inattività
            int hours = inactivity_minutes / 60;
            int minutes = inactivity_minutes % 60;
            snprintf(idle_result, 1000, "\t%d hours %d minutes idle \n", hours, minutes);
        } else {
            // altrimenti, scrivo i minuti di inattività
            snprintf(idle_result, 1000, "   %ld minutes idle \n", inactivity_minutes);
        }
        if (write_permission) {
            //utente non ha i permessi di scrittura
            strcat(idle_result, "     (messages off)\n");
        }

    } else if (context == 0) { // login senza parametri (nomi)
        if (inactivity_seconds >= SIX_MONTHS_SECONDS) {
            // se l'utente è inattivo da più di sei mesi, scrivo l'anno dell'ultima attività
            snprintf(idle_result, 1000, "%d", localtime(&last_activity)->tm_year + 1900);
        } else if (inactivity_minutes >= 60) {
            // se l'utente è inattivo da almeno un'ora, calcolo le ore e i minuti di inattività
            int hours = inactivity_minutes / 60;
            int minutes = inactivity_minutes % 60;
            snprintf(idle_result, 1000, "%d:%0d  ", hours, minutes);
        } else {
            // altrimenti, scrivo i minuti di inattività
            snprintf(idle_result, 1000, "%-3ld  ", inactivity_minutes);
        }
    }
    return idle_result; //stampo la stringa risultante
}

//formatto dataora in modo appropriato in base a opzioni
char* formatted_datetime(time_t time) {
    struct tm *tm_info;
    char *final_formatted_datetime = malloc(30 * sizeof(char)); // alloco dinamicamente memoria per la stringa
    tm_info = localtime(&time); // converto il tempo unix nel fuso orario locale
    // scelgo il formato della data e ora in base alle opzioni opz_l e opz_s
    if(!opt_l && opt_s)
    {
        // se opz_l non è attiva ma opz_s è attiva, formato breve senza giorno della settimana
        strftime(final_formatted_datetime, 30, " %b %d %H:%M", tm_info);
    }else if(!opt_s || (!opt_s && !opt_l && !opt_m && !opt_p) || (opt_s && opt_l))
    {
        // Altrimenti, formato completo con giorno della settimana (se opz_s non è attiva,
        // oppure nessuna opzione è attiva, o entrambe opz_s e opz_l sono attive)
        strftime(final_formatted_datetime, 30, "%a %b %d %H:%M", tm_info);
    }
    return final_formatted_datetime; //restituisce puntatore alla stringa formattata
}
/*
    strftime -> formatto DataOra in base al formato che specifico
    sizeof(array)  -> dimensione max
    localtime(&login_time) -> converto tempo unix nel fuso orario locale
*/


//localtime restituisce un puntatore a una struttura tm che rappresenta il tempo locale
//funzione per stampare la data e l'ora del primo accesso dell'utente
void datetime_first_access(struct utmp *finger_user, int context)
{
    time_t login_time = finger_user->ut_time; //tempo di accesso dell'utente
    char *formatted_time = formatted_datetime(login_time); //formatto il tempo di accesso dell'utente
    char* location_info = " "; //inizializzo una stringa vuota

    //se il formato è long (context == 1) e l'utente remoto è connesso da un host remoto
    if (context == 1 && finger_user->ut_host[0] != '\0') {
        location_info = "from";//aggiungo from a stringa
    }

    if (context == 1) { //stampa long e formatto stringa
        printf("On since %s (CEST) on %s %s %s", formatted_time, finger_user->ut_line, location_info, finger_user->ut_host);
        //se l'utente remoto è connesso da un host remoto vado a capo
        if(finger_user->ut_host[0] != '\0')
        {
            printf("\n");
        }
    } else if (context == 0) { //stampa short
        printf("%s  ", formatted_time); //stampo solo il tempo formattato
    }
    free(formatted_time); // libero la memoria allocata per la stringa formattata del tempo

    //inoltre se il contesto è 1 allora devo stampare anche le informazioni riguardanti
    //l'inattività
    if(context != 0)
    {
        //chiamata di funzione a IDLE
        char* final_idle = IDLE(finger_user, 1);
        printf("%s", final_idle); //stampo tempo di inattività
        free(final_idle);
    }
}

//controllo permessi di scrittura degli utenti
bool write_user_permission(struct utmp *finger_user)
{
    //per determinare se l'utente ha i permessi di scrittura
    struct stat file_stat;
    char permission_path[256];
    //copio /dev/ in path
    strcpy(permission_path, "/dev/");

    // concateno ut_line al path
    strncat(permission_path, finger_user->ut_line, UT_LINESIZE);

    // path esiste?
    if (stat(permission_path, &file_stat) == 0) {
        // verifico se ci sono permessi di scrittura per il gruppo e altri, o solo per il gruppo
        if (((file_stat.st_mode & S_IWGRP) && (file_stat.st_mode & S_IWOTH)) || (file_stat.st_mode & S_IWGRP)){
            // se ci sono permessi di scrittura per il gruppo e altri, o solo per il gruppo, ritorno false (no asterisco)
            return false;
        } else {
            // altrimenti, ritorno true (sì asterisco)
            return true;
        }
    }
    // se il path non esiste, ritorno false
    return false;

}

/*
        alloco memoria per il path e
        - strlen(pw->pw_dir) è la lunghezza del percorso della home directory dell'utente
        - strlen("/.plan") è la lunghezza della parola scritta
        - +1 per il carattere finale '\0'
        e questo è il path
        plan_path è letteralmente un puntatore a caratteri e faccio il cast con char perchè
        malloc restituisce un puntatore di tipo void *
        */

// funzione che stampa il plan dell'utente corrente
void print_user_plan(const char *username) {
    struct passwd *pw = getpwnam(username); //ottengo informazioni sull'utente specificato
    //alloco memoria per il path
    char *plan_path = (char *)malloc(strlen(pw->pw_dir) + strlen("/.plan") + 1);
    strcpy(plan_path, pw->pw_dir); // copio il percorso della home directory dell'utente in plan_path
    strcat(plan_path, "/.plan");   // concateno il nome del file ".plan" alla home directory

    // Controlla se il file ".plan" esiste
    if (access(plan_path, F_OK) != -1) {
        // Stampa il contenuto del file ".plan"
        printf("Plan:\n");
        FILE *plan_file = fopen(plan_path, "r");//apro file in modalità lettura
        if (plan_file) { //se il file si apre
            int c;
            while ((c = fgetc(plan_file)) != EOF) { //leggo carattere per carattere fino alla fine (EOF)
                putchar(c); //singolo carattere che sto stampando
            }
            fclose(plan_file); //chiudo il file
        }
    } else {
        // Stampa un messaggio se il file ".plan" non esiste
        printf("No plan.\n");

    }

    free(plan_path); //libero memoria
}

//funzione che formatta i numeri di telefono in modo appropriato
char* formatted_phone_number(char *telephone_number)
{
    //allocazione memoria per il numero formattato(stringa)
    char* final_formatted_number = malloc(sizeof(char) * 14);

    //caso 1 = lunghezza numero = 10
    if (strlen(telephone_number) == 10)
    {
        //formatto il numero
        snprintf(final_formatted_number, 14, "%c%c%c-%c%c%c-%c%c%c%c",
                 telephone_number[0], telephone_number[1], telephone_number[2],
                 telephone_number[3], telephone_number[4], telephone_number[5],
                 telephone_number[6], telephone_number[7], telephone_number[8], telephone_number[9]);
    }
    //caso 2 = lunghezza numero = 5
    else if(strlen(telephone_number) == 5)
    {
        snprintf(final_formatted_number, 14, "x%c-%c%c%c%c",
                 telephone_number[0], telephone_number[1], telephone_number[2],
                 telephone_number[3], telephone_number[4]);
    }
    //caso 3 = lunghezza numero = 4
    else if(strlen(telephone_number) == 4)
    {
        snprintf(final_formatted_number, 14, "x%c%c%c%c",
                 telephone_number[0], telephone_number[1], telephone_number[2],
                 telephone_number[3]);
    }else{//altrimenti copio in numero_formattato il numero senza formattarlo
        strcpy(final_formatted_number, telephone_number);
    }

    return final_formatted_number;
}

//funzione che scorre e stampa i campi_gecos degli utenti
char* print_gecos_fields(struct passwd *pw, struct utmp *finger_user)
{
    // allocazione della memoria per gecos_result per contenere il risultato
    char* gecos_result = malloc(1000 * sizeof(char));
    gecos_result[0] = '\0'; // inizializzazione della stringa gecos_result
    char *gecos_copy = strdup(pw->pw_gecos); //copio la stringa gecos perché verrà manipolata
    if(gecos_copy != NULL)
    {
        strsep(&gecos_copy, ","); // Ignora il primo campo (il nome utente)
        // ciclo per i primi tre campi del gecos (Room, Office Phone, Home Phone)
        for (int i=0; i<3;  ++i)
        {
            char *field = strsep(&gecos_copy, ",");
            // verifico se il campo è valido (non vuoto)
            if (field != NULL && strcmp(field, "") != 0) {
                if(i==0) //campo 1: Room
                {
                    if(!opt_s || (opt_s && opt_l)){ //per formato LONG
                        strcat(gecos_result, "Room:");
                        strcat(gecos_result, field);
                        strcat(gecos_result, "\n");
                    }else if((finger_user->ut_host[0] == '\0') && opt_s)  //per formato SHORT
                    {
                        strcat(gecos_result, "  ");
                        strcat(gecos_result, field);
                        strcat(gecos_result, " \t");
                    }

                }else if(i==1) //campo 2: Office Phone
                {
                    char *formatted_office_phone = formatted_phone_number(field); //formatto numero di telefono
                    if(formatted_office_phone!=NULL) //se esiste lo stampo in base al formato
                    {
                        if(!opt_s || (opt_s && opt_l)) //per formato LONG
                        {
                            strcat(gecos_result, "Office Phone: ");
                            strcat(gecos_result, formatted_office_phone);
                            strcat(gecos_result, "\n");
                        }else if((finger_user->ut_host[0] == '\0') && opt_s){ //per formato SHORT
                            if(insert_space)
                            {

                                strcat(gecos_result, "\t\t ");
                            }
                            else
                            {
                                strcat(gecos_result, "    ");
                            }
                            strcat(gecos_result, formatted_office_phone);
                            strcat(gecos_result, "\n");
                        }
                    }
                }
                else{ //campo 3: Home Phone
                    char *formatted_home_phone = formatted_phone_number(field); //formatto numero di telefono
                    if(formatted_home_phone != NULL && !opt_s){ //stampa per formato LONG
                        strcat(gecos_result, "Home Phone: ");
                        strcat(gecos_result, formatted_home_phone);
                        strcat(gecos_result, "\n");
                    }
                }
            }

        }

    }
    return gecos_result;
}